#include "AsyncSemaphore.h"

#include "FutureEx.h"
#include "OperationCanceledException.h"
#include "SemaphoreFullException.h"
#include "../Tasks/TaskCombinators.h"
#include "../Utils/FinallyBlock.h"



#include <iostream>


using namespace std;
using namespace RStein::Utils;
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

  Tasks::Task<void> AsyncSemaphore::WaitAsync()
  {
    return WaitAsync(CancellationToken::None());
  }

  Tasks::Task<void> AsyncSemaphore::WaitAsync(CancellationToken cancellationToken)
  {
    lock_guard lock{ _waitersLock };

    if (_currentCount > 0)
    {
      _currentCount--;
      return Tasks::GetCompletedTask();
    }

    optional<CancellationRegistration> cancelRegistration;
    SharedPromise waiterPromise{};
    auto waiterTsk = waiterPromise.GetTask();
    if (cancellationToken.CanBeCanceled())
    {
      cancelRegistration = cancellationToken.Register([waiterPromise]() mutable
        {
          const OperationCanceledException oce{};

          waiterPromise.TrySetException(make_exception_ptr(oce));

        });
    }

    _waiters.emplace_back(pair(std::move(waiterPromise), std::move(cancelRegistration)));

    return waiterTsk;
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

    auto waiterReleased = false;
    do
    {
      auto [promise, cancellationRegistration] = std::move(_waiters.front());
      _waiters.pop_front();
      try
      {
        if (promise.TrySetResult())
        {
          waiterReleased = true;
        }
        if (cancellationRegistration)
        {
          cancellationRegistration->Dispose();
        }
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
    try
    {
      //Do not lock in the destructor/Dispose.
      for (auto& promiseCancellationPair: _waiters)
      {
        FinallyBlock finally([&promiseCancellationPair]()
          {
            if (promiseCancellationPair.second)
            {
                promiseCancellationPair.second->Dispose();
            }
          });

          promiseCancellationPair.first.TrySetException(make_exception_ptr(OperationCanceledException{}));        
      }

      _waiters.clear();
    }
    catch(...)
    {
      cerr << "AsyncSemaphore - exception in dtor.";
    }
  }

}
