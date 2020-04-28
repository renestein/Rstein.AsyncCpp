#pragma once
#include "Task.h"
#include "TaskCompletionSource.h"
#include "../AsyncPrimitives/AggregateException.h"

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

  template <typename... TTask>
  Task<int> WhenAny(TTask&... tasks)
  {
    TaskCompletionSource<int> anyTcs;
    auto taskIndex = 0;
    (Detail::waitAnyTask(tasks, anyTcs, taskIndex++), ...);
    return anyTcs.GetTask();   
  }

  //Identity method.
  template<typename TResult>
  auto TaskFromResult(TResult&& taskResult)->Task<std::decay_t<TResult>>
  {
    //TODO: Detect invalid values.
    TaskCompletionSource<std::decay_t<TResult>> tcs;
    tcs.SetResult(taskResult);
    return tcs.GetTask();
  }


  namespace Detail
  {
    
    template<typename TFunc>
    struct BindFuncHolder
    {
      TFunc _func;

      BindFuncHolder(TFunc func)
      {
        _func = func;
      }
    };
    
    template<typename TFunc>
    struct MapFuncHolder
    {
      TFunc _func;

      MapFuncHolder(TFunc func)
      {
        _func = func;
      }
    };
  }

  Task<void> GetCompletedTask();
  

  template<typename TResult>
  auto TaskFromException(std::exception_ptr exception)->Task<std::decay_t<TResult>>
  {
    TaskCompletionSource<std::decay_t<TResult>> tcs;
    tcs.SetException(exception);
    return tcs.GetTask();
  }

  template<typename TResult>
  auto TaskFromCanceled()->Task<std::decay_t<TResult>>
  {
    TaskCompletionSource<std::decay_t<TResult>> tcs;
    tcs.SetCanceled();
    return tcs.GetTask();
  }

  template<typename TSource, typename TMapFunc>
  auto Fmap(Task<TSource> srcTask, TMapFunc mapFunc)->Task<decltype(mapFunc(srcTask.Result()))>
  {    
    co_return mapFunc(co_await srcTask);
  }

  template<typename TSource, typename TMapFunc>
  auto Fbind(Task<TSource> srcTask, TMapFunc mapFunc)->Task<std::decay_t<decltype(mapFunc(srcTask.Result()).Result())>>
  {    
    co_return co_await mapFunc(co_await srcTask);
    
  }

  template<typename TMapFunc>
  Detail::BindFuncHolder<TMapFunc> Fbind(TMapFunc mapFunc)
  {
    return {mapFunc};
  }

  
  template<typename TMapFunc>
  Detail::MapFuncHolder<TMapFunc> Fmap(TMapFunc mapFunc)
  {
    return {mapFunc};
  }

  template<typename TSource, typename TMapFunc>
  auto operator |(Task<TSource> srcTask, Detail::BindFuncHolder<TMapFunc> mapFunc)->decltype(Fbind(srcTask, mapFunc._func))
  {
    return Fbind(srcTask, mapFunc._func);
  }

  
  template<typename TSource, typename TMapFunc>
  auto operator |(Task<TSource> srcTask, Detail::MapFuncHolder<TMapFunc> mapFunc)->decltype(Fmap(srcTask, mapFunc._func))
  {
    return Fmap(srcTask, mapFunc._func);
  }

}
