#pragma once
#include "Scheduler.h"
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>

class StrandSchedulerDecorator :
	public Scheduler
{
public:
	StrandSchedulerDecorator(const std::shared_ptr<Scheduler> &scheduler);
	virtual ~StrandSchedulerDecorator();
	void Start() override;
	void Stop() override;
	void EnqueueItem(std::function<void()> &&originalFunction) override;
	bool IsMethodInvocationSerialized() override;
	
	
private:
	std::shared_ptr<Scheduler> m_scheduler;
	std::queue<std::function<void()>> m_strandQueue;
	std::mutex m_queueMutex;
	std::atomic<bool> m_operationInProgress;

	void markStrandOperationAsDone();
	std::function<void()> wrapFunctionInStrand(std::function<void()> &&originalfunction);
	void tryDequeItem();
	void tryRunItem(std::function<void()> &&originalfunction);
	void runOnOriginalScheduler(std::function<void()> &&originalfunction);
};

