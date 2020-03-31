#include "Scheduler.h"
namespace RStein::AsyncCpp::Schedulers
{
  Scheduler::Scheduler()  = default;


  Scheduler::~Scheduler()  = default;


  bool Scheduler::await_ready() const
  {
    return false;
  }

  bool Scheduler::await_suspend()
  {
    return false;
  }

  void Scheduler::await_resume() const
  {
  }
}
