#pragma once
#include "CancellationRegistration.h"
#include "../Detail/AsyncPrimitives/CtsSharedState.h"


#include <functional>
#include <memory>


namespace RStein::AsyncCpp::AsyncPrimitives
{
  class CancellationTokenSource;
  class CancellationToken final
  {
    friend class CancellationTokenSource;

  public:
    using CancellationAction = Detail::CtsSharedState::CancellationAction;
    CancellationToken(Detail::CtsSharedState::CtsSharedStatePtr sharedState);
    static CancellationToken None();

    CancellationToken(const CancellationToken& other) = default;
    CancellationToken(CancellationToken&& other) noexcept = default;
    CancellationToken& operator=(const CancellationToken& other) = default;
    CancellationToken& operator=(CancellationToken&& other) noexcept = default;
    ~CancellationToken() = default;

    bool CanBeCanceled() const;
    bool IsCancellationRequested() const;
    void ThrowIfCancellationRequested() const;
    CancellationRegistration Register(CancellationAction cancellationAction) const;
  private:
    static CancellationToken _none;
    Detail::CtsSharedState::CtsSharedStatePtr _sharedState;
    
  };
}
