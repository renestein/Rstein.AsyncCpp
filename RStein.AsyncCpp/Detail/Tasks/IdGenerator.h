#pragma once
#include <atomic>

namespace Detail
{
  template<typename T>
  struct IdGenerator
  {
     static std::atomic<unsigned long> Counter;
  };

  template<typename T>
  std::atomic<unsigned long> IdGenerator<T>::Counter{};
}
