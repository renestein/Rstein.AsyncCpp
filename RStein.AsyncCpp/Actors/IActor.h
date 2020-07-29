#pragma once

template<typename TMessage>
class IActor
{
public:
  virtual ~IActor() = default;

  IActor() = default;

  IActor(const IActor& other) = delete;
  IActor(IActor&& other) noexcept = delete;
  IActor& operator=(const IActor& other) = delete;
  IActor& operator=(IActor&& other) noexcept = delete;
  virtual void Tell(TMessage message) = 0;
};
