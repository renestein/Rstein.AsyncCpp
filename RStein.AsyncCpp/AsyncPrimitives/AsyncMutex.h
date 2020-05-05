#pragma once
#include "AsyncSemaphore.h"
#include "../Tasks/Task.h"

namespace RStein::AsyncCpp::AsyncPrimitives
{
  class AsyncMutex;

  struct AsyncLockGuard
  {
    friend class AsyncMutex;
    AsyncLockGuard(const AsyncLockGuard& other) = delete;
    AsyncLockGuard(AsyncLockGuard&& other) noexcept;
    AsyncLockGuard& operator=(const AsyncLockGuard& other) = delete;
    AsyncLockGuard& operator=(AsyncLockGuard&& other) noexcept = delete;
    ~AsyncLockGuard();
    void Unlock();

    auto operator co_await() const
    {
      assert(_lockTaskPtr.load() != nullptr);
      return _lockTaskPtr.load()->operator co_await();
    }

  private:
    explicit AsyncLockGuard(AsyncMutex* parentLock, Tasks::Task<void>&& lockTask);
    void unlockNow(bool throwIfAlreadyUnlocked);
    AsyncMutex* _parentLock;
    Tasks::Task<void> _lockTask;
    std::atomic<Tasks::Task<void>*> _lockTaskPtr;
  };

  class AsyncMutex
  {
    friend struct AsyncLockGuard;

  public:
    AsyncMutex();
    AsyncMutex(const AsyncMutex& other) = delete;
    AsyncMutex(AsyncMutex&& other) noexcept = delete;
    AsyncMutex& operator=(const AsyncMutex& other) = delete;
    AsyncMutex& operator=(AsyncMutex&& other) noexcept = delete;
    [[nodiscard]] AsyncLockGuard Lock();
    void Unlock(AsyncLockGuard& uniqueLock);

  private:
    AsyncSemaphore _asyncSemaphore;
  };
}
