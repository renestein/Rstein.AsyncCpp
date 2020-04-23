#pragma once
#include <cassert>
#include <functional>
namespace RStein::Functional
{
  template<typename TCallable>
  auto Memoize(TCallable&& originalFunc)
  {
    assert(originalFunc != nullptr);

    return [originalFunc = std::forward<TCallable>(originalFunc)]
    {
         using Ret_Task_Type = decltype(originalFunc());
         static Ret_Task_Type retValue = originalFunc();
         return retValue;
    };
  }
}
