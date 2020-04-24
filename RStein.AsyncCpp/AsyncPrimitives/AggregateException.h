#pragma once
#include <exception>
#include <utility>
#include <vector>

class AggregateException : std::exception
{
public:
  explicit AggregateException(std::vector<std::exception_ptr> exceptions)
    : _exceptions(std::move(exceptions))
  {
  }

  AggregateException(const AggregateException& other) = default;
  AggregateException(AggregateException&& other) noexcept = default;
  AggregateException& operator=(const AggregateException& other) = default;
  AggregateException& operator=(AggregateException&& other) noexcept = default;
  ~AggregateException() = default;

  [[nodiscard]] std::vector<std::exception_ptr> Exceptions() const
  {
    return _exceptions;
  }
  std::exception_ptr FirstExceptionPtr() const
  {
    return !_exceptions.empty()
             ? _exceptions[0]
             : nullptr;
  }
private:
  std::vector<std::exception_ptr> _exceptions;
};
