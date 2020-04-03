#pragma once
#include "../AsyncPrimitives/CancellationToken.h"
#include "Task.h"

namespace RStein::AsyncCpp::Tasks
{
  class TaskFactory
  {
  public:
    template <typename TFunc>
    static auto Run(TFunc&& func)
    {
      return Run(std::forward<TFunc>(func), AsyncPrimitives::CancellationToken::None());
    }

    template <typename TFunc>
    static auto Run(TFunc&& func,
                    const RStein::AsyncCpp::AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken)
    {
      using Ret_Task_Type = decltype(func());
      Task<Ret_Task_Type> task{std::forward<TFunc>(func), cancellationToken};
      task.Start();
      return task;
    }
  };
}
