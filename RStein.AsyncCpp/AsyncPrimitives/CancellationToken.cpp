#include "CancellationToken.h"

#include <utility>


using namespace std;

namespace RStein::AsyncCpp::AsyncPrimitives
{
  CancellationToken::CancellationToken(Detail::CtsSharedState::CtsSharedStatePtr sharedState) : _sharedState(std::move(sharedState))
  {
  }

  CancellationToken CancellationToken::_none = CancellationToken(nullptr);
  CancellationToken CancellationToken::None()
  {
    return _none;
  }

  bool CancellationToken::CanBeCanceled() const
  {
    return _sharedState.get() != nullptr;
  }

  bool CancellationToken::IsCancellationRequested() const
  {
    if (_sharedState)
    {
      return _sharedState->IsCancellationRequested();
    }
    return false;
  }

  void CancellationToken::ThrowIfCancellationRequested() const
  {
    if (!_sharedState)
    {
      return;
    }

    _sharedState->ThrowIfCancellationRequested();

  }

  CancellationRegistration CancellationToken::Register(CancellationAction cancellationAction) const
  {
    CancellationRegistration cancellationRegistration;
    if (!_sharedState)
    {      
      cancellationRegistration._disposeAction = []{};
      return cancellationRegistration;
    }

    cancellationRegistration._disposeAction = _sharedState->Register(std::move(cancellationAction));
    return cancellationRegistration;
  }
}
