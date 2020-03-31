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
  //TODO: Start/Stop is not thread safe.
  //TODO: Better Lock free + work setealing ThreadPool
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
    SimpleThreadPool(int numberOfThreads);   
    virtual ~SimpleThreadPool();

    SimpleThreadPool(const SimpleThreadPool& other) = delete;
    SimpleThreadPool(SimpleThreadPool&& other) = delete;
    SimpleThreadPool& operator=(const SimpleThreadPool& other) = delete;
    SimpleThreadPool& operator=(SimpleThreadPool&& other) = delete;

    void Start();
    void Stop();
    
    void EnqueueItem(WorkItem&& originalFunction);
    int GetNumberOfThreads() const;
    ThreadPoolState GetThreadPoolState();

  private:
    std::queue<WorkItem> _innerQueue;
    std::mutex _lockRoot;
    std::condition_variable _queueConditionVariable;
    ThreadPoolState _threadPoolState;
    int _numberOfThreads;

    std::vector<std::thread> _threads;
    std::atomic<bool> _quitRequest;

    void throwInvalidThreadPoolState();
  };
}
