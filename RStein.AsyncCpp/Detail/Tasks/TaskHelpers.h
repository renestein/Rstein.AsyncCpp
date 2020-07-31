#pragma once
#include "IdGenerator.h"
#include "../../AsyncPrimitives/OperationCanceledException.h"
#include "../../Schedulers/Scheduler.h"
#include "../../Functional/F.h"
#include "../../Tasks/TaskState.h"
#include "../../Utils/FinallyBlock.h"
#include "../../Collections//ThreadSafeMinimalisticVector.h"



#include <any>
#include <cassert>
#include <utility>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace RStein::AsyncCpp::Detail
{
  struct TaskTag;

  template <typename TResult>
  struct TaskSharedState : public std::enable_shared_from_this<TaskSharedState<TResult>>
  {
      
  public:
    using ContinuationFunc = std::function<void()>;
    using Function_Ret_Type = TResult;

    TaskSharedState(std::function<TResult()> func,
                    Schedulers::Scheduler::SchedulerPtr scheduler,
                    bool isInvalidTaskPlaceHolder,
                    AsyncPrimitives::CancellationToken cancellationToken) :
                    std::enable_shared_from_this<TaskSharedState<TResult>>(),
                    _func(std::move(func)),
                    _isInvalidTaskPlaceHolder(isInvalidTaskPlaceHolder),
                    _cancellationToken(std::move(cancellationToken)),
                    _lockObject{},
                    _waitTaskCv{},
                    _scheduler{std::move(scheduler)},
                    _taskId{::Detail::IdGenerator<TaskTag>::Counter++},
                    _state{Tasks::TaskState::Created },
                    _continuations{ std::vector<ContinuationFunc>{} },
                    _exceptionPtr{ nullptr },
                    _preparedNextState{Tasks::TaskState::Created}
    {
      if constexpr(!std::is_same<TResult, void>::value)
      {
        if (!isInvalidTaskPlaceHolder && _func != nullptr)
        {
          _func = Functional::Memoize0(std::move(_func));
        }
      }

     if (!isInvalidTaskPlaceHolder && !_scheduler)
     {
        _scheduler = Schedulers::Scheduler::DefaultScheduler();
     }
    }

    TaskSharedState(bool isInvalidTaskPlaceHolder) : TaskSharedState(nullptr,
                                                    Schedulers::Scheduler::SchedulerPtr{},
                                                    isInvalidTaskPlaceHolder,
                                                    AsyncPrimitives::CancellationToken::None())
    {

    }

    TaskSharedState(const TaskSharedState& other) = delete;

    TaskSharedState(TaskSharedState&& other) noexcept = delete;

    TaskSharedState& operator=(const TaskSharedState& other) = delete;

    TaskSharedState& operator=(TaskSharedState&& other) noexcept = delete;

    unsigned long Id() const
    {
      return _taskId;
    }

    Tasks::TaskState State() const
    {
      std::lock_guard lock{ _lockObject };
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
   
    AsyncPrimitives::CancellationToken CancellationToken() const
    {
      return _cancellationToken;
    }
    template <typename TResultCopy = TResult>
    typename std::enable_if<!std::is_same<TResultCopy, void>::value, TResult>::type
    GetResult() const
    {
      Wait();
      return _func();
    }

    bool IsCtCanceled() const
    {
      return _cancellationToken.IsCancellationRequested();
    }


    void RunTaskFunc()
    {
      assert(_func != nullptr);
      auto isCtCanceled = IsCtCanceled();
      if (isCtCanceled)
      {
        {
          std::lock_guard lock{ _lockObject };
          _state = Tasks::TaskState::Canceled;
        }

        _waitTaskCv.notify_all();
        runContinuations();

        return;
      }

      {
        std::lock_guard lock{ _lockObject };

        if (_state != Tasks::TaskState::Created)
        {
          throw std::logic_error("Task already started.");
        }


        _state = Tasks::TaskState::Scheduled;
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
            CancellationToken().ThrowIfCancellationRequested();

            {
              std::lock_guard lock{_lockObject};
              assert(_state == Tasks::TaskState::Scheduled);
              _state = Tasks::TaskState::Running;
            }

            DoRunTaskNow();
            _state = Tasks::TaskState::RunToCompletion;
          }
          catch (const AsyncPrimitives::OperationCanceledException&)
          {
            _exceptionPtr = std::current_exception();
            std::lock_guard lock{_lockObject};
            assert(_state == Tasks::TaskState::Scheduled || _state == Tasks::TaskState::Running);
            _state = Tasks::TaskState::Canceled;
          }
          catch (...)
          {
            _exceptionPtr = std::current_exception();
            std::lock_guard lock{_lockObject};
            assert(_state == Tasks::TaskState::Running);
            _state = Tasks::TaskState::Faulted;
          }
        });
    }

    void Wait() const
    {
      std::unique_lock lock{ _lockObject };
      while (_state != Tasks::TaskState::RunToCompletion &&
        _state != Tasks::TaskState::Faulted &&
        _state != Tasks::TaskState::Canceled)
      {
        _waitTaskCv.wait(lock);
      }

      if (_state != Tasks::TaskState::RunToCompletion)
      {
        auto exceptionPtr = Exception();
        if (exceptionPtr == nullptr && _state == Tasks::TaskState::Canceled)
        {
          exceptionPtr = make_exception_ptr(AsyncPrimitives::OperationCanceledException());
        }

        rethrow_exception(exceptionPtr);
      }
    }

    void AddContinuation(ContinuationFunc&& continuationFunc)
    {
      //TODO inline task if possible
      auto runContinuationNow = false;
      {
        std::lock_guard lock{ _lockObject };
        if (_state == Tasks::TaskState::RunToCompletion ||
          _state == Tasks::TaskState::Faulted ||
          _state == Tasks::TaskState::Canceled)
        {
          runContinuationNow = true;
        }
        else
        {
          _continuations.Add(continuationFunc);
        }
      }

      if (runContinuationNow)
      {
        continuationFunc();
      }


      
    }

    void SetException(std::exception_ptr exception, bool completeTask)
    {
       if(exception == nullptr)
       {
         throw std::invalid_argument("exception");
       }

      {
        std::lock_guard lock{ _lockObject };
        throwIfTaskCompleted();
        _exceptionPtr = exception;
         if (completeTask)
         {
          _state = Tasks::TaskState::Faulted;
         }
         else
         {
           _preparedNextState = Tasks::TaskState::Faulted;
         }
      }
      if (completeTask)
      {
        _waitTaskCv.notify_all();
        runContinuations();
      }
    }

    bool TrySetException(std::exception_ptr exception)
    {      
       if(exception == nullptr)
       {
         throw std::invalid_argument("exception");
       }

      {
        std::lock_guard lock{ _lockObject };
        if (_state != Tasks::TaskState::Created)
        {
          return false;
        }
        _exceptionPtr = exception;
        _state = Tasks::TaskState::Faulted;
      }
      _waitTaskCv.notify_all();
      runContinuations();
      return true;
    }


    void SetCanceled(bool completeTask)
    {
      {
        std::lock_guard lock{ _lockObject };
        throwIfTaskCompleted();

        if (completeTask)
        {
          _state = Tasks::TaskState::Canceled;
        }
        else
        {
          _preparedNextState = Tasks::TaskState::Canceled;
        }
      }

      if (completeTask)
      { 
        _waitTaskCv.notify_all();
        runContinuations();
      }
    }

    bool TrySetCanceled()
    {
      {
        std::lock_guard lock{ _lockObject };
        if (_state != Tasks::TaskState::Created)
        {
          return false;
        }

        _state = Tasks::TaskState::Canceled;
      }

      _waitTaskCv.notify_all();
      runContinuations();
      return true;
    }

    template <typename TUResult, typename TResultCopy = TResult>
    void SetResult(typename std::enable_if<!std::is_same<TResultCopy, void>::value, TUResult>::type result, bool completeTask)
    {
      {
        std::lock_guard lock{ _lockObject };
        throwIfTaskCompleted();
        _func = [taskResult = std::move(result)]{ return taskResult; };
       if (completeTask)
       {
        _state = Tasks::TaskState::RunToCompletion;
       }
       else
       {
         _preparedNextState = Tasks::TaskState::RunToCompletion;
       }
      }

      if (completeTask)
      {
        _waitTaskCv.notify_all();
        runContinuations();
      }
    }

    
    template <typename TUResult, typename TResultCopy = TResult>
    bool TrySetResult(typename std::enable_if<!std::is_same<TResultCopy, void>::value, TUResult>::type result)
    {
      {
        std::lock_guard lock{ _lockObject };
        if (_state != Tasks::TaskState::Created)
        {
          return false;
        }
        _func = [taskResult = std::move(result)] { return taskResult; };
        _state = Tasks::TaskState::RunToCompletion;
      }

      _waitTaskCv.notify_all();
      runContinuations();
      return true;
    }

    
    template <typename TResultCopy = TResult>
    typename std::enable_if<std::is_same<TResultCopy, void>::value, void>::type
    SetResult(bool completeTask)
    {
      {
        std::lock_guard lock{ _lockObject };
        throwIfTaskCompleted();
        if (completeTask)
        {
          _state = Tasks::TaskState::RunToCompletion;
        }
        else
        {
          _preparedNextState = Tasks::TaskState::RunToCompletion;
        }
      }

      if (completeTask)
      {
        _waitTaskCv.notify_all();
        runContinuations();
      }
    }

    template <typename TResultCopy = TResult>
    typename std::enable_if<std::is_same<TResultCopy, void>::value, bool>::type
    TrySetResult()
    {
      {
        std::lock_guard lock{ _lockObject };
        if (_state != Tasks::TaskState::Created)
        {
          return false;
        }

        _state = Tasks::TaskState::RunToCompletion;
      }

      _waitTaskCv.notify_all();
      runContinuations();
      return true;
    }

    void PublishResult()
    {
      {
        std::lock_guard lock{ _lockObject };
        throwIfTaskCompleted();
        assert(_preparedNextState == Tasks::TaskState::RunToCompletion ||
             _preparedNextState == Tasks::TaskState::Canceled ||
             _preparedNextState == Tasks::TaskState::Faulted);

        _state = _preparedNextState;
      }
      
      _waitTaskCv.notify_all();
      runContinuations();
    }

    ~TaskSharedState()
    {
      if (_isInvalidTaskPlaceHolder)
      {
        return;
      }

      std::lock_guard lock{ _lockObject };
      if (_state != Tasks::TaskState::RunToCompletion &&
        _state != Tasks::TaskState::Faulted &&
        _state != Tasks::TaskState::Canceled)
      {
        //assert(false);
      }
    }


  private:
    std::function<TResult()> _func;
    bool _isInvalidTaskPlaceHolder;
    AsyncPrimitives::CancellationToken _cancellationToken;
    mutable std::mutex _lockObject;
    mutable std::condition_variable _waitTaskCv;
    Schedulers::Scheduler::SchedulerPtr _scheduler;
    unsigned long _taskId;
    Tasks::TaskState _state;
    Collections::ThreadSafeMinimalisticVector<ContinuationFunc> _continuations;
    std::exception_ptr _exceptionPtr;
    Tasks::TaskState _preparedNextState;

    void DoRunTaskNow()
    {
      _func();
    }

    void runContinuations()
    {
      std::vector<ContinuationFunc> continuations;
      {
        std::lock_guard lock{ _lockObject };
        assert(_state == Tasks::TaskState::RunToCompletion ||
          _state == Tasks::TaskState::Faulted ||
          _state == Tasks::TaskState::Canceled);

        continuations = _continuations.GetSnapshot();
        _continuations.Clear();
      }

      for (auto& continuation : continuations)
      {
        continuation();
      }
    }

    void throwIfTaskCompleted() const
    {
      if (_state != Tasks::TaskState::Created)
      {
        throw std::logic_error("Task already completed.");
      }
    }
  };
  
  
}
