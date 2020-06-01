#include "SynchronizationContextScope.h"

#include "SynchronizationContext.h"

namespace RStein::AsyncCpp::Threading
{
  SynchronizationContextScope::SynchronizationContextScope(SynchronizationContext& current) : _synchronizationContext(&current),
                                                                                               _oldContext(SynchronizationContext::SetSynchronizationContext(_synchronizationContext))

  {
  }

  SynchronizationContextScope::~SynchronizationContextScope()
  {
    SynchronizationContext::SetSynchronizationContext(_oldContext);
  }
}
