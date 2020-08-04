#pragma once
#include "../Tasks/Task.h"

namespace RStein::AsyncCpp::Actors
{

  template<typename TMessage>
class IActor
{
  public:

    IActor() = default;
    IActor(const IActor& other) = delete;
    IActor(IActor&& other) noexcept = delete;
    IActor& operator=(const IActor& other) = delete;
    IActor& operator=(IActor&& other) noexcept = delete;
    virtual ~IActor() = default;
    virtual void Tell(TMessage message) = 0;
    virtual Tasks::Task<void> Completion() = 0;
    virtual void Complete() = 0;
  };
}
