#include "ThreadPoolScheduler.h"
#include "SimpleThreadPool.h"

namespace RStein::AsyncCpp::Schedulers
{
  ThreadPoolScheduler::ThreadPoolScheduler(SimpleThreadPool& threadPool) : _threadPool(threadPool)
  {
  }


  ThreadPoolScheduler::~ThreadPoolScheduler() = default;

  void ThreadPoolScheduler::Start()
  {
    if (_threadPool.GetThreadPoolState() != SimpleThreadPool::ThreadPoolState::Started)
    {
      _threadPool.Start();
    }
  }

  void ThreadPoolScheduler::Stop()
  {
    if (_threadPool.GetThreadPoolState() != SimpleThreadPool::ThreadPoolState::Stopped)
    {
      _threadPool.Stop();
    }
  }

  void ThreadPoolScheduler::OnEnqueueItem(std::function<void()>&& originalFunction)
  {
    _threadPool.EnqueueItem(move(originalFunction));
  }

  bool ThreadPoolScheduler::IsMethodInvocationSerialized() const
  {
    return (_threadPool.GetNumberOfThreads() == MAX_THREADS_IN_STRAND);
  }
}
