#include "CurrentThreadScheduler.h"
#include <stdexcept>

using namespace std;

namespace RStein::AsyncCpp::Schedulers
{
  CurrentThreadScheduler::CurrentThreadScheduler() = default;
                                                            

  CurrentThreadScheduler::~CurrentThreadScheduler() = default;

  void CurrentThreadScheduler::Start()
  {
  }

  void CurrentThreadScheduler::Stop()
  {
  }

  void CurrentThreadScheduler::OnEnqueueItem(std::function<void()>&& func)
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
