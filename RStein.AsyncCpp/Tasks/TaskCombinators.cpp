#include "TaskCombinators.h"

namespace RStein::AsyncCpp::Tasks
{
  Task<void> getCompletedTask()
  {
    TaskCompletionSource<void> _tcs;
    _tcs.SetResult();
    return _tcs.GetTask();
  }

  Task<void> GetCompletedTask()
  {
    /*static Task<void> _completedTask = getCompletedTask();*/
    return getCompletedTask();
  }

  Detail::FjoinPlaceholder Fjoin()
  {
    return {};
  }
}
