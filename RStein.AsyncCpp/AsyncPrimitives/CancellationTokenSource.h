#pragma once
#include "CancellationToken.h"
#include <atomic>
#include <memory>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  class CancellationTokenSource final : public std::enable_shared_from_this<CancellationTokenSource>
  {
  public:
    using CancellationTokenSourcePtr = std::shared_ptr<CancellationTokenSource>;
    static CancellationTokenSourcePtr Create();

   
    CancellationTokenSource(const CancellationTokenSource& other) = delete;
    CancellationTokenSource(CancellationTokenSource&& other) noexcept = delete;
    CancellationTokenSource& operator=(const CancellationTokenSource& other) = delete;
    CancellationTokenSource& operator=(CancellationTokenSource&& other) noexcept = delete;

    ~CancellationTokenSource() = default;

    void Cancel();
    bool IsCancellationRequested() const;
    CancellationToken::CancellationTokenPtr Token() const;

  private:
    CancellationTokenSource();
    CancellationToken::CancellationTokenPtr _token;
    std::atomic<bool> _isCancellationRequested;
  };
}
