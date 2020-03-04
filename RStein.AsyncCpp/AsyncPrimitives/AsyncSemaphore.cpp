#include "AsyncSemaphore.h"
#include "FutureEx.h"
#include "OperationCanceledException.h"
#include "SemaphoreFullException.h"


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
    auto operationCanceledExceptionPtr = make_exception_ptr(OperationCanceledException{});
    //Do not lock in destructor.
    for (auto&& promise : _waiters)
    {
      //Assumed unfulfilled promise!
      promise.set_exception(operationCanceledExceptionPtr);
    }

    _waiters.clear();
  }

  future<void> AsyncSemaphore::WaitAsync()
  {
    lock_guard lock{ _waitersLock };

    //cache completed future. Problem with shared_future + await;

    promise<void> newWaitingPromise;
    auto waiterFuture = newWaitingPromise.get_future();
    if (_currentCount > 0)
    {
      _currentCount--;
      newWaitingPromise.set_value();
      return waiterFuture;
    }

    _waiters.push_back(std::move(newWaitingPromise));

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

    auto promise = std::move(_waiters.front());
    _waiters.pop_front();
    promise.set_value();
  }
}
