#pragma once
#include <cassert>
#include <functional>
namespace RStein::Functional
{
  //Not thread safe
  template<typename TCallable>
  auto Memoize0(TCallable&& originalFunc)
  {
    assert(originalFunc != nullptr);
    using Ret_Type = decltype(originalFunc());
    return [originalFunc = std::forward<TCallable>(originalFunc), retTaskType = Ret_Type(), wasMethodCalled = false]() mutable
    {
      if (!wasMethodCalled)
      {
        retTaskType = originalFunc();
        wasMethodCalled = true;
      }
      return retTaskType;
    };
  }
}
