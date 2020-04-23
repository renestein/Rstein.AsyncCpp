#pragma once
#include "Task.h"

#include <exception>

namespace RStein::AsyncCpp::Tasks
{
  template<typename TResult>
  class TaskCompletionSource
  {

  public:
    TaskCompletionSource() : _task{}
    {
   
    }
    TaskCompletionSource(const TaskCompletionSource& other) = delete;
    TaskCompletionSource(TaskCompletionSource&& other) noexcept = default;
    TaskCompletionSource& operator=(const TaskCompletionSource& other) = delete;
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

 
}
