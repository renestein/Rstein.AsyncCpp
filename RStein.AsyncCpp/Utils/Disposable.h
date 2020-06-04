#pragma once
#include <memory>

namespace RStein::Utils
{
  class IDisposable
  {
  public:
    IDisposable() = default;

    IDisposable(const IDisposable& other) = default;
    IDisposable(IDisposable&& other) noexcept = default;
    IDisposable& operator=(const IDisposable& other) = default;
    IDisposable& operator=(IDisposable&& other) noexcept = default;
    virtual ~IDisposable() = default;
    virtual void Dispose() = 0;
  };

  typedef std::shared_ptr<IDisposable> DisposablePtr;
}
