#pragma once
#include <functional>

namespace RStein::AsyncCpp::Threading
{
  class SynchronizationContext
  {
  public:
    using PostSendFunc = std::function<void()>;
    SynchronizationContext() = default;
    SynchronizationContext(const SynchronizationContext& other) = delete;
    SynchronizationContext(SynchronizationContext&& other) noexcept = delete;
    SynchronizationContext& operator=(const SynchronizationContext& other) = delete;
    SynchronizationContext& operator=(SynchronizationContext&& other) noexcept = delete;
    virtual ~SynchronizationContext() = default;
    static SynchronizationContext* Current();

    virtual void Post(PostSendFunc postSendFunc);
    virtual void Send(PostSendFunc postSendFunc);
    virtual void OperationStarted();
    virtual void OperationCompleted();
    bool IsDefault();
    static SynchronizationContext* SetSynchronizationContext(SynchronizationContext* newContext);

    private:
    static SynchronizationContext* _default;
    static SynchronizationContext* _current;
  };
}

