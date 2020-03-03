#pragma once
#include <exception>
namespace RStein::AsyncCpp::AsyncPrimitives
{
  class SemaphoreFullException final : public std::exception
  {
    public:

    SemaphoreFullException() : std::exception("Semaphore is full.")
    {
      
    }
    SemaphoreFullException(const SemaphoreFullException& other) = default;
    SemaphoreFullException(SemaphoreFullException&& other) noexcept = default;
    SemaphoreFullException& operator=(const SemaphoreFullException& other) = default;
    SemaphoreFullException& operator=(SemaphoreFullException&& other) noexcept = default;
    ~SemaphoreFullException() = default;
  }; 
}

