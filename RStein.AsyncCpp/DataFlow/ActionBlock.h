#pragma once
#include "../Detail/DataFlow/DataFlowBlockCommon.h"
#include <memory>

namespace RStein::AsyncCpp::DataFlow
{

  template<typename TInputItem, typename TState = Detail::NoState>
  class ActionBlock : public IInputBlock<TInputItem>,
                      public std::enable_shared_from_this<ActionBlock<TInputItem, TState>>
          
  {

    private:
      using InnerDataFlowBlock = Detail::DataFlowBlockCommon<TInputItem, Detail::NoOutput, TState>;
      using InnerDataFlowBlockPtr = typename InnerDataFlowBlock::DataFlowBlockCommonPtr;
    

  public:

     ActionBlock(typename InnerDataFlowBlock::AsyncActionFuncType actionFunc,
                  typename InnerDataFlowBlock::CanAcceptFuncType canAcceptFunc = [](const auto& _){ return true;}) :
                                                                                  IInputBlock<TInputItem>{},
                                                                                  std::enable_shared_from_this<ActionBlock<TInputItem, TState>>{},
                                                                                  _innerBlock{std::make_shared<InnerDataFlowBlock>([actionFunc](const TInputItem& inputItem, TState*& state) ->Tasks::Task<Detail::NoOutput>
                                                                                              {
                                                                                                co_await actionFunc(inputItem, state).ConfigureAwait(false);
                                                                                                co_return Detail::NoOutput::Default();
                                                                                              },
                                                                                              canAcceptFunc)}
                                                                             
      {
        
      }

     ActionBlock(//TODO: Avoid unused variable, ambiguous ctor
               
                  typename InnerDataFlowBlock::ActionFuncType actionFunc,
                  typename InnerDataFlowBlock::CanAcceptFuncType canAcceptFunc = [](const auto& _){ return true;}) :
                                                                                  IInputBlock<TInputItem>{},
                                                                                  std::enable_shared_from_this<ActionBlock<TInputItem, TState>>{},
                                                                                  _innerBlock{std::make_shared<InnerDataFlowBlock>([actionFunc](const TInputItem& inputItem, TState*& state)
                                                                                              {
                                                                                                actionFunc(inputItem, state);
                                                                                                return Detail::NoOutput::Default();
                                                                                              },
                                                                                              canAcceptFunc)}
                                                                             
      {
        
      }
      ActionBlock(const ActionBlock& other) = delete;
      ActionBlock(ActionBlock&& other) = delete;
      ActionBlock& operator=(const ActionBlock& other) = delete;
      ActionBlock& operator=(ActionBlock&& other) = delete;
      virtual ~ActionBlock() = default;
        
      
      [[nodiscard]] std::string Name() const override;
      void Name(std::string name);  
      [[nodiscard]] typename IDataFlowBlock::TaskVoidType Completion() const override;
      void Start() override;
      void Complete() override;
      void SetFaulted(std::exception_ptr exception) override;
      bool CanAcceptInput(const TInputItem& item) override;
      typename IDataFlowBlock::TaskVoidType AcceptInputAsync(const TInputItem& item) override;
      typename IDataFlowBlock::TaskVoidType AcceptInputAsync(TInputItem&& item) override;
      

     private:
      

     
      InnerDataFlowBlockPtr _innerBlock;
  };

  template <typename TInputItem, typename TState>
  std::string ActionBlock<TInputItem, TState>::Name() const
  {
    return _innerBlock->Name();
  }

  template <typename TInputItem, typename TState>
  void ActionBlock<TInputItem, TState>::Name(std::string name)
  {
    _innerBlock->Name(name);
  }

  template <typename TInputItem, typename TState>
  IDataFlowBlock::TaskVoidType ActionBlock<TInputItem, TState>::Completion() const
  {
    return _innerBlock->Completion();
  }

  template <typename TInputItem, typename TState>
  void ActionBlock<TInputItem, TState>::Start()
  {
    _innerBlock ->Start();
  }

  template <typename TInputItem, typename TState>
  void ActionBlock<TInputItem, TState>::Complete()
  {
    return _innerBlock->Complete();
  }

  template <typename TInputItem, typename TState>
  void ActionBlock<TInputItem, TState>::SetFaulted(std::exception_ptr exception)
  {
    _innerBlock->SetFaulted(exception);
  }

  template <typename TInputItem, typename TState>
  bool ActionBlock<TInputItem, TState>::CanAcceptInput(const TInputItem& item)
  {
    return _innerBlock->CanAcceptInput(item);
  }

  template <typename TInputItem, typename TState>
  IDataFlowBlock::TaskVoidType ActionBlock<TInputItem, TState>::AcceptInputAsync(const TInputItem& item)
  {
    return _innerBlock->AcceptInputAsync(item);
  }

  template <typename TInputItem, typename TState>
  IDataFlowBlock::TaskVoidType ActionBlock<TInputItem, TState>::AcceptInputAsync(TInputItem&& item)
  {
    return _innerBlock->AcceptInputAsync(item);
  }
}
