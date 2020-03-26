#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <atomic>

//Predpoklada se, ze metody Start a Stop nebudou klientem volany ve stejnou dobu - konkurence neni osetrena

class SimpleThreadPool
{
public:
	
	enum class ThreadPoolState
	{
		Created,
		Started,
		Stopped
	};

	using WorkItem = std::function < void() > ;
	SimpleThreadPool();
	SimpleThreadPool(int numberOfThreads);
	SimpleThreadPool(const SimpleThreadPool&) = delete;
	SimpleThreadPool& operator=(const SimpleThreadPool&) = delete;
	virtual ~SimpleThreadPool();
	void Start();
	void Stop();
	//Dopiste si metodu, ktera prijima LVAlue
	void EnqueueItem(WorkItem &&originalFunction);
	int GetNumberOfThreads() const;
	SimpleThreadPool::ThreadPoolState GetThreadPoolState();

private:	
	std::queue<WorkItem> m_innerQueue;
	std::mutex m_lockRoot;
	std::condition_variable m__queueConditionVariable;
	ThreadPoolState m_threadPoolState;
	int m_numberOfThreads;

	std::vector<std::thread> m_threads;
	std::atomic<bool> m_quitRequest;

	void throwInvalidThreadPoolState();

};

