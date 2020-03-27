﻿#pragma once
#include "../IInputOutputBlock.h"
#include "../../AsyncPrimitives/CancellationTokenSource.h"
#include "../../AsyncPrimitives/IAsyncProducerConsumerCollection.h"
#include "../../AsyncPrimitives/OperationCanceledException.h"
#include "../../AsyncPrimitives/SimpleAsyncProducerConsumerCollection.h"
#include "../../Collections/ThreadSafeMinimalisticVector.h"
#include "../../Utils/FinallyBlock.h"
#include "../../AsyncPrimitives/FutureEx.h"
#include <thread>
#include <memory>
#include <functional>

namespace RStein::AsyncCpp::DataFlow::Detail
{
  struct NoState
  {

  };
  struct NoOutput
  {
    static NoOutput& Default()
    {
      static NoOutput noOutput;
      return noOutput;
    }
  };
  template<typename TInputItem, typename TOutputItem, typename TState = NoState>
  class DataFlowBlockCommon : public IInputOutputBlock<TInputItem, TOutputItem>,
    public std::enable_shared_from_this<DataFlowBlockCommon<TInputItem, TOutputItem, TState>>
  {
  public:

    //TODO: Detect awaitable
    using ActionFuncType = std::function<void(const TInputItem& inputItem, TState*& state)>;
    using TransformFuncType = std::function<TOutputItem(const TInputItem& inputItem, TState*& state)>;
    using CanAcceptFuncType = std::function<bool(const TInputItem& item)>;

    using DataFlowBlockCommonPtr = std::shared_ptr<DataFlowBlockCommon<TInputItem, TOutputItem, TState>>;

    explicit DataFlowBlockCommon(TransformFuncType transformFunc, CanAcceptFuncType canAcceptFunc = [] {return true; });
    DataFlowBlockCommon(const DataFlowBlockCommon& other) = delete;
    DataFlowBlockCommon(DataFlowBlockCommon&& other) = delete;
    DataFlowBlockCommon& operator=(const DataFlowBlockCommon& other) = delete;
    DataFlowBlockCommon& operator=(DataFlowBlockCommon&& other) = delete;
    [[nodiscard]] std::string Name() const override;
    void Name(std::string name);
    [[nodiscard]] typename IDataFlowBlock::TaskVoidType Completion() const override;
    void Start() override;
    void Complete() override;
    void SetFaulted(std::exception_ptr exception) override;
    bool CanAcceptInput(const TInputItem& item) override;
    typename IDataFlowBlock::TaskVoidType AcceptInputAsync(const TInputItem& item) override;
    typename IDataFlowBlock::TaskVoidType AcceptInputAsync(TInputItem&& item) override;
    void ConnectTo(const typename IInputBlock<TOutputItem>::InputBlockPtr& nextBlock) override;
    virtual ~DataFlowBlockCommon();

  private:
    enum class BlockState
    {
      Created,
      Starting,
      Started,
      Stopping,
      Stopped
    };
    bool _isAsyncNode;
    TransformFuncType _transformSyncFunc;
    std::function<typename DataFlowBlockCommon::TaskOutputItemType(TInputItem, TState*&)> _transformAsyncFunc;
    std::function<bool(const TInputItem&)> _canAcceptFunc;;
    std::string _name;
    typename DataFlowBlockCommon::PromiseVoidType _completedTaskPromise;
    typename DataFlowBlockCommon::TaskVoidType _completedTask;
    typename DataFlowBlockCommon::PromiseVoidType _startTaskPromise;
    typename DataFlowBlockCommon::TaskVoidType _startTask;
    typename DataFlowBlockCommon::TaskVoidType _processingTask;
    BlockState _state;
    std::mutex _stateMutex;
    AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TInputItem> _inputItems;
    AsyncPrimitives::CancellationTokenSource::CancellationTokenSourcePtr _processingCts;
    Collections::ThreadSafeMinimalisticVector<typename IInputBlock<TOutputItem>::InputBlockPtr> _outputNodes;

    std::shared_future<void> runProcessingTask(
      AsyncPrimitives::CancellationToken::CancellationTokenPtr cancellationToken);
    void completeCommon(std::exception_ptr exceptionPtr);
    void throwIfNotStarted();



  };

  template <typename TInputItem, typename TOutputItem, typename TState>
  DataFlowBlockCommon<TInputItem, TOutputItem, TState>::DataFlowBlockCommon(TransformFuncType transformFunc,
    std::function<bool(const TInputItem&)> canAcceptFunc) :
    IInputOutputBlock<TInputItem, TOutputItem>{},
    std::enable_shared_from_this<DataFlowBlockCommon<TInputItem, TOutputItem, TState>>{},
    _isAsyncNode{ false },
    _transformSyncFunc{},
    _transformAsyncFunc{},
    _canAcceptFunc{canAcceptFunc},
    _name{},
    _completedTaskPromise{},
    _completedTask{ _completedTaskPromise.get_future().share() },
    _startTaskPromise{},
    _startTask{ _startTaskPromise.get_future().share() },
    _state{ BlockState::Created },
    _stateMutex{},
    _inputItems{},
    _processingCts{ AsyncPrimitives::CancellationTokenSource::Create() },
    _outputNodes{ std::vector<typename IInputBlock<TOutputItem>::InputBlockPtr>{} }
  {



    _transformSyncFunc = transformFunc;

    if (!_transformSyncFunc && !_transformAsyncFunc)
    {
      throw std::invalid_argument("transformFunc");
    }
    if (!canAcceptFunc)
    {
      _canAcceptFunc = [](auto _) {return true; };
    }
  }



  template <typename TInputItem, typename TOutputItem, typename TState>
  std::string DataFlowBlockCommon<TInputItem, TOutputItem, TState>::Name() const
  {
    return _name;
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::Name(std::string name)
  {
    _name = name;
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  IDataFlowBlock::TaskVoidType DataFlowBlockCommon<TInputItem, TOutputItem, TState>::Completion() const
  {
    return _completedTask;
  }


  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::Start()
  {
    std::lock_guard lock{ _stateMutex };
    if (_state != BlockState::Created)
    {
      throw std::logic_error("Could not start node!");
    }

    _processingTask = runProcessingTask(_processingCts->Token());

    for (auto& nextBlock : _outputNodes.GetSnapshot())
    {
      nextBlock->Start();
    }

    _state = BlockState::Started;

    _startTaskPromise.set_value();
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::Complete()
  {
    std::lock_guard lock{ _stateMutex };
    completeCommon(nullptr);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::SetFaulted(std::exception_ptr exception)
  {
    std::lock_guard lock{ _stateMutex };
    completeCommon(exception);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  bool DataFlowBlockCommon<TInputItem, TOutputItem, TState>::CanAcceptInput(
    const TInputItem& item)
  {
    //Avoid lock
    {
      std::lock_guard lock{ _stateMutex };
      if (_state != BlockState::Started)
      {
        return false;
      }
    }

    return _canAcceptFunc(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::throwIfNotStarted()
  {
    std::lock_guard lock{ _stateMutex };
    if (_state != BlockState::Started)
    {
      throw std::logic_error("Node does not running");
    }
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  typename IDataFlowBlock::TaskVoidType DataFlowBlockCommon<TInputItem, TOutputItem, TState>::AcceptInputAsync(const TInputItem& item)
  {
    //TODO: Avoid lock
    throwIfNotStarted();
    return _inputItems.AddAsync(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  typename IDataFlowBlock::TaskVoidType DataFlowBlockCommon<TInputItem,
    TOutputItem, TState>::AcceptInputAsync(TInputItem&& item)
  {
    //TODO: Avoid lock
    throwIfNotStarted();
    return _inputItems.AddAsync(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::ConnectTo(
      const typename IInputBlock<TOutputItem>::InputBlockPtr& nextBlock)
  {
    if (!nextBlock)
    {
      throw std::invalid_argument("nextBlock");
    }

    _outputNodes.Add(nextBlock);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  DataFlowBlockCommon<TInputItem, TOutputItem, TState>::~DataFlowBlockCommon()
  {

  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  std::shared_future<void> DataFlowBlockCommon<
    TInputItem, TOutputItem, TState>::runProcessingTask(
      AsyncPrimitives::CancellationToken::CancellationTokenPtr cancellationToken)
  {
    //TODO: Use Scheduler   
    try
    {
      TState state{};
      auto statePtr = &state;
      //For co_await operator
      using namespace RStein::AsyncCpp::AsyncPrimitives;
      co_await _startTask;
      TInputItem inputItem;
      while (!cancellationToken->IsCancellationRequested())
      {
        try
        {
          inputItem = co_await _inputItems.TakeAsync(cancellationToken);
        }
        catch (AsyncPrimitives::OperationCanceledException&)
        {
          continue;
        }

        auto outputItem = _isAsyncNode
          ? co_await _transformAsyncFunc(inputItem, statePtr)
          : _transformSyncFunc(inputItem, statePtr);

        //Todo: Do not copy nodes
        for (auto& outputNode : _outputNodes.GetSnapshot())
        {
          if (outputNode->CanAcceptInput(outputItem))
          {
            co_await outputNode->AcceptInputAsync(outputItem);
          }
        }
      }

      //refactor cycle
      auto toProcessInputItems = _inputItems.TryTakeAll();
      for (auto& item : toProcessInputItems)
      {
        auto outputRemainingItem = _isAsyncNode
          ? co_await _transformAsyncFunc(item, statePtr)
          : _transformSyncFunc(item, statePtr);

        for (auto& outputNode : _outputNodes.GetSnapshot())
        {
          if (outputNode->CanAcceptInput(outputRemainingItem))
          {
            co_await outputNode->AcceptInputAsync(outputRemainingItem);
          }
        }
      }
    }
    catch (const std::exception& ex)
    {
      const auto message = " DataFlow node: " + Name() + " - processing task failed with exception: \n " + ex.what();
      std::cout << message;
      const auto exceptionPtr = std::current_exception();
      //TODO: Schedule call
      auto callCompleteThread = std::thread{
          [sharedThis = this->shared_from_this(), exceptionPtr]
          {
            sharedThis->SetFaulted(exceptionPtr);
          }
      };
      callCompleteThread.detach();
      throw;
    }
  }


  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::completeCommon(std::exception_ptr exceptionPtr)
  {

    if (_state == BlockState::Created)
    {
      throw std::logic_error("Could not stop node.");
    }

    if (_state == BlockState::Stopped)
    {
      return;
    }

    _state = BlockState::Stopping;
    Utils::FinallyBlock finally
    {
        [this, isExceptional = exceptionPtr != nullptr, exceptionPtr = exceptionPtr]
        {
          _state = BlockState::Stopped;

          for (auto& nextBlock : _outputNodes.GetSnapshot())
          {
            //TODO: Handle failing output node;

            if (isExceptional)
            {
              nextBlock->SetFaulted(exceptionPtr);
            }
            else
            {
              nextBlock->Complete();
            }
          }
        }
    };


    try
    {
      _processingCts->Cancel();
      _processingTask.get();
      if (exceptionPtr != nullptr)
      {
        _completedTaskPromise.set_exception(exceptionPtr);
      }
      else
      {
        _completedTaskPromise.set_value();
      }
    }
    catch (const std::future_error&)
    {
      const auto alreadySetMessage = "DataFlow node " + Name() + " Completion task already fulfilled. Future_error ignored.";
      std::cout << alreadySetMessage;
    }
  }
}


