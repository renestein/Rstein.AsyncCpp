#pragma once
#include "IInputBlock.h"

namespace RStein::AsyncCpp::DataFlow
{
  template<typename TInputItem, typename TOutputItem>
  class IInputOutputBlock : public IInputBlock<TInputItem>
  {
  public:
        using IInputOutputBlockPtr = std::shared_ptr<IInputOutputBlock>;
        using OutputType = TOutputItem;
        using TaskOutputItemType = std::shared_future<OutputType>;
        IInputOutputBlock() = default;
        IInputOutputBlock(const IInputOutputBlock& other) = delete;
        IInputOutputBlock(IInputOutputBlock&& other) = delete;
        IInputOutputBlock& operator=(const IInputOutputBlock& other) = delete;
        IInputOutputBlock& operator=(IInputOutputBlock&& other) = delete;
        virtual ~IInputOutputBlock() = default;
        
        virtual void ConnectTo(const typename IInputBlock<TOutputItem>::InputBlockPtr& nextBlock) = 0;
    };
}