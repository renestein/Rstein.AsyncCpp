#pragma once
#include "Scheduler.h"
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>

namespace RStein::AsyncCpp::Schedulers
{
  class StrandSchedulerDecorator :	public Scheduler
  {
  public:
	  StrandSchedulerDecorator(const std::shared_ptr<Scheduler> &scheduler);
	  virtual ~StrandSchedulerDecorator();
	  void Start() override;
	  void Stop() override;
	  void EnqueueItem(std::function<void()> &&originalFunction) override;
	  bool IsMethodInvocationSerialized() override;
	  
	  
  private:
	  std::shared_ptr<Scheduler> _scheduler;
	  std::queue<std::function<void()>> _strandQueue;
	  std::mutex _queueMutex;
	  std::atomic<bool> _operationInProgress;

	  void markStrandOperationAsDone();
	  std::function<void()> wrapFunctionInStrand(std::function<void()> &&originalfunction);
	  void tryDequeItem();
	  void tryRunItem(std::function<void()> &&originalfunction);
	  void runOnOriginalScheduler(std::function<void()> &&originalfunction);
  };
}

