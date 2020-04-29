#pragma once
#include "IInputBlock.h"

namespace RStein::AsyncCpp::DataFlow
{
  template<typename TInputItem, typename TOutputItem>
  class IInputOutputBlock : public IInputBlock<TInputItem>
  {
  public:
        using IInputOutputBlockPtr = std::shared_ptr<IInputOutputBlock<TInputItem, TOutputItem>>;
        using OutputType = TOutputItem;
        using TaskOutputItemType = Tasks::Task<OutputType>;
        IInputOutputBlock() = default;
        IInputOutputBlock(const IInputOutputBlock& other) = delete;
        IInputOutputBlock(IInputOutputBlock&& other) = delete;
        IInputOutputBlock& operator=(const IInputOutputBlock& other) = delete;
        IInputOutputBlock& operator=(IInputOutputBlock&& other) = delete;
        virtual ~IInputOutputBlock() = default;
        
        virtual void ConnectTo(const typename IInputBlock<TOutputItem>::InputBlockPtr& nextBlock) = 0;
             
        template<typename TNextBlock>
        const std::shared_ptr<TNextBlock>& Then(const std::shared_ptr<TNextBlock>& nextBlock)
        {
          ConnectTo(nextBlock);
          return nextBlock;
        }
  };
}