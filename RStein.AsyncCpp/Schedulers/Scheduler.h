#pragma once
#include <memory>
#include <functional>

namespace RStein::AsyncCpp::Schedulers
{
  

class Scheduler : public std::enable_shared_from_this<Scheduler>
{
  public:
	  Scheduler();
	  virtual ~Scheduler();
	  virtual void Start() = 0;
	  virtual void Stop() = 0;
	  virtual void EnqueueItem(std::function<void()> &&originalFunction) = 0;
	  virtual bool IsMethodInvocationSerialized() = 0;

    //awaiter members
    bool await_ready() const;
    bool await_suspend();
    void await_resume() const;   
    //end awaiter members

  };
}

