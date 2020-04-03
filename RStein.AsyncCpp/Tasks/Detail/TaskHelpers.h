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

using namespace RStein::AsyncCpp::Collections;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace RStein::AsyncCpp::Schedulers;
using namespace std;

namespace RStein::AsyncCpp::Tasks::Detail
{
  struct NONE_RESULT
  {
  };

  template <typename TFunc>
  struct FunctionWrapper
  {
  public:

    using Function_Ret_Type = decltype(declval<TFunc>()());
    using Is_Void_Return_Function = is_same<Function_Ret_Type, void>;
    using Ret_Type = conditional_t<Is_Void_Return_Function::value, NONE_RESULT, Function_Ret_Type>;

    FunctionWrapper(TFunc&& func,
                    bool isTaskReturnFunc,
                    CancellationToken::CancellationTokenPtr cancellationToken)
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

    CancellationToken::CancellationTokenPtr CancellationToken() const
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
    CancellationToken::CancellationTokenPtr _cancellationToken;
    Ret_Type _retValue;
    bool _hasRetValue;
  };

  struct TaskSharedState : public std::enable_shared_from_this<TaskSharedState>
  {
  public:

    using ContinuationFunc = std::function<void()>;
    TaskSharedState() : enable_shared_from_this<TaskSharedState>{},
                        _lockObject{},
                        _waitTaskCv{},
                        _scheduler{Scheduler::DefaultScheduler()},
                        _taskId{_idGenerator++},
                        _state{TaskState::Created},
                        _continuations{vector<ContinuationFunc>{}},
                        _exceptionPtr{nullptr}

    {
    }

    unsigned long Id() const
    {
      return _taskId;
    }

    TaskState State()
    {
      lock_guard lock{_lockObject};
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


    virtual ~TaskSharedState()
    {
      lock_guard lock{_lockObject};
      if (_state != TaskState::RunToCompletion &&
          _state != TaskState::Faulted &&
          _state != TaskState::Canceled)
      {
        throw std::logic_error("Broken Task shared state");
      }
    }

    void RunTaskFunc()
    {
      auto isCtCanceled = IsCtCanceled();
      if (isCtCanceled)
      {
        {
          lock_guard lock{_lockObject};
          _state = TaskState::Canceled;
        }

        _waitTaskCv.notify_all();
        runContinuations();

        return;
      }

      {
        lock_guard lock{_lockObject};

        if (_state != TaskState::Created)
        {
          throw std::logic_error("Task already started.");
        }


        _state = TaskState::Scheduled;
      }

      _scheduler->EnqueueItem([this, sharedThis = shared_from_this()]
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
            lock_guard lock{_lockObject};
            assert(_state == TaskState::Scheduled);
            _state = TaskState::Running;
          }

          DoRunTaskNow();
          _state = TaskState::RunToCompletion;
        }
        catch (const AsyncPrimitives::OperationCanceledException&)
        {
          _exceptionPtr = current_exception();
          lock_guard lock{_lockObject};
          if (_state == TaskState::Canceled)
          {
            __debugbreak();
          }
          assert(_state == TaskState::Scheduled || _state == TaskState::Running);
          _state = TaskState::Canceled;
        }
        catch (...)
        {
          _exceptionPtr = current_exception();
          lock_guard lock{_lockObject};
          assert(_state == TaskState::Running);
          _state = TaskState::Faulted;
        }
      });
    }

    void Wait() const
    {
      unique_lock lock{_lockObject};
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
      lock_guard lock{_lockObject};
      if (_state == TaskState::RunToCompletion ||
          _state == TaskState::Faulted ||
          _state == TaskState::Canceled)
      {
        continuationFunc();
        return;
      }

      _continuations.Add(continuationFunc);
    }

    virtual bool IsTaskBasedReturnFunc() = 0;
    virtual CancellationToken::CancellationTokenPtr CancellationToken() = 0;
    virtual bool IsCtCanceled() = 0;   

  protected:
    mutable mutex _lockObject;
    mutable condition_variable _waitTaskCv;
    Scheduler::SchedulerPtr _scheduler;
    virtual void DoRunTaskNow() = 0;
  private:
    //TODO Check overflow.
    inline static std::atomic<unsigned long> _idGenerator{};
    unsigned long _taskId;
    TaskState _state;
    ThreadSafeMinimalisticVector<ContinuationFunc> _continuations;
    exception_ptr _exceptionPtr;

    void runContinuations()
    {
      vector<ContinuationFunc> continuations;
      {
        lock_guard lock{_lockObject};
        assert(_state == TaskState::RunToCompletion ||
               _state == TaskState::Faulted ||
               _state == TaskState::Canceled);

        continuations = _continuations.GetSnapshot();
        _continuations.Clear();
      }

      for (auto& continuation: continuations)
      {
        continuation();
      }
    }
  };

  template <typename TFunc>
  struct TypedTaskSharedState : TaskSharedState
  {
  public:

    using Ret_Type = typename FunctionWrapper<TFunc>::Ret_Type;
    using Function_Ret_Type = typename FunctionWrapper<TFunc>::Function_Ret_Type;

    TypedTaskSharedState(TFunc func, bool isTaskReturnFunc, CancellationToken::CancellationTokenPtr cancellationToken):
      TaskSharedState{},
      _func(std::move(func),
            isTaskReturnFunc,
            cancellationToken)
    {
    }

    bool IsTaskBasedReturnFunc() override
    {
      return _func.IsTaskBasedReturnFunc();
    }

    CancellationToken::CancellationTokenPtr CancellationToken() override
    {
      return _func.CancellationToken();
    }

    typename FunctionWrapper<TFunc>::Ret_Type GetResult() const
    {
      Wait();
      return _func.ReturnValue();
    }

    bool IsCtCanceled() override
    {
      return _func.IsCancellationRequested();
    }

    virtual ~TypedTaskSharedState() = default;

  protected:

    void DoRunTaskNow() override
    {
      _func.Run();
    }

  private:
    FunctionWrapper<TFunc> _func;
  };
}
