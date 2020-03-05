#pragma once
#include <algorithm>
#include <mutex>
#include <optional>
#include <queue>

namespace RStein::AsyncCpp::Collections
{
  //Be aware of limitations of this class.
  template<typename T>
  class ThreadSafeMinimalisticQueue
  {
  public:
    using size_type = typename std::queue<T>::size_type;
    using reference = typename std::queue<T>::reference;
    using const_reference = typename std::queue<T>::const_reference;

    ThreadSafeMinimalisticQueue();
    ThreadSafeMinimalisticQueue(std::queue<T> innerQueue);
    ThreadSafeMinimalisticQueue(const ThreadSafeMinimalisticQueue& other) = delete;
    ThreadSafeMinimalisticQueue(ThreadSafeMinimalisticQueue&& other) noexcept = delete;
    ThreadSafeMinimalisticQueue& operator=(const ThreadSafeMinimalisticQueue& other) = delete;
    ThreadSafeMinimalisticQueue& operator=(ThreadSafeMinimalisticQueue&& other) noexcept = delete;
    virtual ~ThreadSafeMinimalisticQueue() = default;
    void Push(const T& item);
    void Push(T&& item);
    std::optional<T> TryPop();

  private:
    std::queue<T> _innerQueue;
    mutable std::mutex _mutex;

  };

  template <typename T>
  ThreadSafeMinimalisticQueue<T>::ThreadSafeMinimalisticQueue() : _innerQueue{},
                                                                  _mutex{}
  {

  }

  template <typename T>
  ThreadSafeMinimalisticQueue<T>::ThreadSafeMinimalisticQueue(std::queue<T> innerQueue) : _innerQueue{std::move(innerQueue)},
                                                                                          _mutex{}
 
  {
  }


  template <typename T>
  void ThreadSafeMinimalisticQueue<T>::Push(const T& item)
  {
     std::lock_guard{_mutex};
    _innerQueue.push(item);
  }

  template <typename T>
  void ThreadSafeMinimalisticQueue<T>::Push(T&& item)
  {
    std::lock_guard{_mutex};
    _innerQueue.push(std::move(item));
  }

  template <typename T>
  std::optional<T> ThreadSafeMinimalisticQueue<T>::TryPop()
  {
    std::lock_guard{_mutex};
    if (_innerQueue.empty())
    {
      return {};
    }

    auto item = _innerQueue.front();
    _innerQueue.pop();
    return std::optional{item};
  }
}

