#pragma once
#include <exception>


namespace RStein::AsyncCpp::AsyncPrimitives
{
  class OperationCanceledException final : public std::exception
  {
  public:


    OperationCanceledException() : std::exception("Operation canceled.")
    {
      
    }

    OperationCanceledException(const OperationCanceledException& other) = default;

    OperationCanceledException(OperationCanceledException&& other) noexcept = default;
    OperationCanceledException& operator=(const OperationCanceledException& other) = default;
    OperationCanceledException& operator=(OperationCanceledException&& other) noexcept = default;
    ~OperationCanceledException() = default;

  };

  
}
