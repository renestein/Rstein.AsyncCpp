#include "AsyncMutex.h"

namespace RStein::AsyncCpp::AsyncPrimitives
{
  AsyncLockGuard::AsyncLockGuard(AsyncLockGuard&& other) noexcept: _parentLock{other._parentLock},
                                                                   _lockTask(std::move(other._lockTask)),
                                                                   _lockTaskPtr{&_lockTask}
  {
    other._lockTaskPtr.exchange(nullptr);
    assert(_parentLock);
    assert(_lockTaskPtr);
  }

  AsyncLockGuard::~AsyncLockGuard()
  {
    unlockNow(false);
  }

  void AsyncLockGuard::Unlock()
  {
    unlockNow(true);
  }

  AsyncLockGuard::AsyncLockGuard(AsyncMutex* parentLock, Tasks::Task<void>&& lockTask) : _parentLock{parentLock},
                                                                                         _lockTask{std::move(lockTask)},
                                                                                         _lockTaskPtr{&_lockTask}
  {
    assert(_parentLock != nullptr);
    assert(_lockTaskPtr != nullptr);
  }

  void AsyncLockGuard::unlockNow(bool throwIfAlreadyUnlocked)
  {
    if (auto taskPtr = _lockTaskPtr.exchange(nullptr); !taskPtr)
    {
      if (throwIfAlreadyUnlocked)
      {
        throw std::logic_error{"Attempt to unlock already unlocked or moved AsyncLockGuard."};
      }

      return;
    }

    _parentLock->_asyncSemaphore.Release();
  }

  AsyncMutex::AsyncMutex(): _asyncSemaphore{1, 1}
  {
  }

  AsyncLockGuard AsyncMutex::Lock()
  {
    auto lockTask = _asyncSemaphore.WaitAsync();
    return AsyncLockGuard{this, (std::move(lockTask))};
  }

  void AsyncMutex::Unlock(AsyncLockGuard& uniqueLock)
  {
    if (uniqueLock._parentLock != this)
    {
      throw std::logic_error("Foreign AsyncLockGuard could not be unlocked.");
    }

    uniqueLock.unlockNow(true);
  }
}
