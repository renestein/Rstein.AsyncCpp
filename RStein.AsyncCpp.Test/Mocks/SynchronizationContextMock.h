#pragma once
#include "../../RStein.AsyncCpp/Threading/SynchronizationContext.h"

#include <cassert>

namespace RStein::AsyncCpp::Mocks
{
  class TestSynchronizationContextMock : public RStein::AsyncCpp::Threading::SynchronizationContext
  {
  public:

    TestSynchronizationContextMock() = default;
    TestSynchronizationContextMock(const TestSynchronizationContextMock& other) = delete;
    TestSynchronizationContextMock(TestSynchronizationContextMock&& other) noexcept = delete;
    TestSynchronizationContextMock& operator=(const TestSynchronizationContextMock& other) = delete;
    TestSynchronizationContextMock& operator=(TestSynchronizationContextMock&& other) noexcept = delete;
    ~TestSynchronizationContextMock() = default;

    void Post(PostSendFunc postSendFunc) override
    {
      ++_postCallsCount;     
      SynchronizationContext::Post(postSendFunc);
    }

    void Send(PostSendFunc postSendFunc) override
    {
      ++_sendCallsCount;
      SynchronizationContext::Send(postSendFunc);
    }

    int PostCallsCount() const
    {
      return _postCallsCount;
    }

    int SendCallsCount() const
    {
      return _postCallsCount;
    }

    bool WasPostCalled() const
    {
      return _postCallsCount > 0;
    }

    bool WaSendCalled() const
    {
      return _sendCallsCount > 0;
    }

  private:
    volatile int _postCallsCount = 0;
    volatile int _sendCallsCount = 0;
  };
}
