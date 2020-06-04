#pragma once
#include "Scheduler.h"
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>

namespace RStein::AsyncCpp::Schedulers
{
  //TODO: Change locks, atomic etc.
  class StrandSchedulerDecorator :	public Scheduler
  {
  public:
    explicit StrandSchedulerDecorator(const std::shared_ptr<Scheduler>& scheduler);
	  virtual ~StrandSchedulerDecorator();

    StrandSchedulerDecorator(const StrandSchedulerDecorator& other) = delete;
    StrandSchedulerDecorator(StrandSchedulerDecorator&& other) = delete;
    StrandSchedulerDecorator& operator=(const StrandSchedulerDecorator& other) = delete;
    StrandSchedulerDecorator& operator=(StrandSchedulerDecorator&& other) = delete;

	  void Start() override;
	  void Stop() override;
	  bool IsMethodInvocationSerialized() const override;
	  
  protected:
    void OnEnqueueItem(std::function<void()> &&originalFunction) override;
  private:
	  std::shared_ptr<Scheduler> _scheduler;
	  std::queue<std::function<void()>> _strandQueue;
	  std::mutex _queueMutex;
	  std::atomic<bool> _operationInProgress;

	  void markStrandOperationAsDone();
	  std::function<void()> wrapFunctionInStrand(std::function<void()>&& originalFunction);
	  void tryDequeItem();
	  void tryRunItem(std::function<void()>&& originalFunction);
	  void runOnOriginalScheduler(std::function<void()>&& originalFunction) const;
  };
}

