#pragma once
#include "GlobalTaskSettings.h"
#include "../AsyncPrimitives/CancellationToken.h"
#include "TaskState.h"
#include "../Detail/Tasks/TaskHelpers.h"
#include "../Threading/SynchronizationContext.h"

#include <any>
#include <exception>
#include <memory>
#include <ostream>

#if defined(__clang__)
#include "../ClangWinSpecific/Coroutine.h"
#elif defined(__cpp_impl_coroutine)
#include <coroutine>
#else
#include <experimental/coroutine>
#endif

namespace RStein::AsyncCpp::Tasks
{
  template<typename TResult>
  class TaskCompletionSource;

  
  template <typename TResult = void>
  class Task
  {
  private:
  template<typename>
  struct is_task_returning_nested_task : std::false_type
  {
    
  };

  template<typename TNestedResult>
  struct is_task_returning_nested_task<Task<TNestedResult>> : std::true_type
  {
    
  };

  static inline constexpr bool is_task_returning_nested_task_v = is_task_returning_nested_task<TResult>::value;

  public:

    using TypedTaskSharedState = Detail::TaskSharedState<TResult>;

    using Ret_Type = TResult;
    struct InvalidPlaceholderTaskCreator
    {
       Task<TResult> operator()()
       {
         static Task<TResult> placeholderTask{true};
         return placeholderTask;
       }
    };

    constexpr static bool IsTaskReturningTask()
    {
      return is_task_returning_nested_task_v;
    }

    constexpr static bool IsTaskReturningVoid()
    {
      return std::is_same<Ret_Type, void>::value;
    }
    
    template<typename TFunc>
    explicit Task(TFunc func) : Task{std::move(func),
                                     Schedulers::Scheduler::DefaultScheduler(),
                                     AsyncPrimitives::CancellationToken::None()}
    {
    }

    template<typename TFunc>
    Task(TFunc func, AsyncPrimitives::CancellationToken cancellationToken) : Task
                                                                              {
                                                                            std::move(func),
                                                                            Schedulers::Scheduler::DefaultScheduler(),
                                                                            std::move(cancellationToken)}
  
    {
    }

    template<typename TFunc>
    Task(TFunc func, const Schedulers::Scheduler::SchedulerPtr& scheduler) : Task{std::move(func),
                                                                            scheduler,                                                                           
                                                                            AsyncPrimitives::CancellationToken::None()}
    {
      
    }

    template<typename TFunc>
    Task(TFunc func, const Schedulers::Scheduler::SchedulerPtr& scheduler, AsyncPrimitives::CancellationToken cancellationToken) :
      _sharedTaskState{std::make_shared<TypedTaskSharedState>(std::move(func),
                                                              scheduler,
                                                              false,
                                                              std::move(cancellationToken))}
    {
      static_assert(!std::is_rvalue_reference_v<TResult>, "RValue reference is not supported.");
    }

    //TODO: Revisit copy/move, ensure that all operations are thread safe.
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

    template <class TContinuation>
    auto ContinueWith(TContinuation continuation,
                      const AsyncPrimitives::CancellationToken& cancellationToken);
    
    template<typename TContinuation>
    auto ContinueWith(TContinuation continuation);

    template <class TContinuation>
    auto ContinueWith(TContinuation continuation,
                      const Schedulers::Scheduler::SchedulerPtr& continuationScheduler);

    template <class TContinuation>
    auto ContinueWith(TContinuation continuation,
                      const Schedulers::Scheduler::SchedulerPtr& continuationScheduler,
                      const AsyncPrimitives::CancellationToken& cancellationToken);

    std::exception_ptr Exception() const;

    auto operator co_await() const
    {           
      return ConfigureAwait(!Threading::SynchronizationContext::Current()->IsDefault());      
    }

     struct SyncContextAwareTaskAwaiter
      {
         Task<TResult> _task;
         bool _continueOnCapturedContext;

         SyncContextAwareTaskAwaiter(Task<TResult> task, bool continueOnCapturedContext): _task(task),
                                                                                          _continueOnCapturedContext(continueOnCapturedContext)
         {
           
         }

         [[nodiscard]] bool await_ready() const
         {
           return !GlobalTaskSettings::TaskAwaiterAwaitReadyAlwaysReturnsFalse && _task.IsCompleted();
         }

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
        [[nodiscard]] bool await_suspend(std::coroutine_handle<> continuation)
#else
        [[nodiscard]] bool await_suspend(std::experimental::coroutine_handle<> continuation)
#endif
        {
          if (!GlobalTaskSettings::TaskAwaiterAwaitReadyAlwaysReturnsFalse && _task.IsCompleted())
          {
            return false;
          }
           auto syncContext = Threading::SynchronizationContext::Current();
           if (GlobalTaskSettings::UseOnlyConfigureAwaitFalseBehavior ||
             !_continueOnCapturedContext ||
             syncContext->IsDefault())
           {
             _task.ContinueWith([continuation=continuation](const auto& _) mutable {continuation();});
           }
           else
           {
             _task.ContinueWith([continuation=continuation, syncContext](const auto& _)
             {
               syncContext->Post(continuation);
             });
           }

           return true;
        }

        Ret_Type await_resume() const
        {
          if constexpr(IsTaskReturningVoid())
          {
            _task.Wait(); //Propagate exception
            return (void) 0;
          }
          else
          {
             return _task.Result();
          }
        }
      };

    auto ConfigureAwait(bool continueOnCapturedContext) const 
    {
     
      SyncContextAwareTaskAwaiter awaiter{*this, continueOnCapturedContext};
      return awaiter;
    }
    template<typename TResultCopy=TResult>
    std::enable_if_t<is_task_returning_nested_task_v, TResultCopy> Unwrap()
    {
      //Unwrap without co_await;
      decltype(this->Result()) proxyTask{false};
      //TODO: Unify logic, write TaskFromAnother Task function;
      //try to keep potential coroutine func alive (thisCopy)
      this->ContinueWith([thisCopy = *this, proxyTask](auto& self)
      {
        
        if constexpr (decltype(self.Result())::IsTaskReturningVoid())
        {
          self.Result().ContinueWith([thisCopy = thisCopy, proxyTask](auto& innerTask)
          {
            assert(innerTask.IsCompleted());

            switch (innerTask.State())
            {
              case TaskState::RunToCompletion:
              {
               proxyTask._sharedTaskState->template TrySetResult<void>();
                break;
              }
              case TaskState::Faulted:
              {
                proxyTask._sharedTaskState->TrySetException(innerTask.Exception());
                break;
              }
              case TaskState::Canceled:
              {
                proxyTask._sharedTaskState->TrySetCanceled();;
                break;
              }
              default:
              {
                assert(false);
                break;
              }
            }
            
          });

        }
        else
        {
          self.Result().ContinueWith([thisCopy = thisCopy, proxyTask](auto& innerTask)
          {
            assert(innerTask.IsCompleted());
           switch (innerTask.State())
            {
              case TaskState::RunToCompletion:
              {
                proxyTask._sharedTaskState->template TrySetResult<decltype(innerTask.Result())>(innerTask.Result());
                break;
              }
              case TaskState::Faulted:
              {
                proxyTask._sharedTaskState->TrySetException(innerTask.Exception());
                break;
              }
              case TaskState::Canceled:
              {
                proxyTask._sharedTaskState->TrySetCanceled();;
                break;
              }
              default:
              {
                assert(false);
                break;
              }
            }

          });
          
        }
      });

      return proxyTask;
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
    friend class Task<Task<TResult>>;

    //TaskCompletionSource uses this ctor
    Task(bool isInvalidPlaceholderTask = false) : _sharedTaskState(std::make_shared<TypedTaskSharedState>(isInvalidPlaceholderTask))
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
  auto Task<TResult>::ContinueWith(TContinuation continuation,
                                   const AsyncPrimitives::CancellationToken& cancellationToken)
  {
    return ContinueWith(std::move(continuation),
                          Schedulers::Scheduler::DefaultScheduler(),
                          cancellationToken);
  }

  template<typename TResult>
  template<typename TContinuation>
  auto Task<TResult>::ContinueWith(TContinuation continuation)
  {
    return ContinueWith(std::move(continuation),
                          Schedulers::Scheduler::DefaultScheduler(),
                          AsyncPrimitives::CancellationToken::None());
  }

  template<typename TResult>
  template<typename TContinuation>
  auto Task<TResult>::ContinueWith(TContinuation continuation, const Schedulers::Scheduler::SchedulerPtr& continuationScheduler)
  {
    return ContinueWith(std::move(continuation),
                        continuationScheduler,
                        AsyncPrimitives::CancellationToken::None());
  }

  
  template<typename TResult>
  template<typename TContinuation>
  auto Task<TResult>::ContinueWith(TContinuation continuation,
                                   const Schedulers::Scheduler::SchedulerPtr& continuationScheduler,
                                   const AsyncPrimitives::CancellationToken& cancellationToken)
  {
    using Continuation_Return_Type = decltype(continuation(*this));
    auto continuationFunc = [continuation = std::move(continuation), thisCopy=*this] () mutable {return continuation(thisCopy);};
    Task<Continuation_Return_Type> continuationTask{continuationFunc,
                                                    continuationScheduler,
                                                    cancellationToken};
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
namespace RStein::Functional
  {
    template<typename TResult>
    struct Result_Traits<AsyncCpp::Tasks::Task<TResult>>
    {
      using Result_Type = AsyncCpp::Tasks::Task<TResult>;
      using Default_Value_Func = typename AsyncCpp::Tasks::Task<TResult>::InvalidPlaceholderTaskCreator;
    };  
  }
