#include "CancellationToken.h"


#include "CancellationTokenSource.h"

#include "OperationCanceledException.h"
#include <algorithm>
#include <cassert>


using namespace std;

namespace RStein::AsyncCpp::AsyncPrimitives
{
  CancellationToken::CancellationToken() : std::enable_shared_from_this<CancellationToken>(),
                                           _isCanceled(false),
                                           _cancelActionsMutex(),
                                           _cancelActions(),
                                           _cancellationActionCounter(0)
  {
  }

  CancellationToken::CancellationTokenPtr CancellationToken::None()
  {
    return getNoneRef();
  }

  bool CancellationToken::CanBeCanceled() const
  {
    return this != getNoneRef().get();
  }

  bool CancellationToken::IsCancellationRequested() const
  {
    return _isCanceled;
  }

  void CancellationToken::ThrowIfCancellationRequested() const
  {
    if (_isCanceled)
    {
      const auto parent = _parent.lock();
      throw OperationCanceledException(parent);
    }
  }

  CancellationRegistration CancellationToken::Register(CancellationAction cancellationAction)
  {
    CancellationRegistration cancellationRegistration;
    if (!cancellationAction || !CanBeCanceled())
    {
      cancellationRegistration._disposeAction = []{};
      return cancellationRegistration;
    }

    auto id = -1;
    {
      lock_guard lock{_cancelActionsMutex};
      auto id = _cancellationActionCounter++;
      CancellationActionIdPair idActionPair{id, std::move(cancellationAction)};
      _cancelActions.push_back(idActionPair);
    }
    weak_ptr<CancellationToken> weakThis = shared_from_this();
    assert(id >= 0);

    cancellationRegistration._disposeAction = [weakThis, id]()
    {
      auto sharedThis = weakThis.lock();
      if (!sharedThis)
      {
        return;
      }

      sharedThis->removeCancellation(id);
    };


    return cancellationRegistration;
  }

  CancellationToken::CancellationTokenPtr CancellationToken::New()
  {
    return shared_ptr<CancellationToken>(new CancellationToken);
  }

  CancellationToken::CancellationTokenPtr& CancellationToken::getNoneRef()
  {
    static CancellationTokenPtr None = New();
    return None;
  }

  void CancellationToken::NotifyCanceled()
  {
    _isCanceled = true;
  }

  void CancellationToken::removeCancellation(int id)
  {
    lock_guard lock{_cancelActionsMutex};
    _cancelActions.erase(remove_if(_cancelActions.begin(), _cancelActions.end(), [id](auto pair) {return pair.first == id;}),
                _cancelActions.end());
  }
}
