#pragma once

#include "Scheduler.h."

namespace RStein::AsyncCpp::Schedulers
{
  class CurrentThreadScheduler : public Scheduler
  {
  public:
    
    CurrentThreadScheduler();
    virtual ~CurrentThreadScheduler();
    CurrentThreadScheduler(const CurrentThreadScheduler& other) = delete;
    CurrentThreadScheduler(CurrentThreadScheduler&& other) = delete;
    CurrentThreadScheduler& operator=(const CurrentThreadScheduler& other) = delete;
    CurrentThreadScheduler& operator=(CurrentThreadScheduler&& other) = delete;

    void Start() override;
    void Stop() override;
    
    
    bool IsMethodInvocationSerialized() const override;

  protected:
    void OnEnqueueItem(std::function<void()>&& func) override;
  };
}
