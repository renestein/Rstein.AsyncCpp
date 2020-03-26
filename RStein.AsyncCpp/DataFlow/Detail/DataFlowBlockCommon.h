#pragma once
#include "../IInputBlock.h"

namespace RStein::AsyncCpp::DataFlow::Detail
{
  template<typename TInputItem, typename TOutputItem, typename TState=void>
  class DataFlowBlockCommon : IDataFlowBlock
  {
  public:
      [[nodiscard]] virtual std::string Name() const = 0;
      [[nodiscard]] virtual TaskVoidType Completion() const = 0;
      virtual void Start();
      virtual void Complete();
      virtual void SetFaulted(std::exception_ptr exception);
      virtual ~DataFlowBlockCommon();
  };
}


