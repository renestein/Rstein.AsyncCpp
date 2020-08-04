#pragma once
#include "../Tasks/Task.h"

namespace RStein::AsyncCpp::Actors
{
  template<typename TMessage, typename TResult>
  class IReplyActor
  {
    
  public:
    IReplyActor() = default;

    IReplyActor(const IReplyActor& other) = delete;
    IReplyActor(IReplyActor&& other) noexcept = delete;
    IReplyActor& operator=(const IReplyActor& other) = delete;
    IReplyActor& operator=(IReplyActor&& other) noexcept = delete;
    virtual ~IReplyActor() = default;

    virtual Tasks::Task<TResult> Ask(TMessage message) = 0;
    virtual Tasks::Task<void> Completion() = 0;
    virtual void Complete() = 0;
  };
}

