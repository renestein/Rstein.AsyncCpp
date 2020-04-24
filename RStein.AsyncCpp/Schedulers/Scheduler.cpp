#include "Scheduler.h"

#include "SimpleThreadPool.h"
#include "ThreadPoolScheduler.h"
#include "../Utils/FinallyBlock.h"

#include <experimental/coroutine>
#include <algorithm>

using namespace std;
using namespace RStein::Utils;

namespace RStein::AsyncCpp::Schedulers
{
  Scheduler::Scheduler() = default;


  Scheduler::~Scheduler() = default;

  thread_local Scheduler::SchedulerPtr Scheduler::_currentScheduler = Scheduler::SchedulerPtr{};

  Scheduler::SchedulerPtr Scheduler::initDefaultScheduler()
  {
    //TODO: Better ThreadPool
    static unsigned int MIN_THREADS = 8;
    static unsigned int HW_THREADS = std::thread::hardware_concurrency() * 2;
    const int THREADS_COUNT = max(MIN_THREADS, HW_THREADS);

    static SimpleThreadPool threadPool{THREADS_COUNT};
    static SchedulerPtr defaultScheduler = std::make_shared<ThreadPoolScheduler>(threadPool);
    defaultScheduler->Start();
    return defaultScheduler;
  }

  //TODO: Change Create/Start/Stop of the default scheduler
  Scheduler::SchedulerPtr Scheduler::DefaultScheduler()
  {
     static SchedulerPtr defaultScheduler = initDefaultScheduler();
    return defaultScheduler;
  }

  void Scheduler::StopDefaultScheduler()
  {
    DefaultScheduler()->Stop();
  } 
 
  Scheduler::SchedulerPtr Scheduler::CurrentScheduler()
  {
    return _currentScheduler;
  }


  void Scheduler::EnqueueItem(std::function<void()>&& originalFunction)
  {
    
    FinallyBlock finally{[]{_currentScheduler = SchedulerPtr{};}};

    OnEnqueueItem([originalFunction = std::move(originalFunction), scheduler = shared_from_this()]()
    {
      _currentScheduler = scheduler;
      FinallyBlock finally{[]{_currentScheduler = SchedulerPtr{};}};
      originalFunction();
    });
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
