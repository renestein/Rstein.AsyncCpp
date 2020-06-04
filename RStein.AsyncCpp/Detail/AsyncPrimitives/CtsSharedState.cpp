#include "CtsSharedState.h"

#include "../../AsyncPrimitives/OperationCanceledException.h"

#include <cassert>
#include <iostream>

using namespace std;
namespace RStein::AsyncCpp::Detail
{
  
    CtsSharedState::CtsSharedState() : std::enable_shared_from_this<CtsSharedState>(),
                                        _isCanceled(false),
                                        _cancelActionsMutex(),
                                        _cancelActions(),
                                        _cancellationActionCounter(0)
  {
  }

 
  bool CtsSharedState::IsCancellationRequested() const
  {
    return _isCanceled;
  }

  void CtsSharedState::ThrowIfCancellationRequested() const
  {
    if (_isCanceled)
    {
      throw AsyncPrimitives::OperationCanceledException();
    }
  }


   CtsSharedState::CtsSharedStatePtr CtsSharedState::New()
  {
    return make_shared<CtsSharedState>();
  }

  void CtsSharedState::RunCancelActions()
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
      RunCancelAction(action);
    }
  }

  void CtsSharedState::RunCancelAction(const CancellationAction& action) const
  {
    assert(action);
    try
    {
      action();
    }
    catch (...)
    {
      std::cerr << "Error when calling cancellation callback.";
    }
  }

  void CtsSharedState::NotifyCanceled()
  {
    auto canceledNow = !_isCanceled.exchange(true);

    if (canceledNow)
    {
      RunCancelActions();
    }
  }

  void CtsSharedState::RemoveCancellation(int id)
  {
    lock_guard lock{_cancelActionsMutex};
    _cancelActions.erase(remove_if(_cancelActions.begin(), _cancelActions.end(), [id](auto pair)
                         {
                           return pair.first == id;
                         }),
                         _cancelActions.end());
  }

  std::function<void()> CtsSharedState::Register(CancellationAction cancellationAction)
  {
    if (!cancellationAction)
    {
      return []{};
    }


    auto id = -1;
    {
      std::lock_guard lock{_cancelActionsMutex};

      if (IsCancellationRequested())
      {
        RunCancelAction(cancellationAction);

        return []{};
      }

      id = ++_cancellationActionCounter;
      CancellationActionIdPair idActionPair{id, std::move(cancellationAction)};
      _cancelActions.push_back(idActionPair);
    }

    auto ptr = shared_from_this();
    std::weak_ptr<CtsSharedState> weakThis = shared_from_this();
    assert(id >= 0);

    return [weakThis, id]()
    {
      auto sharedThis = weakThis.lock();
      if (!sharedThis)
      {
        return;
      }

      sharedThis->RemoveCancellation(id);
    };
  }
}
