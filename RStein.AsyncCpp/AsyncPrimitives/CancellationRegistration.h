#pragma once
#include <functional>
namespace RStein::AsyncCpp::AsyncPrimitives
{
  class CancellationRegistration final
  {
    friend class CancellationToken;
  public:
    CancellationRegistration(const CancellationRegistration& other) = delete;
    CancellationRegistration(CancellationRegistration&& other) noexcept;
    CancellationRegistration& operator=(const CancellationRegistration& other) = delete;
    CancellationRegistration& operator=(CancellationRegistration&& other) noexcept;
    ~CancellationRegistration() = default;
    void Dispose() const;
  private:
    CancellationRegistration();
    std::function<void()> _disposeAction;
  };
}
