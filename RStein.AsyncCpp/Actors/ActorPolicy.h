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
    bool Complete();
    Tasks::Task<void> Completion() const;
    
  protected:
    template <typename TFunc>
    auto ScheduleFunction(TFunc&& func) -> Tasks::Task<decltype(func())>;

    virtual bool CanCompleteNow();
    virtual void OnCompleted();

    private:
    using Actor_Queue_Type  = DataFlow::ActionBlock<std::function<void()>>::InputBlockPtr;
    Actor_Queue_Type _actorQueue;
    Tasks::Task<void> _completionTask;
  };

  inline ActorPolicy::ActorPolicy() : _actorQueue{DataFlow::DataFlowSyncFactory::CreateActionBlock<std::function<void()>>([](const std::function<void()>& message){message();})},
                                      _completionTask{_actorQueue->Completion().ContinueWith([this](auto& _) {OnCompleted();})}
  {
    _actorQueue->Start();
  }

  inline bool ActorPolicy::Complete()
  {
    if(!CanCompleteNow())
    {
      return false;
    }

    try
    {
      _actorQueue->Complete();
     _completionTask.Wait();
    }
    catch(...)
    {
    }

    return true;
  }

  inline Tasks::Task<void> ActorPolicy::Completion() const
  {
    return _completionTask;
  }

  inline ActorPolicy::~ActorPolicy()
  {
    Complete();
  }

  inline bool ActorPolicy::CanCompleteNow()
  {
    return true;
  }

  inline void ActorPolicy::OnCompleted()
  {
  }

  template <typename TFunc>
  auto ActorPolicy::ScheduleFunction(TFunc&& func) -> Tasks::Task<decltype(func())>
  {    

    using Ret_Task_Type = decltype(func());
    using TaskCompletionSource_Type = Tasks::TaskCompletionSource<Ret_Task_Type>;
    TaskCompletionSource_Type tcs{};
    co_await _actorQueue->AcceptInputAsync([func=func, tcs] () mutable
    {
      try
      {
        if constexpr (Tasks::Task<Ret_Task_Type>::IsTaskReturningVoid())
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

