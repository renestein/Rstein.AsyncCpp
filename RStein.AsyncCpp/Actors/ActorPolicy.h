#pragma once
#include "../DataFlow/ActionBlock.h"
#include "../DataFlow/DataflowAsyncFactory.h"
#include "../DataFlow/DataFlowSyncFactory.h"
#include "../Tasks/TaskCompletionSource.h"

#include <memory>

namespace RStein::AsyncCpp::Actors
{  
  class ActorPolicy
  {
    public:

    ActorPolicy();

    ActorPolicy(const ActorPolicy& other) = delete;
    ActorPolicy(ActorPolicy&& other) noexcept = delete;
    ActorPolicy& operator=(const ActorPolicy& other) = delete;
    ActorPolicy& operator=(ActorPolicy&& other) noexcept = delete;
    virtual ~ActorPolicy();

  protected:
    template <typename TFunc>
    auto AddFuncMessage(TFunc&& func) -> Tasks::Task<decltype(func())>;

    private:
    using Actor_Queue_Type  = DataFlow::ActionBlock<std::function<void()>>::InputBlockPtr;
    Actor_Queue_Type _actorQueue;
  };

  inline ActorPolicy::ActorPolicy() : _actorQueue{DataFlow::DataFlowSyncFactory::CreateActionBlock<std::function<void()>>([](const std::function<void()>& message){message();})}
  {
    _actorQueue->Start();
  }

  inline ActorPolicy::~ActorPolicy()
  {
    try
    {
      _actorQueue->Complete();
      _actorQueue->Completion().Wait();
    }
    catch(...)
    {      
    }
  }

  template <typename TFunc>
  auto ActorPolicy::AddFuncMessage(TFunc&& func) -> Tasks::Task<decltype(func())>
  {
    if (!func)
    {
      throw std::invalid_argument("func");
    }

    using Ret_Task_Type = decltype(func());
    using TaskCompletionSource_Type = Tasks::TaskCompletionSource<Ret_Task_Type>;
    TaskCompletionSource_Type tcs{};
    co_await _actorQueue->AcceptInputAsync([func=func, tcs]
    {
      try
      {
        if constexpr (typename TaskCompletionSource_Type::Task_Type::IsTaskReturningVoid())
        {
          func();
          tcs.SetResult();
        }
        else
        {
          Ret_Task_Type retValue = func();
          tcs.SetResult(retValue);
        }
      }
      catch(...)
      {
        tcs.SetException(std::current_exception());
      }
    });

    co_return co_await tcs.GetTask();
  }
}

