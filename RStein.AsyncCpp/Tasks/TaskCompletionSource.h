#pragma once
#include "Task.h"
#include "../AsyncPrimitives/OperationCanceledException.h"


#include <exception>

#if defined(__clang__)
#include "../ClangWinSpecific/Coroutine.h"
#elif defined(__cpp_impl_coroutine)
#include <coroutine>
#else
#include <experimental/coroutine>
#endif

namespace RStein::AsyncCpp::Tasks
{
   template<typename TPType>
   struct TaskPromise;

   struct TaskVoidPromise;
  template<typename TResult>
  class TaskCompletionSource
  {

  public:
    using TASK_TYPE = Task<TResult>;
    friend struct TaskPromise<TResult>;
    friend struct TaskVoidPromise;
    TaskCompletionSource() : TaskCompletionSource(/*delayPropagationOfTheResult*/ false)
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
      _task._sharedTaskState->SetException(exception, !_delayPropagationOfTheResult);
    }

    bool TrySetException(std::exception_ptr exception)
    {
      return _task._sharedTaskState->TrySetException(exception);
    }
    void SetCanceled()
    {
      return _task._sharedTaskState->SetCanceled(!_delayPropagationOfTheResult);
    }

    bool TrySetCanceled()
    {
      return _task._sharedTaskState->TrySetCanceled();
    }

    template <typename TUResult, typename TResultCopy = TResult>
    typename std::enable_if<!std::is_same<TResultCopy, void>::value, void>::type
    SetResult(TUResult&& result)
    {
      _task._sharedTaskState->template SetResult<TUResult>(std::forward<TUResult>(result), !_delayPropagationOfTheResult);
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
      _task._sharedTaskState->SetResult(!_delayPropagationOfTheResult);
    }

    template <typename TResultCopy = TResult>
    typename std::enable_if<std::is_same<TResultCopy, void>::value, bool>::type
    TrySetResult()
    {
      return _task._sharedTaskState->TrySetResult();
    }

    private:
    TASK_TYPE _task;
    bool _delayPropagationOfTheResult;

    TaskCompletionSource(bool delayPropagationOfTheResult): _task{},
                                                          _delayPropagationOfTheResult(delayPropagationOfTheResult)
    {
      
    }

    void publishResult() const
    {
      if (_task.State() != Tasks::TaskState::Created)
      {
        __debugbreak();
      }
      assert(_task.State() == Tasks::TaskState::Created);
      _task._sharedTaskState->PublishResult();
    }
  };

  template<typename TResult>
  struct TaskPromise
  {    

    TaskPromise() : _tcs{true}
    {

    }

    [[nodiscard]] Task<TResult> get_return_object() const
    {
      return _tcs.GetTask();
    }

    
#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never initial_suspend() const noexcept
#else
    [[nodiscard]] std::experimental::suspend_never initial_suspend() const noexcept
#endif
    
    {
      return {};
    }

    template <typename TU>
    void return_value(TU&& retValue)
    {
      _tcs.SetResult(std::forward<TU>(retValue));
    }

    //TODO return suspend_always, capture and destroy coroutine_handle in Task?
    
#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never final_suspend() const noexcept
#else
    [[nodiscard]] std::experimental::suspend_never final_suspend() const noexcept
#endif
    {
      _tcs.publishResult();
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
        _tcs.SetCanceled();
      }
      catch(...)
      {
        _tcs.SetException(std::current_exception());
      }
    }

    private:
   
      TaskCompletionSource<TResult> _tcs;  
  };

  
  struct TaskVoidPromise
  {    

    TaskVoidPromise() : _tcs{true}
    {

    }

    [[nodiscard]] Task<void> get_return_object() const
    {
      return _tcs.GetTask();
    }
    
#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never initial_suspend() const noexcept
#else
  [[nodiscard]] std::experimental::suspend_never initial_suspend() const noexcept
#endif
   
    {
      return {};
    }


    void return_void()
    {
      _tcs.SetResult();
    }

    //TODO return suspend_always, capture and destroy coroutine_handle in Task?
#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never final_suspend() const noexcept
#else
    [[nodiscard]] std::experimental::suspend_never final_suspend() const noexcept
#endif
    {
      _tcs.publishResult();
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
        _tcs.SetCanceled();
      }
      catch(...)
      {
        _tcs.SetException(std::current_exception());
      }
    }

    private:
      TaskCompletionSource<void> _tcs;
  };
}

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
 namespace std
#else
 namespace std::experimental
#endif
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
