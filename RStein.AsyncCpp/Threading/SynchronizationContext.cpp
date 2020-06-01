#include "SynchronizationContext.h"

#include <stdexcept>

namespace RStein::AsyncCpp::Threading
{
  SynchronizationContext* SynchronizationContext::_default = new SynchronizationContext;
  SynchronizationContext* SynchronizationContext::_current = _default;

  SynchronizationContext* SynchronizationContext::Current()
  {
    return _current;
  }

  void SynchronizationContext::Post(PostSendFunc postSendFunc)
  {
    if (!postSendFunc)
    {
      throw std::invalid_argument{"postSendFunc"};
    }

    postSendFunc();
  }

  void SynchronizationContext::Send(PostSendFunc postSendFunc)
  {
    if (!postSendFunc)
    {
      throw std::invalid_argument{"postSendFunc"};
    }

    postSendFunc();
  }

  void SynchronizationContext::OperationStarted()
  {
  }

  void SynchronizationContext::OperationCompleted()
  {
  }

  bool SynchronizationContext::IsDefault()
  {
    return this == _default;
  }

  SynchronizationContext* SynchronizationContext::SetSynchronizationContext(SynchronizationContext* newContext)
  {
 
    auto* oldContext = _current;
    _current  = newContext == nullptr
                ? _default
                : newContext;
    return oldContext;

  }
}
