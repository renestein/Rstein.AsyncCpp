#pragma once
#include <cassert>

namespace RStein::AsyncCpp::Threading
{
  class SynchronizationContext;
  class SynchronizationContextScope
  {
  public:
    SynchronizationContextScope(SynchronizationContext& current);
    ~SynchronizationContextScope();

  private:
    SynchronizationContext* _synchronizationContext;
    SynchronizationContext* _oldContext;

    //Prevent instantiation of the class on the heap;
    void* operator new(size_t) noexcept
    {
      assert(false);
      return nullptr;
    }

    void* operator new[](size_t) noexcept
    {
      assert(false);
      return nullptr;
    }
    void operator delete(void*)
    {
        assert(false);
    }
    void operator delete[](void*)
    {
        assert(false);
    }
  };
  ;
}
