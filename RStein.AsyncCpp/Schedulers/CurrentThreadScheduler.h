#pragma once

#include "Scheduler.h."

namespace RStein::AsyncCpp::Schedulers
{
  class CurrentThreadScheduler : public Scheduler
  {
  public:
    
    CurrentThreadScheduler();
    virtual ~CurrentThreadScheduler(void);

    
    void EnqueueItem(std::function<void()>&& func) override;
    bool IsMethodInvocationSerialized() const override;


  };
}
