#pragma once
#include <algorithm>
#include <cassert>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

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
    std::optional<T> tryPopInner();
    std::optional<T> TryPop();
    std::vector<T> PopAll();

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
     std::lock_guard lock{_mutex};
    _innerQueue.push(item);
  }

  template <typename T>
  void ThreadSafeMinimalisticQueue<T>::Push(T&& item)
  {
    std::lock_guard lock{_mutex};
    _innerQueue.push(std::forward<T>(item));
  }


  template <typename T>
  std::optional<T> ThreadSafeMinimalisticQueue<T>::TryPop()
  {
    std::lock_guard lock{_mutex};
    return tryPopInner();
  }

  template <typename T>
  std::vector<T> ThreadSafeMinimalisticQueue<T>::PopAll()
  {
    std::lock_guard lock{_mutex};
    auto itemsCount = _innerQueue.size();
    std::vector<T> retVector{};
    retVector.reserve(itemsCount);

    for (size_t i = 0; i < itemsCount; ++i)
    {
      auto optionalValue = tryPopInner();
      assert(optionalValue);
      retVector.push_back(optionalValue.value());
    }

    return retVector;
  }

  
  template <typename T>
  std::optional<T> ThreadSafeMinimalisticQueue<T>::tryPopInner()
  {
    if (_innerQueue.empty())
    {
      return {};
    }

    auto item = _innerQueue.front();
    _innerQueue.pop();
    return std::optional{item};
  }
}

