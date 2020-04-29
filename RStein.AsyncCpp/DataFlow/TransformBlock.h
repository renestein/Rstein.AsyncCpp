#pragma once
#include "IInputOutputBlock.h"
#include "../Detail/DataFlow/DataFlowBlockCommon.h"
#include <memory>

namespace RStein::AsyncCpp::DataFlow
{
  template<typename TInputItem, typename TOutputItem, typename TState=Detail::NoState>
  class TransformBlock : public IInputOutputBlock<TInputItem, TOutputItem>,
                         public std::enable_shared_from_this<TransformBlock<TInputItem, TOutputItem, TState>>

  {

  private:
    using InnerDataFlowBlock = Detail::DataFlowBlockCommon<TInputItem, TOutputItem, TState>;
    using InnerDataFlowBlockPtr = typename InnerDataFlowBlock::DataFlowBlockCommonPtr;

  public:
    explicit TransformBlock(typename InnerDataFlowBlock::TransformFuncType transformFunc, typename InnerDataFlowBlock::CanAcceptFuncType canAcceptFunc = [] (auto _){return true;});
    explicit TransformBlock(typename InnerDataFlowBlock::AsyncTransformFuncType transformFunc, typename InnerDataFlowBlock::CanAcceptFuncType canAcceptFunc = [] (auto _){return true;});
    TransformBlock(const TransformBlock& other) = delete;
    TransformBlock(TransformBlock&& other) = delete;
    TransformBlock& operator=(const TransformBlock& other) = delete;
    TransformBlock& operator=(TransformBlock&& other) = delete;

      [[nodiscard]] std::string Name() const override;
      void Name(std::string name);  
      [[nodiscard]] IDataFlowBlock::TaskVoidType Completion() const override;
      void Start() override;
      void Complete() override;
      void SetFaulted(std::exception_ptr exception) override;
      bool CanAcceptInput(const TInputItem& item) override;

      typename IDataFlowBlock::TaskVoidType AcceptInputAsync(const TInputItem& item) override;
      IDataFlowBlock::TaskVoidType AcceptInputAsync(TInputItem&& item) override;

      void ConnectTo(const typename IInputBlock<TOutputItem>::InputBlockPtr& nextBlock) override;
      virtual ~TransformBlock() = default;
 
  private:
   
    InnerDataFlowBlockPtr _innerBlock;
  };

  template <typename TInputItem, typename TOutputItem, typename TState>
  TransformBlock<TInputItem, TOutputItem, TState>::TransformBlock(typename InnerDataFlowBlock::TransformFuncType transformFunc,
                                                                  typename InnerDataFlowBlock::CanAcceptFuncType canAcceptFunc) : IInputOutputBlock<TInputItem, TOutputItem>{},
                                                                                                                                  std::enable_shared_from_this<TransformBlock<TInputItem, TOutputItem, TState>>{},
                                                                                                                                  _innerBlock{std::make_shared<InnerDataFlowBlock>(transformFunc, canAcceptFunc)}
  {

  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  TransformBlock<TInputItem, TOutputItem, TState>::TransformBlock(
      typename InnerDataFlowBlock::AsyncTransformFuncType transformFunc,
      typename InnerDataFlowBlock::CanAcceptFuncType canAcceptFunc) : IInputOutputBlock<TInputItem, TOutputItem>{},
                                                                     std::enable_shared_from_this<TransformBlock<TInputItem, TOutputItem, TState>>{},
                                                                     _innerBlock{std::make_shared<InnerDataFlowBlock>(transformFunc, canAcceptFunc)}
  {

  }


  template <typename TInputItem, typename TOutputItem, typename TState>
  std::string TransformBlock<TInputItem, TOutputItem, TState>::Name() const
  {
    return _innerBlock->Name();
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void TransformBlock<TInputItem, TOutputItem, TState>::Name(std::string name)
  {
    _innerBlock->Name(name);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  IDataFlowBlock::TaskVoidType TransformBlock<
    TInputItem, TOutputItem, TState>::Completion() const
  {
    return _innerBlock->Completion();
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void TransformBlock<TInputItem, TOutputItem, TState>::Start()
  {
    _innerBlock ->Start();
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void TransformBlock<TInputItem, TOutputItem, TState>::Complete()
  {
    return _innerBlock->Complete();
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void TransformBlock<TInputItem, TOutputItem, TState>::SetFaulted(std::exception_ptr exception)
  {
    _innerBlock->SetFaulted(exception);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  bool TransformBlock<TInputItem, TOutputItem, TState>::CanAcceptInput(const TInputItem& item)
  {
    return _innerBlock->CanAcceptInput(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  RStein::AsyncCpp::DataFlow::IDataFlowBlock::TaskVoidType TransformBlock<
    TInputItem, TOutputItem, TState>::AcceptInputAsync(const TInputItem& item)
  {
    return _innerBlock->AcceptInputAsync(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  IDataFlowBlock::TaskVoidType TransformBlock<TInputItem, TOutputItem, TState>::AcceptInputAsync(TInputItem&& item)
  {
    return _innerBlock->AcceptInputAsync(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void TransformBlock<TInputItem, TOutputItem, TState>::ConnectTo(
      const typename RStein::AsyncCpp::DataFlow::IInputBlock<TOutputItem>::InputBlockPtr& nextBlock)
  {
    _innerBlock->Then(nextBlock);
  }

}