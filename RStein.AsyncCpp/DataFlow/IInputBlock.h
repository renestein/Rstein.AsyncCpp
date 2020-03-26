#pragma once
#include "IDataFlowBlock.h"
#include <future>

namespace RStein::AsyncCpp::DataFlow
{
    template<typename TInputITem>
    class IInputBlock : public IDataFlowBlock
    {
      public:
        using InputType = TInputITem;
        using TaskInputItemType = std::shared_future<InputType>;
        IInputBlock() = default;
        IInputBlock(const IInputBlock& other) = delete;
        IInputBlock(IInputBlock&& other) = delete;
        IInputBlock& operator=(const IInputBlock& other) = delete;
        IInputBlock& operator=(IInputBlock&& other) = delete;
        virtual ~IInputBlock() = default; 
        virtual TaskVoidType AcceptInputAsync(const InputType& item) = 0;
        virtual TaskVoidType AcceptInputAsync(InputType&& item) = 0;
        
    };
}
