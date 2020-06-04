#pragma once
#include "CancellationToken.h"
#include "../Tasks/Task.h"
#include "../Tasks/TaskCompletionSource.h"


#include <deque>
#include <optional>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  class AsyncSemaphore final
  {
  public:
    AsyncSemaphore(int maxCount, int initialCount);
    void Dispose();
    AsyncSemaphore(const AsyncSemaphore& other) = delete;
    AsyncSemaphore(AsyncSemaphore&& other) noexcept = delete;
    AsyncSemaphore& operator=(const AsyncSemaphore& other) = delete;
    AsyncSemaphore& operator=(AsyncSemaphore&& other) noexcept = delete;
    ~AsyncSemaphore();

    [[nodiscard]] Tasks::Task<void> WaitAsync();
    [[nodiscard]] Tasks::Task<void> WaitAsync(CancellationToken cancellationToken);
    void Release();
        
  private:

    using SharedPromise = Tasks::TaskCompletionSource<void>;
    using WaiterPair = std::pair<SharedPromise, std::optional<CancellationRegistration>>;
    using Waiters = std::deque<WaiterPair>;
    const int _initialCount;
    const int _maxCount;
    int _currentCount;
    Waiters _waiters;
    std::mutex _waitersLock;
  };
}
