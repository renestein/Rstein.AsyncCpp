#include "Scheduler.h"

#include <experimental/coroutine>

namespace RStein::AsyncCpp::Schedulers
{
  Scheduler::Scheduler()  = default;


  Scheduler::~Scheduler()  = default;


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
