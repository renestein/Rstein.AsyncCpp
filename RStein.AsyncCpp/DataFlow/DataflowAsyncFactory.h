#pragma once
#include "ActionBlock.h"
#include "Detail/DataFlowBlockCommon.h"
#include "IInputBlock.h"
#include "../AsyncPrimitives/FutureEx.h"
#include <future>
namespace RStein::AsyncCpp::DataFlow
{
class DataFlowAsyncFactory
{
  public:
    template<typename TInput, typename TState>
      static typename IInputBlock<TInput>::InputBlockPtr CreateActionBlock(typename Detail::DataFlowBlockCommon<TInput, Detail::NoOutput, TState>::AsyncActionFuncType actionFunc,
                                                                           typename Detail::DataFlowBlockCommon<TInput, Detail::NoOutput, TState>::CanAcceptFuncType canAcceptFunc = [](auto& _){return true;})
      {
        return std:: make_shared<ActionBlock<TInput, TState>>(std::move(actionFunc), std::move(canAcceptFunc));
      }

      template<typename TInput>
      static typename IInputBlock<TInput>::InputBlockPtr CreateActionBlock(std::function<std::shared_future<void>(const TInput& input)> actionFunc,
                                                                          typename Detail::DataFlowBlockCommon<TInput, Detail::NoOutput, Detail::NoState>::CanAcceptFuncType canAcceptFunc = [](auto& _){return true;})
      {
        return CreateActionBlock<TInput, Detail::NoState>([actionFunc=std::move(actionFunc)] (const TInput& input, auto _)->std::shared_future<void> {co_await actionFunc(input);},
                                                          std::move(canAcceptFunc));
      }
  };
}
