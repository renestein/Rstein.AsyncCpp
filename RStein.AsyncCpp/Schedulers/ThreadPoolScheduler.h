#pragma once
#include "Scheduler.h"

namespace RStein::AsyncCpp::Schedulers
{
  class SimpleThreadPool;

  class ThreadPoolScheduler :
      public Scheduler
  {
  public:
    static const int MAX_THREADS_IN_STRAND = 1;

    explicit ThreadPoolScheduler(SimpleThreadPool& threadPool);
    virtual ~ThreadPoolScheduler();

    ThreadPoolScheduler(const ThreadPoolScheduler& other) = delete;
    ThreadPoolScheduler(ThreadPoolScheduler&& other) = delete;
    ThreadPoolScheduler& operator=(const ThreadPoolScheduler& other) = delete;
    ThreadPoolScheduler& operator=(ThreadPoolScheduler&& other) = delete;

    void Start() override;
    void stopThreadPool() const;
    void Stop() override;
    
    bool IsMethodInvocationSerialized() const override;
  protected:
    void OnEnqueueItem(std::function<void()>&& originalFunction) override;
  private:
    SimpleThreadPool& _threadPool;
  };
}
