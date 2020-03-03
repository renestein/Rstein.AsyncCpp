#pragma once
#include <deque>
#include <future>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  class AsyncSemaphore
  {
  public:
    AsyncSemaphore(int maxCount, int initialCount);
    AsyncSemaphore(const AsyncSemaphore& other) = delete;
    AsyncSemaphore(AsyncSemaphore&& other) noexcept = delete;
    AsyncSemaphore& operator=(const AsyncSemaphore& other) = delete;
    AsyncSemaphore& operator=(AsyncSemaphore&& other) noexcept = delete;
    ~AsyncSemaphore();

    std::shared_future<void> WaitAsync();
    void Release();
        
  private:
    static std::shared_future<void> _completedFuture;
    using Waiters = std::deque<std::promise<void>>;
    int _initialCount;
    int _maxCount;
    int _currentCount;
    Waiters _waiters;
    std::mutex _waitersLock;
  };
}
