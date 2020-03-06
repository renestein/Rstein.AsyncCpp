#include "ThreadPoolScheduler.h"
#include "SimpleThreadPool.h"



ThreadPoolScheduler::ThreadPoolScheduler(SimpleThreadPool &threadPool) : m_threadPool(threadPool)
{

}


ThreadPoolScheduler::~ThreadPoolScheduler()
{
}

void ThreadPoolScheduler::Start()
{

}

void ThreadPoolScheduler::Stop()
{

}

void ThreadPoolScheduler::EnqueueItem(std::function<void()> &&originalFunction)
{
	m_threadPool.EnqueueItem(move(originalFunction));
}

bool ThreadPoolScheduler::IsMethodInvocationSerialized()
{
	return (m_threadPool.GetNumberOfThreads() == MAX_THREADS_IN_STRAND);
}
