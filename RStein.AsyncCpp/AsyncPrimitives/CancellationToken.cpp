#include "CancellationToken.h"

#include "CancellationTokenSource.h"
#include "OperationCanceledException.h"

#include <algorithm>
#include <cassert>
#include <iostream>


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
      cancellationRegistration._disposeAction = []
      {
      };
      return cancellationRegistration;
    }


    auto id = -1;
    {
      lock_guard lock{_cancelActionsMutex};

      if (IsCancellationRequested())
      {
        runCancelAction(cancellationAction);
        cancellationRegistration._disposeAction = []
        {
        };
        return cancellationRegistration;
      }

      id = ++_cancellationActionCounter;
      CancellationActionIdPair idActionPair{id, std::move(cancellationAction)};
      _cancelActions.push_back(idActionPair);
    }

    auto ptr = shared_from_this();
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


  void CancellationToken::runCancelActions()
  {
    // ReSharper disable once CppJoinDeclarationAndAssignment
    CancellationActionCollection cancelActionCollection;

    {
      std::lock_guard mutex{_cancelActionsMutex};
      // ReSharper disable once CppJoinDeclarationAndAssignment
      cancelActionCollection = std::move(_cancelActions);
    }

    for (auto& [_, action] : cancelActionCollection)
    {
      runCancelAction(action);
    }
  }

  void CancellationToken::runCancelAction(const CancellationAction& action) const
  {
    assert(action);
    try
    {
      action();
    }
    catch (...)
    {
      cerr << "Error when calling cancellation callback.";
    }
  }

  void CancellationToken::notifyCanceled()
  {
    assert(!_isCanceled);
    _isCanceled = true;

    runCancelActions();
  }

  void CancellationToken::removeCancellation(int id)
  {
    lock_guard lock{_cancelActionsMutex};
    _cancelActions.erase(remove_if(_cancelActions.begin(), _cancelActions.end(), [id](auto pair)
                         {
                           return pair.first == id;
                         }),
                         _cancelActions.end());
  }
}
