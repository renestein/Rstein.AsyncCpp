#pragma once
#include "../AsyncPrimitives/CancellationToken.h"
#include <any>
#include <atomic>
#include <exception>
#include <memory>
#include <functional>

namespace my_namespace
{
  
}
namespace RStein::AsyncCpp::Tasks
{
  enum class TaskState
  {
    Created,
    Scheduled,
    Running,
    Faulted,
    Canceled,
    RunToCompletion
  };

   template<typename TFunc>
   struct TypedTaskSharedState;

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
      template<typename TR>
      explicit Task(std::function<TR()>&& func);

      template<typename TR>
      Task(std::function<TR()>&& func, AsyncPrimitives::CancellationToken::CancellationTokenPtr);

      template<typename TR>
      explicit Task(std::function<TaskT<TR>>&& func);

      template<typename TR>
      explicit Task(std::function<TaskT<TR>>&& func, AsyncPrimitives::CancellationToken::CancellationTokenPtr);
     

      template<typename TFunc>
      friend struct TypedTaskSharedState;

      std::any GetTaskResult() const;
    
      struct TaskSharedState;
      using TaskSharedStatePtr = std::shared_ptr<TaskSharedState>;
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
  class TaskT : Task
  {

  public:
       //TODO: Avoid function, use template
       explicit TaskT(std::function<TResult()>&& action) : Task{action}
       {
         
       }

       TaskT(std::function<TResult()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) : Task{action, cancellationToken}
       {
         
       }

      explicit TaskT(std::function<TaskT<TResult>()>&& action) : Task{action}
      {
         
      }

      TaskT(std::function<Task()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) : Task{action, cancellationToken}
      {
         
      }
    
      TaskT(const TaskT& other) = default;
      TaskT(TaskT&& other) noexcept = default;
      TaskT& operator=(const TaskT& other) = default;
      TaskT& operator=(TaskT&& other) noexcept = default;
      TResult GetResult()
      {
        Wait();
        return std::any_cast<TResult>(GetTaskResult());
      }
  };

}
