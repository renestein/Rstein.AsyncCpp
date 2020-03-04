#include "CancellationRegistration.h"

#include <cassert>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  CancellationRegistration::CancellationRegistration() : _disposeAction()
  {

  }

  CancellationRegistration::CancellationRegistration(CancellationRegistration&& other) noexcept : _disposeAction()
  {
    std::swap(_disposeAction, other._disposeAction);
  }

  CancellationRegistration& CancellationRegistration::operator=(CancellationRegistration&& other) noexcept
  {
    std::swap(_disposeAction, other._disposeAction);
    return *this;
  }

  void CancellationRegistration::Dispose() const
  {
    assert(_disposeAction != nullptr);
    if (_disposeAction)
    {
      _disposeAction();
    }
  }

}
