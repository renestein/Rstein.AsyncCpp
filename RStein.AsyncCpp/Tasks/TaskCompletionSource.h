#pragma once
#include "Task.h"
#include "../AsyncPrimitives/OperationCanceledException.h"


#include <exception>
#include <experimental/coroutine>

namespace RStein::AsyncCpp::Tasks
{
  template<typename TResult>
  class TaskCompletionSource
  {

  public:
    TaskCompletionSource() : _task{}
    {
   
    }
    TaskCompletionSource(const TaskCompletionSource& other) = default;
    TaskCompletionSource(TaskCompletionSource&& other) noexcept = default;
    TaskCompletionSource& operator=(const TaskCompletionSource& other) = default;
    TaskCompletionSource& operator=(TaskCompletionSource&& other) noexcept = default;
    ~TaskCompletionSource() = default;

    //[[nodiscard]] Task<TResult> Task() const  - problem
    [[nodiscard]] Task<TResult> GetTask() const
    {
      return _task;
    }
   
    void SetException(std::exception_ptr exception)
    {
      _task._sharedTaskState->SetException(exception);
    }

    bool TrySetException(std::exception_ptr exception)
    {
      return _task._sharedTaskState->TrySetException(exception);
    }
    void SetCanceled()
    {
      return _task._sharedTaskState->SetCanceled();
    }

    bool TrySetCanceled()
    {
      return _task._sharedTaskState->TrySetCanceled();
    }

    template <typename TUResult, typename TResultCopy = TResult>
    typename std::enable_if<!std::is_same<TResultCopy, void>::value, void>::type
    SetResult(TUResult&& result)
    {
      _task._sharedTaskState->template SetResult<TUResult>(std::forward<TUResult>(result));
    }

    template <typename TUResult, typename TResultCopy = TResult>
    typename std::enable_if<!std::is_same<TResultCopy, void>::value, bool>::type
    TrySetResult(TUResult&& result)
    {
      return _task._sharedTaskState->template TrySetResult<TUResult>(std::forward<TUResult>(result));
    }

    template <typename TResultCopy = TResult>
    typename std::enable_if<std::is_same<TResultCopy, void>::value, void>::type
    SetResult()
    {
      _task._sharedTaskState->SetResult();
    }

    template <typename TResultCopy = TResult>
    typename std::enable_if<std::is_same<TResultCopy, void>::value, bool>::type
    TrySetResult()
    {
      return _task._sharedTaskState->TrySetResult();
    }

    private:
    Task<TResult> _task;

  };

  template<typename TResult>
  struct TaskPromise
  {    

    TaskPromise() : _tcs()
    {

    }

    [[nodiscard]] Task<TResult> get_return_object() const
    {
      return _tcs.GetTask();
    }

    [[nodiscard]] std::experimental::suspend_never initial_suspend() const noexcept
    {
      return {};
    }

    template <typename TU>
    void return_value(TU&& retValue)
    {
      _tcs.SetResult(std::forward<TU>(retValue));
    }

    [[nodiscard]] std::experimental::suspend_never final_suspend() const noexcept
    {
      return {};
    }

    void unhandled_exception()
    {
       try
      {
        std::rethrow_exception(std::current_exception());
      }
      catch(AsyncPrimitives::OperationCanceledException&)
      {
        _tcs.TrySetCanceled();
      }
      catch(...)
      {
        _tcs.TrySetException(std::current_exception());
      }
    }

    private:
      TaskCompletionSource<TResult> _tcs;  
  };

  struct TaskVoidPromise
  {    

    TaskVoidPromise() : _tcs()
    {

    }

    [[nodiscard]] Task<void> get_return_object() const
    {
      return _tcs.GetTask();
    }

    [[nodiscard]] std::experimental::suspend_never initial_suspend() const noexcept
    {
      return {};
    }


    void return_void()
    {
      _tcs.SetResult();
    }

    [[nodiscard]] std::experimental::suspend_never final_suspend() const noexcept
    {
      return {};
    }

    void unhandled_exception()
    {
      try
      {
        std::rethrow_exception(std::current_exception());
      }
      catch(AsyncPrimitives::OperationCanceledException&)
      {
        _tcs.TrySetCanceled();
      }
      catch(...)
      {
        _tcs.TrySetException(std::current_exception());
      }
    }

    private:
      TaskCompletionSource<void> _tcs;  
  };
}

 namespace std::experimental
{
  template <typename... ARGS>
  struct coroutine_traits<RStein::AsyncCpp::Tasks::Task<void>, ARGS...>
  {
    using promise_type = RStein::AsyncCpp::Tasks::TaskVoidPromise;
  };

  template <typename TR, typename... ARGS>
  struct coroutine_traits<RStein::AsyncCpp::Tasks::Task<TR>, ARGS...>
  {
    using promise_type = RStein::AsyncCpp::Tasks::TaskPromise<TR>;
  };
}
