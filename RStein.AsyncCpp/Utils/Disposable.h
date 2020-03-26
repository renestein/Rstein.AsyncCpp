#pragma once
#include <memory>

namespace RStein
{
  namespace Utils
  {
    class Disposable
    {
    public:
      Disposable();
      virtual void Dispose() = 0;
      virtual ~Disposable() = default;
    };

    typedef std::shared_ptr<Disposable> DisposablePtr;
  }
}
