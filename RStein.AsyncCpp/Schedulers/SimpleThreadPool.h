#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <atomic>

namespace RStein::AsyncCpp::Schedulers
{
  //TODO: Better Lock free + work stealing ThreadPool
  //TODO: Start/Stop is not thread safe.
  class SimpleThreadPool
  {
  public:

    enum class ThreadPoolState
    {
      Created,
      Started,
      Stopped
    };

    using WorkItem = std::function<void()>;
    SimpleThreadPool();
    SimpleThreadPool(unsigned int numberOfThreads); 
    virtual ~SimpleThreadPool();

    SimpleThreadPool(const SimpleThreadPool& other) = delete;
    SimpleThreadPool(SimpleThreadPool&& other) = delete;
    SimpleThreadPool& operator=(const SimpleThreadPool& other) = delete;
    SimpleThreadPool& operator=(SimpleThreadPool&& other) = delete;

    void Start();
    void Stop();
    
    void EnqueueItem(WorkItem originalFunction);
    unsigned GetNumberOfThreads() const;
    ThreadPoolState GetThreadPoolState() const;

  private:
    std::queue<WorkItem> _innerQueue;
    std::mutex _lockRoot;
    std::condition_variable _queueConditionVariable;
    ThreadPoolState _threadPoolState;
    unsigned int _numberOfThreads;

    std::vector<std::thread> _threads;
    std::atomic<bool> _quitRequest;

    void throwInvalidThreadPoolState();
  };
}
