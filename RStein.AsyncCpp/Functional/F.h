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
    if constexpr (std::is_reference_v<Ret_Type>)
    {
      std::remove_reference_t<Ret_Type>* retTypePtr = nullptr;
      return [originalFunc = std::forward<TCallable>(originalFunc), retTaskTypePtr = retTypePtr, wasMethodCalled = false]() mutable ->Ret_Type
      {
        if (!wasMethodCalled)
        {
          Ret_Type value = originalFunc();
          retTaskTypePtr = &value;
          wasMethodCalled = true;
        }
        return *retTaskTypePtr;
      };
    }
    else
    {
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
}
