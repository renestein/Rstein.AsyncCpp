#include "SimpleThreadPool.h"
#include <stdexcept>

using namespace std;

SimpleThreadPool::SimpleThreadPool() : SimpleThreadPool(thread::hardware_concurrency())
{

}

SimpleThreadPool::SimpleThreadPool(int numberOfThreads) : m_innerQueue(),
															m_lockRoot(),
															m__queueConditionVariable(),
															m_threadPoolState(ThreadPoolState::Created),
															m_numberOfThreads(numberOfThreads),
															m_quitRequest(false)

{
	if (numberOfThreads < 0)
	{
		invalid_argument numberOfThreadsInvalidEx("numberOfThreads");
		throw numberOfThreadsInvalidEx;
	}
}


SimpleThreadPool::~SimpleThreadPool()
{
	if (m_threadPoolState != ThreadPoolState::Stopped)
	{
		//Z destruktoru se nemaji vyvolavat vyjimky.
		//Bezpecnejsi? Stop - viz kurz
		/*throwInvalidThreadPoolState();*/
		//Log Invalid life cycle
	}
}

void SimpleThreadPool::Start()
{
	if (m_threadPoolState != ThreadPoolState::Created)
	{
		throwInvalidThreadPoolState();
	}

	for (int i = 0; i < m_numberOfThreads; i++)
	{
		m_threads.emplace_back([this]
		{
			do 
			{
				WorkItem currentWorkItem;
				
				{
					unique_lock<mutex> lock(m_lockRoot);
					while (!m_quitRequest.load() && m_innerQueue.empty())
					{
						m__queueConditionVariable.wait(lock);
					}

					if (m_quitRequest.load() && m_innerQueue.empty())
					{
						break;
					}

					currentWorkItem = move(m_innerQueue.front());
					m_innerQueue.pop();
				}

				try
				{
					currentWorkItem();
				}
				catch (...)
				{
					__debugbreak();
				}

			} while (!m_quitRequest.load());
		});
	}

	m_threadPoolState = ThreadPoolState::Started;
}

void SimpleThreadPool::Stop()
{
	if (m_threadPoolState != ThreadPoolState::Started)
	{
		throwInvalidThreadPoolState();
	}

	m_quitRequest.store(true);
	m__queueConditionVariable.notify_all();
	for (auto &thread : m_threads)
	{
		thread.join();
	}

	m_threadPoolState = ThreadPoolState::Stopped;
}

void SimpleThreadPool::EnqueueItem(WorkItem &&originalFunction)
{
	unique_lock<mutex> lock(m_lockRoot);
	m_innerQueue.push(move(originalFunction));
	m__queueConditionVariable.notify_one();
}

int SimpleThreadPool::GetNumberOfThreads() const
{
	return m_numberOfThreads;
}

SimpleThreadPool::ThreadPoolState SimpleThreadPool::GetThreadPoolState()
{
	return m_threadPoolState;
}

void SimpleThreadPool::throwInvalidThreadPoolState()
{
	throw std::logic_error("ThreadPool is in invalid state.");
}
