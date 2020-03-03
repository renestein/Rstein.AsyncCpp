#pragma once
#include <deque>
#include <future>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  class AsyncSemaphore final
  {
  public:
    AsyncSemaphore(int maxCount, int initialCount);
    AsyncSemaphore(const AsyncSemaphore& other) = delete;
    AsyncSemaphore(AsyncSemaphore&& other) noexcept = delete;
    AsyncSemaphore& operator=(const AsyncSemaphore& other) = delete;
    AsyncSemaphore& operator=(AsyncSemaphore&& other) noexcept = delete;
    ~AsyncSemaphore();

    [[nodiscard]]
    std::future<void> WaitAsync();
    void Release();
        
  private:
    using Waiters = std::deque<std::promise<void>>;
    const int _initialCount;
    const int _maxCount;
    int _currentCount;
    Waiters _waiters;
    std::mutex _waitersLock;
  };
}
