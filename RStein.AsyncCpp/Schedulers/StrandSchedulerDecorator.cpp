#include "StrandSchedulerDecorator.h"
#include <stdexcept>

using namespace std;
StrandSchedulerDecorator::StrandSchedulerDecorator(const std::shared_ptr<Scheduler> &scheduler) :
m_scheduler(scheduler),
m_strandQueue(),
m_queueMutex(),
m_operationInProgress(false)
																							
{
	if (!scheduler)
	{
		invalid_argument invalidSchedulerEx("scheduler");
		throw invalidSchedulerEx;
	}
}


StrandSchedulerDecorator::~StrandSchedulerDecorator()
{
}

void StrandSchedulerDecorator::Start()
{
	m_scheduler->Start();
}

void StrandSchedulerDecorator::Stop()
{
	m_scheduler->Stop();
}

void StrandSchedulerDecorator::EnqueueItem(std::function<void()> &&originalFunction)
{
	if (m_scheduler->IsMethodInvocationSerialized())
	{
		m_scheduler->EnqueueItem(move(originalFunction));
		return;
	}

	auto wrappedFunction = wrapFunctionInStrand(move(originalFunction));
	lock_guard<mutex> lock(m_queueMutex);
	
	tryRunItem(move(wrappedFunction));
}

bool StrandSchedulerDecorator::IsMethodInvocationSerialized()
{
	return true;
}

std::function<void()> StrandSchedulerDecorator::wrapFunctionInStrand(std::function<void()> &&originalFunction)
{
	return [originalFunction, this]
	{
		originalFunction();
		markStrandOperationAsDone();
	};
}

void StrandSchedulerDecorator::markStrandOperationAsDone()
{
	lock_guard<mutex> lock(m_queueMutex);
	m_operationInProgress.store(false);
	tryDequeItem();
}

void StrandSchedulerDecorator::tryDequeItem()
{
	if (m_strandQueue.empty())
	{
		return;
	}

	auto function = m_strandQueue.front();
	m_strandQueue.pop();
	tryRunItem(move(function));
}

void StrandSchedulerDecorator::tryRunItem(std::function<void()> &&originalfunction)
{
	if (!m_operationInProgress.load())
	{
		m_operationInProgress.store(true);
		runOnOriginalScheduler(move(originalfunction));
		return;
	}

	m_strandQueue.push(move(originalfunction));

}

void StrandSchedulerDecorator::runOnOriginalScheduler(std::function<void()> &&originalfunction)
{
	m_scheduler->EnqueueItem(move(originalfunction));
}
