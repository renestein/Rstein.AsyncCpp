#pragma once

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

namespace RStein::AsyncCpp::Detail
{
  class CtsSharedState : public std::enable_shared_from_this<CtsSharedState>
  {
    
  public:

    using CtsSharedStatePtr = std::shared_ptr<CtsSharedState>;
    using CancellationAction = std::function<void()>;
    using CancellationActionIdPair = std::pair<int,CancellationAction>;
    using  CancellationActionCollection =  std::deque<CancellationActionIdPair>;    

    CtsSharedState();    
    CtsSharedState(const CtsSharedState& other) = delete;
    CtsSharedState(CtsSharedState&& other) noexcept = delete;
    CtsSharedState& operator=(const CtsSharedState& other) = delete;
    CtsSharedState& operator=(CtsSharedState&& other) noexcept = delete;
    ~CtsSharedState() = default;
    
    bool IsCancellationRequested() const;
    void ThrowIfCancellationRequested() const;

    std::function<void()> Register(CancellationAction cancellationAction);

    static CtsSharedStatePtr New();
    void RunCancelAction(const CancellationAction& action) const;
    void RunCancelActions();
    void NotifyCanceled();
    void RemoveCancellation(int id);

  private:
    std::atomic<bool> _isCanceled;
    mutable std::mutex _cancelActionsMutex;
    CancellationActionCollection _cancelActions;
    int _cancellationActionCounter;
    
  };
}
