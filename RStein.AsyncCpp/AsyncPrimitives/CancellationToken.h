#pragma once
#include "CancellationRegistration.h"

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>


namespace RStein::AsyncCpp::AsyncPrimitives
{
  class CancellationTokenSource;
  class CancellationToken : private std::enable_shared_from_this<CancellationToken>
  {
    friend class CancellationTokenSource;

  public:

    using CancellationTokenPtr = std::shared_ptr<CancellationToken>;
    using CancellationAction = std::function<void()>;
    using CancellationActionIdPair = std::pair<int,CancellationAction>;
    using  CancellationActionCollection =  std::deque<CancellationActionIdPair>;
    CancellationToken(const CancellationToken& other) = delete;
    CancellationToken(CancellationToken&& other) noexcept = delete;
    CancellationToken& operator=(const CancellationToken& other) = delete;
    CancellationToken& operator=(CancellationToken&& other) noexcept = delete;
    ~CancellationToken() = default;
    void removeCancellation(int id);

    static CancellationTokenPtr None();
    bool CanBeCanceled() const;
    bool IsCancellationRequested() const;
    void ThrowIfCancellationRequested() const;
    CancellationRegistration Register(CancellationAction cancellationAction);
  private:
    CancellationToken();    
    static CancellationTokenPtr New();
    static CancellationToken::CancellationTokenPtr& getNoneRef();
    std::weak_ptr<CancellationTokenSource> _parent;
    std::atomic<bool> _isCanceled;
    mutable std::mutex _cancelActionsMutex;
    CancellationActionCollection _cancelActions;
    int _cancellationActionCounter;
    void NotifyCanceled();
    void removeCancellation(const CancellationAction& action);
  };
}
