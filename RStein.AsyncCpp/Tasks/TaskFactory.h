#pragma once
#include "../AsyncPrimitives/CancellationToken.h"
#include "Task.h"

namespace RStein::AsyncCpp::Tasks
{
  class TaskFactory
  {
  private:
    template <typename TFunc>
    static auto unwrapNestedTaskRun(TFunc&& func,
                    AsyncPrimitives::CancellationToken cancellationToken,
                    const Schedulers::Scheduler::SchedulerPtr& scheduler)->decltype(func())
    {
      using Ret_Task_Type = decltype(func());
      Task<Ret_Task_Type> outerTask{std::forward<TFunc>(func), scheduler, std::move(cancellationToken)};
      outerTask.Start();

      auto nestedTask = co_await outerTask.ConfigureAwait(false);
      if constexpr (decltype(nestedTask)::IsTaskReturningVoid())
      {
        co_await nestedTask.ConfigureAwait(false);
      }
      else
      { 
        auto result = co_await nestedTask.ConfigureAwait(false);
        co_return result;
      }
    }

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
      
      if constexpr (Task<Ret_Task_Type>::IsTaskReturningTask())
      {
        return TaskFactory::unwrapNestedTaskRun(std::forward<TFunc>(func),
                                   std::move(cancellationToken),
                                   scheduler);
      }
      else
      {
        Task<Ret_Task_Type> task{std::forward<TFunc>(func), scheduler, std::move(cancellationToken)};
        task.Start();
        return task;
      }
    }
    
  };
}
