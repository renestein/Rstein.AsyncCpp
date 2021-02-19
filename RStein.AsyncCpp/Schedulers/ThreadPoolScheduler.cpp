#include "ThreadPoolScheduler.h"
#include "SimpleThreadPool.h"

namespace RStein::AsyncCpp::Schedulers
{
  ThreadPoolScheduler::ThreadPoolScheduler(SimpleThreadPool& threadPool) : _threadPool(threadPool)
  {
  }


  ThreadPoolScheduler::~ThreadPoolScheduler()
  {
    try
    {
      stopThreadPool();
    }
    catch(...)
    {
      
    }
  };

  void ThreadPoolScheduler::Start()
  {
    if (_threadPool.GetThreadPoolState() != SimpleThreadPool::ThreadPoolState::Started)
    {
      _threadPool.Start();
    }
  }

  void ThreadPoolScheduler::stopThreadPool() const
  {
    if (_threadPool.GetThreadPoolState() != SimpleThreadPool::ThreadPoolState::Stopped)
    {
      _threadPool.Stop();
    }
  }

  void ThreadPoolScheduler::Stop()
  {
    stopThreadPool();
  }

  void ThreadPoolScheduler::OnEnqueueItem(std::function<void()>&& originalFunction)
  {
    _threadPool.EnqueueItem(move(originalFunction));
  }

  bool ThreadPoolScheduler::IsMethodInvocationSerialized() const
  {
    return _threadPool.GetNumberOfThreads() == MAX_THREADS_IN_STRAND;
  }
}
