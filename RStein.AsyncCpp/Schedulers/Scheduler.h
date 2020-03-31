#pragma once
#include <experimental/resumable>
#include <memory>
#include <functional>
#include <thread>

namespace RStein::AsyncCpp::Schedulers
{
  

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
    static SchedulerPtr CurrentScheduler();
	  virtual void Start() = 0;
	  virtual void Stop() = 0;
	  virtual void EnqueueItem(std::function<void()>&& originalFunction) = 0;
	  virtual bool IsMethodInvocationSerialized() const = 0 ;
    

    //awaiter members
    bool await_ready() const;
    bool await_suspend(std::experimental::coroutine_handle<> coroutine);
    void await_resume() const;   
    //end awaiter members

private:
    static thread_local SchedulerPtr _currentScheduler;
  };
}

