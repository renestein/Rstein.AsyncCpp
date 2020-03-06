#pragma once
#include "Scheduler.h"

class SimpleThreadPool;
class ThreadPoolScheduler :
	public Scheduler
{
public:
	static const int MAX_THREADS_IN_STRAND = 1;

	ThreadPoolScheduler(SimpleThreadPool &threadPool);
	virtual ~ThreadPoolScheduler();
	void Start() override;
	void Stop() override;
	void EnqueueItem(std::function<void()> &&originalFunction) override;
	bool IsMethodInvocationSerialized() override;

private:
	SimpleThreadPool &m_threadPool;
};

