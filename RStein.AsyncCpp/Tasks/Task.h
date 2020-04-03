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
  class Task
  {

    public:

       //TODO: Avoid function, use template
       template<typename TFunc>
       explicit Task(TFunc func) :  _sharedTaskState{std::make_shared<Detail::TypedTaskSharedState<TFunc>>(std::move(func),
                                                                                                                           false,
                                                                                                                           CancellationToken::None())}
       {
         
       }

      template<typename TFunc>
      Task(TFunc func, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) : _sharedTaskState{std::make_shared<Detail::TypedTaskSharedState<TFunc>>(std::move(func),
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

      //static Task Run(std::function<void()>&& action);
    
      //static Task Run(std::function<void()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken);

     
      //TODO: Enable when Task is coroutine promise

    /*  static Task Run (std::function<Task()>&& action);
      static Task Run(std::function<Task()>&& action, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken);*/

      void Wait() const;

      //TODO: Add Scheduler overloads
      //TODO: Use template instead of the func
      /*
      Task ContinueWith(std::function<void(const Task& task)>&& continuation);
      template<typename TCr>
      Task ContinueWith(std::function<TCr(const Task& task)>&& continuation);*/

      std::exception_ptr Exception() const;

    protected:
      Task();

      std::any GetTaskResult() const;
      using TaskSharedStatePtr = std::shared_ptr<Detail::TaskSharedState>;
      TaskSharedStatePtr _sharedTaskState;

    private:    
    Task(TaskSharedStatePtr taskSharedState) : _sharedTaskState(taskSharedState)
    {
      
    }
    void addContinuation(Task& continuationTask) const;
    
  };

  template<typename TResult>
  unsigned long Task<TResult>::Id() const
  {
    return _sharedTaskState->Id();
  }
  template<typename TResult>
  void Task<TResult>::Start()
  {
    _sharedTaskState->RunTaskFunc();
  }

  template<typename TResult>
  bool Task<TResult>::IsCanceled() const
  {
    return _sharedTaskState->State() == TaskState::Canceled;
  }

  template<typename TResult>
  bool Task<TResult>::IsCompleted() const
  {
    auto taskState = _sharedTaskState->State();
    return taskState == TaskState::RunToCompletion ||
           taskState == TaskState::Faulted ||
            taskState == TaskState::Canceled;

  }

  template<typename TResult>
  bool Task<TResult>::IsFaulted() const
  {
    return _sharedTaskState->State() == TaskState::Faulted;
  }

  template<typename TResult>
  TaskState Task<TResult>::State() const
  {
    return _sharedTaskState->State();
  }

  template<typename TResult>
  void Task<TResult>::Wait() const
  {
    return _sharedTaskState->Wait();
  }

  /*template<typename TResult>
  Task<TResult> Task<TResult>::ContinueWith(std::function<void(const Task& task)>&& continuation)
  {
    if (!continuation)
    {
      throw std::invalid_argument("continuation");
    }

    Task continuationTask{[continuation = std::move(continuation), thisCopy=*this]{continuation(thisCopy);}};
    addContinuation(continuationTask);  
    return continuationTask;
  }*/

  template<typename TResult>
  std::exception_ptr Task<TResult>::Exception() const
  {
    return _sharedTaskState->Exception();
  }

  template<typename TResult>
  Task<TResult>::Task() : _sharedTaskState{}
  {

  }

  template<typename TResult>
  void Task<TResult>::addContinuation(Task& continuationTask) const
  {
    _sharedTaskState->AddContinuation([continuationTask] () mutable{continuationTask.Start();});
  }

  //template<typename TResult>
  //Task<TResult> Task<TResult>::Run(std::function<void()>&& action)
  //{
  //  return Run(std::move(action), CancellationToken::None());
  //}

  //template<typename TResult>
  //Task<TResult> Task<TResult>::Run(std::function<void()>&& action, const CancellationToken::CancellationTokenPtr& cancellationToken)
  //{
  //   auto task = Task{std::move(action), cancellationToken};
  //   task.Start();
  //   return task;
  //}

  class TaskFactory
  {
    public:
     template<typename TFunc>
     static auto Run(TFunc&& func)
      {      
        using Ret_Type = typename Detail::TypedTaskSharedState<TFunc>::Ret_Type;
        Task<Ret_Type> task{std::forward<TFunc>(func)};
        task.Start();
        return task;
      }    
  };

}
