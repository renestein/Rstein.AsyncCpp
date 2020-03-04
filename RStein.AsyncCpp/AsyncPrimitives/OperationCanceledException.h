#pragma once
#include "CancellationTokenSource.h"
#include <exception>


namespace RStein::AsyncCpp::AsyncPrimitives
{
  class OperationCanceledException final : public std::exception
  {
  public:

    OperationCanceledException() : OperationCanceledException(CancellationTokenSource::CancellationTokenSourcePtr())
    {
    }

    OperationCanceledException(const OperationCanceledException& other) = default;
    OperationCanceledException(OperationCanceledException&& other) noexcept = default;
    OperationCanceledException(CancellationTokenSource::CancellationTokenSourcePtr cts) : std::exception("Operation canceled."),
                                                                                          _cts(std::move(cts))
    {
      
    }
    OperationCanceledException& operator=(const OperationCanceledException& other) = default;
    OperationCanceledException& operator=(OperationCanceledException&& other) noexcept = default;
    ~OperationCanceledException() = default;
    CancellationTokenSource::CancellationTokenSourcePtr _cts;
  };

  
}
