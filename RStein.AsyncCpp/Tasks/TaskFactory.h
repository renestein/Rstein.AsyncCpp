﻿#pragma once
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
      return Run(std::forward<TFunc>(func),
                 AsyncPrimitives::CancellationToken::None(),
                 Schedulers::Scheduler::DefaultScheduler());
    }

    template <typename TFunc>
    static auto Run(TFunc&& func,
                    const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken)
    {
      return Run(std::forward<TFunc>(func),
                cancellationToken,
                Schedulers::Scheduler::DefaultScheduler());
    }

    
    template <typename TFunc>
    static auto Run(TFunc&& func,
                    const Schedulers::Scheduler::SchedulerPtr& scheduler)
    {
      return Run(std::forward<TFunc>(func),
                AsyncPrimitives::CancellationToken::None(),
                scheduler);
    }

    template <typename TFunc>
    static auto Run(TFunc&& func,
                    const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken,
                    const Schedulers::Scheduler::SchedulerPtr& scheduler)
    {
      using Ret_Task_Type = decltype(func());
      Task<Ret_Task_Type> task{std::forward<TFunc>(func), scheduler, cancellationToken};
      task.Start();
      return task;
    }
  };
}
