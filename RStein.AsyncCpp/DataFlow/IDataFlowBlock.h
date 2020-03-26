﻿#pragma once
#include <future>
#include <string>

namespace RStein::AsyncCpp::DataFlow
{
  class IDataFlowBlock
  {
    public:
      using TaskVoidType = std::shared_future<void>;
      IDataFlowBlock() = default;;
      IDataFlowBlock(const IDataFlowBlock& other) = delete;
      IDataFlowBlock(IDataFlowBlock&& other) = delete;
      IDataFlowBlock& operator=(const IDataFlowBlock& other) = delete;
      IDataFlowBlock& operator=(IDataFlowBlock&& other) = delete;
      [[nodiscard]] virtual std::string Name() const = 0;
      [[nodiscard]] virtual TaskVoidType Completion() const = 0;
      virtual void Start() = 0;
      virtual void Complete() = 0;
      virtual void SetFaulted(std::exception_ptr exception) = 0;
      virtual ~IDataFlowBlock() = default;
  };
}
