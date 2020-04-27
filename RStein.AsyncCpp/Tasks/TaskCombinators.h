#pragma once
#include "Task.h"
#include "TaskCompletionSource.h"

namespace RStein::AsyncCpp::Tasks
{
  namespace Detail
  {
    template <typename TTask>
    void waitForTask(const TTask& task, std::vector<std::exception_ptr>& exceptions)
    {
      try
      {
        task.Wait();
      }
      catch (...)
      {
        exceptions.push_back(std::current_exception());
      }
    }

    template <typename TTask>
    Task<void> awaitTask(std::vector<std::exception_ptr>& exceptions, const TTask& task)
    {
      try
      {
        co_await task;
      }
      catch (...) 
      {
        exceptions.push_back(std::current_exception());
      }
    }

    template <typename TTaskFirst, typename TTaskSecond, typename... TTaskRest>
    Task<void> awaitTask(std::vector<std::exception_ptr>& exceptions,
                          const TTaskFirst& task,
                          const TTaskSecond& task2,
                          TTaskRest&&... tasksRest)
    {
      co_await awaitTask(exceptions, task);
      co_await awaitTask(exceptions, task2, std::forward<TTaskRest>(tasksRest)...);
    }

    template <typename TTask>
    void waitAnyTask(TTask&& task, TaskCompletionSource<int>& tcs, int taskIndex) 
    {
      auto continuation = [tcs, taskIndex](auto& previous) mutable {tcs.TrySetResult(taskIndex);};
      task.ContinueWith(continuation);      
    }

  }

    
   
  template <typename... TTask>
  void WaitAll(const TTask&... tasks)
  {
    std::vector<std::exception_ptr> exceptions;
    (Detail::waitForTask(tasks, exceptions), ...);
    if (!exceptions.empty())
    {
      throw AggregateException{exceptions};
    }
  }

  template <typename... TTask>
  Task<void> WhenAll(TTask&&... tasks)
  {
    std::vector<std::exception_ptr> exceptions;

    co_await Detail::awaitTask(exceptions, std::forward<TTask>(tasks)...);

    if (!exceptions.empty())
    {
      throw AggregateException{exceptions};
    }
  }

  
  template <typename... TTask>
  int WaitAny(TTask&... tasks)
  {
    TaskCompletionSource<int> anyTcs;
    auto taskIndex = 0;
    (Detail::waitAnyTask(tasks, anyTcs, taskIndex++), ...);
    return anyTcs.GetTask().Result();   
  }

}
