#pragma once
#include "../../AsyncPrimitives/OperationCanceledException.h"
#include "../../Collections/ThreadSafeMinimalisticVector.h"
#include "../../Schedulers/Scheduler.h"
#include "../../Utils/FinallyBlock.h"
#include "../TaskState.h"
#include <any>
#include <cassert>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace RStein::AsyncCpp::Tasks::Detail
{
  struct NONE_RESULT
  {
  };

  template <typename TFunc>
  struct FunctionWrapper
  {
  public:

    using Function_Ret_Type = decltype(std::declval<TFunc>()());
    using Is_Void_Return_Function = std::is_same<Function_Ret_Type, void>;
    using Ret_Type = std::conditional_t<Is_Void_Return_Function::value, NONE_RESULT, Function_Ret_Type>;

    FunctionWrapper(TFunc&& func,
                    bool isTaskReturnFunc,
                    AsyncPrimitives::CancellationToken::CancellationTokenPtr cancellationToken)
      : _func(std::move(func)),
        _isTaskReturnFunc(isTaskReturnFunc),
        _cancellationToken(std::move(cancellationToken)),
        _retValue{},
        _hasRetValue{false}
    {
    }

    void Run()
    {
      assert(!_hasRetValue);
      if constexpr (Is_Void_Return_Function::value)
      {
        _func();
      }
      else
      {
        _retValue = _func();
      }
      _hasRetValue = true;
    }

    bool IsTaskBasedReturnFunc() const
    {
      return _isTaskReturnFunc;
    }

    constexpr bool IsVoidFunc()
    {
      return Is_Void_Return_Function::value;
    }

    Ret_Type ReturnValue() const
    {
      assert(_hasRetValue);
      return _retValue;
    }

    AsyncPrimitives::CancellationToken::CancellationTokenPtr CancellationToken() const
    {
      return _cancellationToken;
    }

    bool IsCancellationRequested() const
    {
      return _cancellationToken->IsCancellationRequested();
    }


  private:
    TFunc _func;
    bool _isTaskReturnFunc;
    AsyncPrimitives::CancellationToken::CancellationTokenPtr _cancellationToken;
    Ret_Type _retValue;
    bool _hasRetValue;
  };

  template <typename TFunc>
  struct TaskSharedState : public std::enable_shared_from_this<TaskSharedState<TFunc>>
  {
  public:
    using ContinuationFunc = std::function<void()>;
    using Ret_Type = typename FunctionWrapper<TFunc>::Ret_Type;
    using Function_Ret_Type = typename FunctionWrapper<TFunc>::Function_Ret_Type;

    TaskSharedState(TFunc func, bool isTaskReturnFunc, AsyncPrimitives::CancellationToken::CancellationTokenPtr cancellationToken):
       std::enable_shared_from_this<TaskSharedState<TFunc>>(),
      _func(std::move(func),
            isTaskReturnFunc,
            cancellationToken),
      _lockObject{},
      _waitTaskCv{},
      _scheduler{Schedulers::Scheduler::DefaultScheduler()},
      _taskId{_idGenerator++},
      _state{TaskState::Created},
      _continuations{std::vector<ContinuationFunc>{}},
      _exceptionPtr{nullptr}
    {
    }

    unsigned long Id() const
    {
      return _taskId;
    }

    TaskState State()
    {
      std::lock_guard lock{_lockObject};
      return _state;
    }

    bool HasException() const
    {
      return _exceptionPtr != nullptr;
    }

    std::exception_ptr Exception() const
    {
      return _exceptionPtr;
    }

    bool IsTaskBasedReturnFunc()
    {
      return _func.IsTaskBasedReturnFunc();
    }

    AsyncPrimitives::CancellationToken::CancellationTokenPtr CancellationToken()
    {
      return _func.CancellationToken();
    }

    typename FunctionWrapper<TFunc>::Ret_Type GetResult() const
    {
      Wait();
      return _func.ReturnValue();
    }

    bool IsCtCanceled()
    {
      return _func.IsCancellationRequested();
    }


    void RunTaskFunc()
    {
      auto isCtCanceled = IsCtCanceled();
      if (isCtCanceled)
      {
        {
          std::lock_guard lock{_lockObject};
          _state = TaskState::Canceled;
        }

        _waitTaskCv.notify_all();
        runContinuations();

        return;
      }

      {
        std::lock_guard lock{_lockObject};

        if (_state != TaskState::Created)
        {
          throw std::logic_error("Task already started.");
        }


        _state = TaskState::Scheduled;
      }

      _scheduler->EnqueueItem([this, sharedThis = this->shared_from_this()]
      {
        Utils::FinallyBlock finally
        {

            [this]
            {
              _waitTaskCv.notify_all();
              runContinuations();
            }
        };
        try
        {
          CancellationToken()->ThrowIfCancellationRequested();

          {
            std::lock_guard lock{_lockObject};
            assert(_state == TaskState::Scheduled);
            _state = TaskState::Running;
          }

          DoRunTaskNow();
          _state = TaskState::RunToCompletion;
        }
        catch (const AsyncPrimitives::OperationCanceledException&)
        {
          _exceptionPtr = std::current_exception();
          std::lock_guard lock{_lockObject};
          if (_state == TaskState::Canceled)
          {
            __debugbreak();
          }
          assert(_state == TaskState::Scheduled || _state == TaskState::Running);
          _state = TaskState::Canceled;
        }
        catch (...)
        {
          _exceptionPtr = std::current_exception();
          std::lock_guard lock{_lockObject};
          assert(_state == TaskState::Running);
          _state = TaskState::Faulted;
        }
      });
    }

    void Wait() const
    {
      std::unique_lock lock{_lockObject};
      while (_state != TaskState::RunToCompletion &&
             _state != TaskState::Faulted &&
             _state != TaskState::Canceled)
      {
        _waitTaskCv.wait(lock);
      }

      if (_state != TaskState::RunToCompletion)
      {
        auto exceptionPtr = Exception();
        if (exceptionPtr == nullptr && _state == TaskState::Canceled)
        {
          exceptionPtr = make_exception_ptr(AsyncPrimitives::OperationCanceledException());
        }

        rethrow_exception(exceptionPtr);
      }
    }

    void AddContinuation(ContinuationFunc&& continuationFunc)
    {
      //TODO inline task if possible
      std::lock_guard lock{_lockObject};
      if (_state == TaskState::RunToCompletion ||
          _state == TaskState::Faulted ||
          _state == TaskState::Canceled)
      {
        continuationFunc();
        return;
      }

      _continuations.Add(continuationFunc);
    }

    ~TaskSharedState()
    {
      std::lock_guard lock{_lockObject};
      if (_state != TaskState::RunToCompletion &&
          _state != TaskState::Faulted &&
          _state != TaskState::Canceled)
      {
        assert(false);
      }
    }


  private:
    FunctionWrapper<TFunc> _func;
    mutable std::mutex _lockObject;
    mutable std::condition_variable _waitTaskCv;
    Schedulers::Scheduler::SchedulerPtr _scheduler;
    inline static std::atomic<unsigned long> _idGenerator{};
    unsigned long _taskId;
    TaskState _state;
    Collections::ThreadSafeMinimalisticVector<ContinuationFunc> _continuations;
    std::exception_ptr _exceptionPtr;

    void DoRunTaskNow()
    {
      _func.Run();
    }

    void runContinuations()
    {
      std::vector<ContinuationFunc> continuations;
      {
        std::lock_guard lock{_lockObject};
        assert(_state == TaskState::RunToCompletion ||
               _state == TaskState::Faulted ||
               _state == TaskState::Canceled);

        continuations = _continuations.GetSnapshot();
        _continuations.Clear();
      }

      for (auto& continuation : continuations)
      {
        continuation();
      }
    }
  };
}
