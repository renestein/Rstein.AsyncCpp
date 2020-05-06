#pragma once
namespace RStein::Functional
{
  template<typename TResult>
  struct DefaultValueCreator
  {
    TResult operator()()
    {
      return TResult{};
    }
  };

  template <typename TResult>
  struct Result_Traits
  {
    using Result_Type = TResult;
    using Default_Value_Func = DefaultValueCreator<TResult>;
  };
  
  //Not thread safe
  template<typename TCallable>
  auto Memoize0(TCallable&& originalFunc)
  {    
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
      return [originalFunc = std::forward<TCallable>(originalFunc), retTaskType = typename Result_Traits<Ret_Type>::Default_Value_Func{} (), wasMethodCalled = false]() mutable
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
