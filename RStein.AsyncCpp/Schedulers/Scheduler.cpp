#include "Scheduler.h"

#include "SimpleThreadPool.h"
#include "ThreadPoolScheduler.h"
#include "../Threading/SynchronizationContext.h"
#include "../Utils/FinallyBlock.h"

#ifdef __cpp_impl_coroutine
#include <coroutine>
#else
#include <experimental/coroutine>
#endif
#include <algorithm>

#include <cassert>

using namespace std;
using namespace RStein::Utils;

namespace RStein::AsyncCpp::Schedulers
{
  Scheduler::Scheduler() = default;


  Scheduler::~Scheduler() = default;

  thread_local Scheduler::SchedulerPtr Scheduler::_currentScheduler = SchedulerPtr{};

  Scheduler::SchedulerPtr Scheduler::initDefaultScheduler()
  {
    //TODO: Better ThreadPool
    static unsigned int MIN_THREADS = 8;
    static unsigned int HW_THREADS = std::thread::hardware_concurrency() * 2;
    const unsigned int THREADS_COUNT = max(MIN_THREADS, HW_THREADS);

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


  bool Scheduler::await_ready() const
  {
    return false;
  }

#ifdef __cpp_impl_coroutine
  bool Scheduler::await_suspend(std::coroutine_handle<> coroutine)
#else
  bool Scheduler::await_suspend(std::experimental::coroutine_handle<> coroutine)
#endif
  {
    EnqueueItem(coroutine);
    return true;
  }

  void Scheduler::await_resume() const
  {
  }

  class SynchronizationContextScheduler : public Scheduler
  {
    public:

    SynchronizationContextScheduler(Threading::SynchronizationContext* synchronizationContext): _synchronizationContext(synchronizationContext)
    {
      if (!synchronizationContext)
      {
        throw invalid_argument("synchronizationContext");
      }
    }

    SynchronizationContextScheduler(const SynchronizationContextScheduler& other) = delete;

    SynchronizationContextScheduler(SynchronizationContextScheduler&& other) noexcept = delete;

    SynchronizationContextScheduler& operator=(const SynchronizationContextScheduler& other) = delete;

    SynchronizationContextScheduler& operator=(SynchronizationContextScheduler&& other) noexcept = delete;
    ~SynchronizationContextScheduler() = default;
    void Start() override
    {
      
    }
    void Stop() override
    {
      
    }
    bool IsMethodInvocationSerialized() const override
    {
      return false;
    };

  protected:
    void OnEnqueueItem(std::function<void()>&& originalFunction) override
    {
      assert(_synchronizationContext);
      _synchronizationContext->Post(std::move(originalFunction));
    }

  private:
    Threading::SynchronizationContext* _synchronizationContext;
  };

  Scheduler::SchedulerPtr Scheduler::FromCurrentSynchronizationContext()
  {
    auto* synchronizationContext = Threading::SynchronizationContext::Current();
    assert(synchronizationContext != nullptr);
    SchedulerPtr scheduler = make_shared<SynchronizationContextScheduler>(synchronizationContext);
    scheduler->Start();
    return scheduler;
  }
  
}
