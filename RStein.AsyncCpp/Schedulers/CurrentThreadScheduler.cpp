#include "CurrentThreadScheduler.h"
#include <stdexcept>

using namespace std;

namespace RStein::AsyncCpp::Schedulers
{
  CurrentThreadScheduler::CurrentThreadScheduler() = default;
                                                            

  CurrentThreadScheduler::~CurrentThreadScheduler() = default;



  void CurrentThreadScheduler::EnqueueItem(std::function<void()>&& func)
  {
    if (!func)
    {
      invalid_argument funcExc("func");
      throw funcExc;
    }

     func();
  }

  bool CurrentThreadScheduler::IsMethodInvocationSerialized() const
  {
    return false;
  }
}
