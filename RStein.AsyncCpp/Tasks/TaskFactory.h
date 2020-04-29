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
      return Run(std::forward<TFunc>(func),
                 AsyncPrimitives::CancellationToken::None(),
                 Schedulers::Scheduler::DefaultScheduler());
    }

    template <typename TFunc>
    static auto Run(TFunc&& func,
                    AsyncPrimitives::CancellationToken cancellationToken)
    {
      return Run(std::forward<TFunc>(func),
                std::move(cancellationToken),
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
                    AsyncPrimitives::CancellationToken cancellationToken,
                    const Schedulers::Scheduler::SchedulerPtr& scheduler)
    {
      using Ret_Task_Type = decltype(func());
      Task<Ret_Task_Type> task{std::forward<TFunc>(func), scheduler, std::move(cancellationToken)};
      task.Start();
      return task;
    }
  };
}
