#pragma once
#include <algorithm>
#include <mutex>
#include <optional>
#include <vector>
#include <iterator>

namespace RStein::AsyncCpp::Collections
{
  //Be aware of limitations of this class.
  template<typename T>
  class ThreadSafeMinimalisticVector
  {
  public:
    using size_type = typename std::vector<T>::size_type;
    using value_type = typename std::vector<T>::value_type;
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using reverse_iterator = typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;
    explicit ThreadSafeMinimalisticVector(size_type count);
    explicit ThreadSafeMinimalisticVector(std::vector<T> innerVector);
    ThreadSafeMinimalisticVector(const ThreadSafeMinimalisticVector& other) = delete;
    ThreadSafeMinimalisticVector(ThreadSafeMinimalisticVector&& other) noexcept = delete;
    ThreadSafeMinimalisticVector& operator=(const ThreadSafeMinimalisticVector& other) = delete;
    ThreadSafeMinimalisticVector& operator=(ThreadSafeMinimalisticVector&& other) noexcept = delete;
    virtual ~ThreadSafeMinimalisticVector() = default;
    void Add(const T& item);
    void Add(T&& item);
    void Remove(const T& item);
    template <class TPredicate>
    void RemoveIf(TPredicate&& predicate);  
    void Reserve(size_type newCapacity);
    std::vector<T> GetSnapshot();
    template <typename TR, typename TMapFunc>
    auto MapSnapshot(TMapFunc&& mapFunc);
    void Clear(); 
    reference operator [] (int index);
    const_reference operator [] (int index) const;
    iterator begin() noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator cend() const noexcept;
    iterator rbegin() noexcept;
    const_iterator crbegin() const noexcept;
    iterator rend() noexcept;
    const_iterator crend() const noexcept;
  private:
    std::vector<T> _innerVector;
    mutable std::mutex _mutex;
  

  };

  template <typename T>
  ThreadSafeMinimalisticVector<T>::ThreadSafeMinimalisticVector(size_type count) : _innerVector{count},
                                                                                    _mutex{}
  {

  }

  template <typename T>
  ThreadSafeMinimalisticVector<T>::ThreadSafeMinimalisticVector(std::vector<T> innerVector) : _innerVector(std::move(innerVector)),
                                                                                              _mutex{}
 
  {
  }


  template <typename T>
  void ThreadSafeMinimalisticVector<T>::Add(const T& item)
  {
     std::lock_guard lock{_mutex};
    _innerVector.push_back(item);
  }

  template <typename T>
  void ThreadSafeMinimalisticVector<T>::Add(T&& item)
  {
    std::lock_guard lock{_mutex};
    _innerVector.push_back(std::forward<T>(item));
  }

  template <typename T>
  void ThreadSafeMinimalisticVector<T>::Remove(const T& item)
  {
     std::lock_guard lock{_mutex};
    _innerVector.erase(std::remove(_innerVector.begin(), _innerVector.end(), item), _innerVector.end());
  }

  template <typename T>
  template <typename TPredicate>
  void ThreadSafeMinimalisticVector<T>::RemoveIf(TPredicate&& predicate)

  {
     std::lock_guard lock{_mutex};
    _innerVector.erase(std::remove_if(_innerVector.begin(), _innerVector.end(), predicate), _innerVector.end());
  }

  template <typename T>
  void ThreadSafeMinimalisticVector<T>::Reserve(size_type newCapacity)
  {
    std::lock_guard lock{_mutex};
    _innerVector.reserve(newCapacity);
  }

  template <typename T>
  std::vector<T> ThreadSafeMinimalisticVector<T>::GetSnapshot()
  {
    std::lock_guard lock{_mutex};
    auto copy = _innerVector;
    return copy;
  }


  template <typename T>
  template <typename TR, typename TMapFunc>
  auto ThreadSafeMinimalisticVector<T>::MapSnapshot(TMapFunc&& mapFunc)
  {
    
    std::lock_guard lock{_mutex};
    std::vector<TR> mappedValues{};
    mappedValues.reserve(_innerVector.size());
    std::transform(_innerVector.begin(), _innerVector.end(), std::back_inserter(mappedValues), mapFunc);
    return mappedValues;
  }

  template <typename T>
  void ThreadSafeMinimalisticVector<T>::Clear()
  {
    std::lock_guard lock{_mutex};
    _innerVector.clear();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::iterator ThreadSafeMinimalisticVector<T>::begin() noexcept
  {
    std::lock_guard lock{_mutex};
    return _innerVector.begin();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::const_iterator ThreadSafeMinimalisticVector<T>::cbegin() const noexcept
  {
    std::lock_guard lock{_mutex};
    return  _innerVector.cbegin();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::iterator ThreadSafeMinimalisticVector<T>::end() noexcept
  {
    std::lock_guard lock{_mutex};
    return _innerVector.end();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::const_iterator ThreadSafeMinimalisticVector<T>::cend() const noexcept
  {
    std::lock_guard lock{_mutex};
    return _innerVector.cend();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::iterator ThreadSafeMinimalisticVector<T>::rbegin() noexcept
  {
    std::lock_guard lock{_mutex};
    return _innerVector.rbegin();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::const_iterator ThreadSafeMinimalisticVector<T>::crbegin() const noexcept
  {
    std::lock_guard lock{_mutex};
    return _innerVector.crbegin();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::iterator ThreadSafeMinimalisticVector<T>::rend() noexcept
  {
    std::lock_guard lock{_mutex};
    return _innerVector.rend();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::const_iterator ThreadSafeMinimalisticVector<T>::crend() const noexcept
  {
    std::lock_guard lock{_mutex};
    return _innerVector.crend();
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::reference ThreadSafeMinimalisticVector<T>::operator[](int index)
  {
    std::lock_guard lock{_mutex};
    return _innerVector.at(index);
  }

  template <typename T>
  typename ThreadSafeMinimalisticVector<T>::const_reference ThreadSafeMinimalisticVector<T>::operator[](int index) const
  {
    std::lock_guard lock{_mutex};
    return _innerVector.at(index);
  }
}

