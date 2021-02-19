#pragma once

#include "../Utils/FinallyBlock.h"
#if defined(__clang__)
#include "../ClangWinSpecific/Coroutine.h"
#elif defined(__cpp_impl_coroutine) 
#include <coroutine>
#else
#include <experimental/resumable>
#endif
#include <memory>
#include <functional>
#include <xcall_once.h>

namespace RStein::AsyncCpp::Schedulers
{
  class SimpleThreadPool;

class Scheduler : public std::enable_shared_from_this<Scheduler>
{
  public:

    using SchedulerPtr = std::shared_ptr<Scheduler>;
	  Scheduler();
	  virtual ~Scheduler() = 0;

    Scheduler(const Scheduler& other) = delete;
    Scheduler(Scheduler&& other) = delete;
    Scheduler& operator=(const Scheduler& other) = delete;
    Scheduler& operator=(Scheduler&& other) = delete;

    static SchedulerPtr DefaultScheduler();
    static void StopDefaultScheduler();
    static SchedulerPtr CurrentScheduler();
    static SchedulerPtr FromCurrentSynchronizationContext();
	  virtual void Start() = 0;
	  virtual void Stop() = 0;

    template<typename TFunc>
	  void EnqueueItem(TFunc originalFunction);
	  virtual bool IsMethodInvocationSerialized() const = 0 ;
    
    //awaiter members
    bool await_ready() const;
 
 #if defined(__cpp_impl_coroutine) && !defined(__clang__)
    bool await_suspend(std::coroutine_handle<> coroutine);
#else
    bool await_suspend(std::experimental::coroutine_handle<> coroutine);
#endif
    void await_resume() const;   
    //end awaiter members
protected:
    virtual void OnEnqueueItem(std::function<void()>&& originalFunction) = 0;
private:
    static std::unique_ptr<SimpleThreadPool> _defaultSchedulerThreadPool;
    static thread_local SchedulerPtr _currentScheduler;
    static SchedulerPtr _defaultScheduler;
    static std::once_flag _initSchedulerOnceFlag;
    static void initDefaultScheduler(); 
};

template <typename TFunc>
void Scheduler::EnqueueItem(TFunc originalFunction)
{
  Utils::FinallyBlock finally{[]{_currentScheduler = SchedulerPtr{};}};

    OnEnqueueItem([originalFunction = std::move(originalFunction), scheduler = shared_from_this()]() mutable
    {
      _currentScheduler = scheduler;
      Utils::FinallyBlock finally{[]{_currentScheduler = SchedulerPtr{};}};
      originalFunction();
    });
}
}

