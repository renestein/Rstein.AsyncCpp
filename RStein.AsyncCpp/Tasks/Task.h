#pragma once
#include "../AsyncPrimitives/CancellationToken.h"
#include "TaskState.h"
#include "Detail/TaskHelpers.h"
#include <any>
#include <exception>
#include <memory>
#include <functional>


namespace RStein::AsyncCpp::Tasks
{

  template<typename TResult>
  class TaskT;

 
  class Task
  {

    public:

       //TODO: Avoid function, use template
       explicit Task(std::function<void()>&& action);
      Task(std::function<void()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr&
           cancellationToken);

      explicit Task(std::function<Task()>&& action);
      Task(std::function<Task()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr&
           cancellationToken);
    
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

      static Task Run(std::function<void()>&& action);
    
      static Task Run(std::function<void()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken);

      template<typename TResult>
      static TaskT<TResult> Run(std::function<TResult()>&& func)
      {
        TaskT<TResult> task{std::move(func)};
        task.Start();
        return task;
      }
      //TODO: Enable when Task is coroutine promise

    /*  static Task Run (std::function<Task()>&& action);
      static Task Run(std::function<Task()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken);*/

      void Wait() const;
      //TODO: Add Scheduler overloads
      //TODO: Use template instead of the func
      
      Task ContinueWith(std::function<void(const Task& task)>&& continuation);
      template<typename TCr>
      Task ContinueWith(std::function<TCr(const Task& task)>&& continuation);

      std::exception_ptr Exception() const;

    protected:
      Task();

      std::any GetTaskResult() const;
      using TaskSharedStatePtr = std::shared_ptr<Detail::TaskSharedState>;
      TaskSharedStatePtr _sharedTaskState;

    private:
    void addContinuation(Task& continuationTask) const;
    
  };

  template <typename TCr>
  Task Task::ContinueWith(std::function<TCr(const Task& task)>&& continuation)
  {
    if (!continuation)
    {
      throw std::invalid_argument("continuation");
    }

    TaskT<TCr>continuationTask {[thisCopy = *this, continuation = std::move(continuation)]{continuation(thisCopy);}};
    addContinuation(continuationTask);
    return continuationTask;
  }

  template<typename TResult>
  class TaskT : public Task
  {

  public:

       //TODO: Avoid function, use template
       explicit TaskT(std::function<TResult()>&& func) : Task{}
                                                        
       {
         _sharedTaskState = std::make_shared<Detail::TypedTaskSharedState<std::function<TResult()>>>(std::move(func),
                                                                                                  false,
                                                                                                  CancellationToken::None());
       }

       TaskT(std::function<TResult()>&& func, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) : Task{func, cancellationToken}
       {
         
       }

      explicit TaskT(std::function<TaskT<TResult>()>&& func) : Task{func}
      {
         
      }

      TaskT(std::function<Task()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) : Task{action, cancellationToken}
      {
         
      }
    
      TaskT(const TaskT& other) = default;
      TaskT(TaskT&& other) noexcept = default;
      TaskT& operator=(const TaskT& other) = default;
      TaskT& operator=(TaskT&& other) noexcept = default;
      TResult Result()
      {        
        return static_pointer_cast<Detail::TypedTaskSharedState<std::function<TResult()>>>(_sharedTaskState)->GetResult();
      }
  };

}
