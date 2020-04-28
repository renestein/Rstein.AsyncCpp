#pragma once
#include "CancellationToken.h"
#include <memory>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  class CancellationTokenSource
  {
  public:

    CancellationTokenSource();
    CancellationTokenSource(const CancellationTokenSource& other) = default;
    CancellationTokenSource(CancellationTokenSource&& other) noexcept = default;
    CancellationTokenSource& operator=(const CancellationTokenSource& other) = default;
    CancellationTokenSource& operator=(CancellationTokenSource&& other) noexcept = default;

    ~CancellationTokenSource() = default;

    void Cancel() const;
    bool IsCancellationRequested() const;
    CancellationToken Token() const;

  private:
    using CtsSharedStatePtr = Detail::CtsSharedState::CtsSharedStatePtr;
    CtsSharedStatePtr _sharedState;
    CancellationToken _token;
  };
}
