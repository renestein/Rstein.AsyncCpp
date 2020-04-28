#pragma once
#include "IDataFlowBlock.h"

#include <future>
#include <memory>

namespace RStein::AsyncCpp::DataFlow
{
    template<typename TInputITem>
    class IInputBlock : public IDataFlowBlock
    {
      public:
        using InputBlockPtr = std::shared_ptr<IInputBlock>;
        using InputType = TInputITem;
        using TaskInputItemType = std::shared_future<InputType>;
        IInputBlock() = default;
        IInputBlock(const IInputBlock& other) = delete;
        IInputBlock(IInputBlock&& other) = delete;
        IInputBlock& operator=(const IInputBlock& other) = delete;
        IInputBlock& operator=(IInputBlock&& other) = delete;
        virtual ~IInputBlock() = default;
        virtual bool CanAcceptInput(const InputType& item) = 0;
        virtual TaskVoidType AcceptInputAsync(const InputType& item) = 0;
        virtual TaskVoidType AcceptInputAsync(InputType&& item) = 0;
        
    };
}
