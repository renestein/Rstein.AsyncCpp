#include "Scheduler.h"

#include "SimpleThreadPool.h"
#include "ThreadPoolScheduler.h"
#include <experimental/coroutine>
#include <algorithm>

using namespace std;

namespace RStein::AsyncCpp::Schedulers
{
  Scheduler::Scheduler() = default;


  Scheduler::~Scheduler() = default;

  thread_local Scheduler::SchedulerPtr Scheduler::_currentScheduler = Scheduler::SchedulerPtr{};

  Scheduler::SchedulerPtr Scheduler::DefaultScheduler()
  {
    //TODO: Better ThreadPool
    static unsigned int MIN_THREADS = 8;
    static unsigned int HW_THREADS = std::thread::hardware_concurrency() * 2;
    const int THREADS_COUNT = max(MIN_THREADS, HW_THREADS);

    static SimpleThreadPool threadPool{THREADS_COUNT};
    static SchedulerPtr defaultScheduler = std::make_shared<ThreadPoolScheduler>(threadPool);
    return defaultScheduler;
  }


  Scheduler::SchedulerPtr Scheduler::CurrentScheduler()
  {
    return _currentScheduler;
  }


  bool Scheduler::await_ready() const
  {
    return false;
  }

  bool Scheduler::await_suspend(std::experimental::coroutine_handle<> coroutine)
  {
    EnqueueItem(coroutine);
    return true;
  }

  void Scheduler::await_resume() const
  {
  }
}
