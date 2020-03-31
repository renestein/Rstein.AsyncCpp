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

    ThreadPoolScheduler(SimpleThreadPool& threadPool);
    virtual ~ThreadPoolScheduler();

    ThreadPoolScheduler(const ThreadPoolScheduler& other) = delete;
    ThreadPoolScheduler(ThreadPoolScheduler&& other) = delete;
    ThreadPoolScheduler& operator=(const ThreadPoolScheduler& other) = delete;
    ThreadPoolScheduler& operator=(ThreadPoolScheduler&& other) = delete;

    void Start() override;
    void Stop() override;
    void EnqueueItem(std::function<void()>&& originalFunction) override;
    bool IsMethodInvocationSerialized() const override;

  private:
    SimpleThreadPool& _threadPool;
  };
}
