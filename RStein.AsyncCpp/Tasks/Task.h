#pragma once
#include "../AsyncPrimitives/CancellationToken.h"
#include "TaskState.h"
#include "Detail/TaskHelpers.h"
#include <any>
#include <exception>
#include <memory>
#include <functional>
#include <ostream>


namespace RStein::AsyncCpp::Tasks
{
  template<typename TResult>
  class TaskCompletionSource;

  template <typename TResult = void>
  class Task
  {
  public:

    
    using TypedTaskSharedState = Detail::TaskSharedState<TResult>;
    using Ret_Type = TResult;

    template<typename TFunc>
    explicit Task(TFunc func) : Task{std::move(func),
                                     Schedulers::Scheduler::DefaultScheduler(),
                                     AsyncPrimitives::CancellationToken::None()}
    {
    }

    template<typename TFunc>
    Task(TFunc func, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) : Task
                                                                                                        {
                                                                                                          std::move(func),
                                                                                                          Schedulers::Scheduler::DefaultScheduler(),
                                                                                                          cancellationToken,
                                                                                                        }
  
    {
    }

    template<typename TFunc>
    Task(TFunc func, const Schedulers::Scheduler::SchedulerPtr& scheduler) : Task{std::move(func),
                                                                            scheduler,                                                                           
                                                                            AsyncPrimitives::CancellationToken::None()}
    {
      
    }

    template<typename TFunc>
    Task(TFunc func, const Schedulers::Scheduler::SchedulerPtr& scheduler, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) :
      _sharedTaskState{std::make_shared<TypedTaskSharedState>(std::move(func),
                                                              scheduler,
                                                              false,
                                                              cancellationToken)}
    {
    }

    Task(const Task& other) = default;
    Task(Task&& other) noexcept = default;
    Task& operator=(const Task& other) = default;
    Task& operator=(Task&& other) noexcept = default;

    ~Task() = default;
    unsigned long Id() const;
    void Start();
    bool IsCanceled() const;
    bool IsCompleted() const;
    bool IsFaulted() const;
    TaskState State() const;
    void Wait() const;
    template <typename TResultCopy = Ret_Type>
    typename std::enable_if<!std::is_same<TResultCopy, void>::value, Ret_Type>::type Result() const
    {
      return _sharedTaskState->GetResult();
    }

    
    template<typename TContinuation>
    auto ContinueWith(TContinuation continuation);
    template <class TContinuation>
    auto ContinueWith(TContinuation continuation, const Schedulers::Scheduler::SchedulerPtr& continuationScheduler);
    std::exception_ptr Exception() const;

    auto operator co_await() const
    {
      struct TaskAwaiter
      {
         Task<TResult> _task;

         TaskAwaiter(Task<TResult> task): _task(task)
         {
           
         }

         [[nodiscard]] bool await_ready() const
         {
           return _task.IsCompleted();
         }

        [[nodiscard]] bool await_suspend(std::experimental::coroutine_handle<> continuation)
        {
          if (_task.IsCompleted())
          {
            return false;
          }
          _task.ContinueWith([continuation=std::move(continuation)](const auto& _) {continuation();});
           return true;
        }

        [[nodiscard]] Ret_Type await_resume() const
        {
          if constexpr(std::is_same<Ret_Type, void>::value)
          {
            _task.Wait(); //Propagate exception
          }
          else
          {
             return _task.Result();
          }
        }
      };

      TaskAwaiter awaiter{*this};
      return awaiter;

    }


    friend bool operator==(const Task& lhs, const Task& rhs)
    {
      return lhs._sharedTaskState == rhs._sharedTaskState;
    }

    friend bool operator!=(const Task& lhs, const Task& rhs)
    {
      return !(lhs == rhs);
    }

    friend bool operator<(const Task& lhs, const Task& rhs)
    {
      return lhs._sharedTaskState < rhs._sharedTaskState;
    }

    friend bool operator<=(const Task& lhs, const Task& rhs)
    {
      return !(rhs < lhs);
    }

    friend bool operator>(const Task& lhs, const Task& rhs)
    {
      return rhs < lhs;
    }

    friend bool operator>=(const Task& lhs, const Task& rhs)
    {
      return !(lhs < rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const Task& obj)
    {
      return os << "Task Id: " << obj.Id 
                << " State: " << obj.State()
                << " IsCompleted: " << obj.IsCompleted();
    }

  protected:

    using TaskSharedStatePtr = std::shared_ptr<TypedTaskSharedState>;
    TaskSharedStatePtr _sharedTaskState;

  private:
    
    friend class TaskCompletionSource<TResult>;

    //TaskCompletionSource uses this ctor
    Task() : _sharedTaskState(std::make_shared<TypedTaskSharedState>())
    {
    }

    template<typename TContinuationFunc>
    void addContinuation(Task<TContinuationFunc>& continuationTask) const;
  };

  template <typename TResult>
  unsigned long Task<TResult>::Id() const
  {
    return _sharedTaskState->Id();
  }

  template <typename TResult>
  void Task<TResult>::Start()
  {
    _sharedTaskState->RunTaskFunc();
  }

  template <typename TResult>
  bool Task<TResult>::IsCanceled() const
  {
    return _sharedTaskState->State() == TaskState::Canceled;
  }

  template <typename TResult>
  bool Task<TResult>::IsCompleted() const
  {
    auto taskState = _sharedTaskState->State();
    return taskState == TaskState::RunToCompletion ||
           taskState == TaskState::Faulted ||
           taskState == TaskState::Canceled;
  }

  template <typename TResult>
  bool Task<TResult>::IsFaulted() const
  {
    return _sharedTaskState->State() == TaskState::Faulted;
  }

  template <typename TResult>
  TaskState Task<TResult>::State() const
  {
    return _sharedTaskState->State();
  }

  template <typename TResult>
  void Task<TResult>::Wait() const
  {
    return _sharedTaskState->Wait();
  }

  template<typename TResult>
  template<typename TContinuation>
  auto Task<TResult>::ContinueWith(TContinuation continuation)
  {
    return ContinueWith(std::move(continuation), Schedulers::Scheduler::DefaultScheduler());
  }

  template<typename TResult>
  template<typename TContinuation>
  auto Task<TResult>::ContinueWith(TContinuation continuation, const Schedulers::Scheduler::SchedulerPtr& continuationScheduler)
  {
    using Continuation_Return_Type = decltype(continuation(*this));
    auto continuationFunc = [continuation = std::move(continuation), thisCopy=*this] () mutable {return continuation(thisCopy);};
    Task<Continuation_Return_Type> continuationTask{continuationFunc, continuationScheduler};
    addContinuation(continuationTask);  
    return continuationTask;
  }

  template <typename TResult>
  std::exception_ptr Task<TResult>::Exception() const
  {
    return _sharedTaskState->Exception();
  }

  template <typename TResult>
  template <typename TContinuationResult>
  void Task<TResult>::addContinuation(Task<TContinuationResult>& continuationTask) const
  {
    _sharedTaskState->AddContinuation([continuationTask]() mutable
    {
      continuationTask.Start();
    });
  }  
  
}
