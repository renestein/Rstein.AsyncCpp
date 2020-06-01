#pragma once

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
    void* operator new(size_t);
    void* operator new[](size_t);
    void operator delete(void*);
    void operator delete[](void*);
  };
  ;
}
