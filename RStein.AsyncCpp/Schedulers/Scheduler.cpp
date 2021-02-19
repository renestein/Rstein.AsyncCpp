#include "Scheduler.h"

#include "SimpleThreadPool.h"
#include "ThreadPoolScheduler.h"
#include "../Threading/SynchronizationContext.h"
#include "../Utils/FinallyBlock.h"

#if defined(__clang__)
#include "../ClangWinSpecific/Coroutine.h"
#elif defined(__cpp_impl_coroutine)
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

  std::unique_ptr<SimpleThreadPool> Scheduler::_defaultSchedulerThreadPool{};
  thread_local Scheduler::SchedulerPtr Scheduler::_currentScheduler{};
  Scheduler::SchedulerPtr Scheduler::_defaultScheduler{};
  once_flag Scheduler::_initSchedulerOnceFlag{};

  void Scheduler::initDefaultScheduler()
  {
    //TODO: Better ThreadPool
    static unsigned int MIN_THREADS = 8;
    static unsigned int HW_THREADS = std::thread::hardware_concurrency() * 2;
    const unsigned int THREADS_COUNT = max(MIN_THREADS, HW_THREADS);

    _defaultSchedulerThreadPool = make_unique<SimpleThreadPool>(THREADS_COUNT);
    _defaultSchedulerThreadPool->Start();
    _defaultScheduler = std::make_shared<ThreadPoolScheduler>(*_defaultSchedulerThreadPool);
    _defaultScheduler->Start();   
  }

  Scheduler::SchedulerPtr Scheduler::DefaultScheduler()
  {
    call_once(_initSchedulerOnceFlag, &initDefaultScheduler);  
    assert(_defaultScheduler);
    return _defaultScheduler;
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

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
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
