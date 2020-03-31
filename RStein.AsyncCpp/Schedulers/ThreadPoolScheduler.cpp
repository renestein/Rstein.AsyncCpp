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
  }

  void ThreadPoolScheduler::Stop()
  {
  }

  void ThreadPoolScheduler::EnqueueItem(std::function<void()>&& originalFunction)
  {
    _threadPool.EnqueueItem(move(originalFunction));
  }

  bool ThreadPoolScheduler::IsMethodInvocationSerialized() const
  {
    return (_threadPool.GetNumberOfThreads() == MAX_THREADS_IN_STRAND);
  }
}
