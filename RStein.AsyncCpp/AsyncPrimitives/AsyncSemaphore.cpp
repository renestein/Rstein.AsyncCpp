#include "AsyncSemaphore.h"

#include "FutureEx.h"
#include "OperationCanceledException.h"
#include "SemaphoreFullException.h"

#include <iostream>


using namespace std;

namespace RStein::AsyncCpp::AsyncPrimitives
{
  AsyncSemaphore::AsyncSemaphore(int maxCount, int initialCount) : _initialCount{ initialCount },
    _maxCount{ maxCount },
    _currentCount{ initialCount },
    _waiters{},
    _waitersLock{}
  {
    if (_maxCount < _initialCount || _maxCount < 0)
    {
      throw std::invalid_argument("_maxCount");
    }
    if (_initialCount < 0)
    {
      throw std::invalid_argument("_initialCount");
    }
  }

  AsyncSemaphore::~AsyncSemaphore()
  {
    Dispose();
  }

  future<void> AsyncSemaphore::WaitAsync()
  {
    return WaitAsync(CancellationToken::CancellationTokenPtr());
  }

  future<void> AsyncSemaphore::WaitAsync(const CancellationToken::CancellationTokenPtr& cancellationToken)
  {
    lock_guard lock{ _waitersLock };

    //cache completed future. Problem with shared_future + await;

    //We need to use promise in two contexts, so use make_shared for now as a workaround.
    SharedPromise newWaitingPromise = make_shared<promise<void>>();
    auto waiterFuture = newWaitingPromise->get_future();

    if (_currentCount > 0)
    {
      _currentCount--;
      newWaitingPromise->set_value();
      return waiterFuture;
    }

    optional<CancellationRegistration> cancelRegistration;
    if (cancellationToken && cancellationToken->CanBeCanceled())
    {
      cancelRegistration = cancellationToken->Register([newWaitingPromise, cancellationToken]
        {

          OperationCanceledException oce{};
          //Throw if the future is ready
          try
          {
            newWaitingPromise->set_exception(make_exception_ptr(oce));
          }
          catch (const future_error & error)
          {
            //Already fulfilled promise.
          }
        });
    }

    _waiters.emplace_back(pair(std::move(newWaitingPromise), std::move(cancelRegistration)));

    return waiterFuture;
  }

  void AsyncSemaphore::Release()
  {
    lock_guard lock{ _waitersLock };
    if (_currentCount == _maxCount)
    {
      throw SemaphoreFullException{};
    }

    if (_waiters.empty())
    {
      _currentCount++;
      return;
    }

    bool waiterReleased = false;
    do
    {
      auto [promise, cancellationRegistration] = std::move(_waiters.front());
      _waiters.pop_front();
      try
      {
        promise->set_value();
        waiterReleased = true;
        if (cancellationRegistration)
        {
          cancellationRegistration->Dispose();
        }
      }
      catch (const future_error & error)
      {
        cerr << "AsyncSemaphore::Release - Already fulfilled future.\n";
      }
      catch (...)
      {
        cerr << "AsyncSemaphore::Release - Unknown error";
      }

    } while (!waiterReleased && !_waiters.empty());

    if (_waiters.empty() && !waiterReleased)
    {
      _currentCount++;
    }
  }

  void AsyncSemaphore::Dispose()
  {
    //Do not lock in the destructor/Dispose.
    for (auto& [promise, cancellation] : _waiters)
    {
      try
      {
        promise->set_exception(make_exception_ptr(OperationCanceledException{}));
        //TODO: Dispose even in case the promise set_exception method throw
        if (cancellation)
        {
          cancellation->Dispose();
        }
      }
      catch(...)
      {
        
      }
    }

    _waiters.clear();
  }

}
