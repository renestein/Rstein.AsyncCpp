#pragma once
#include "../../AsyncPrimitives/CancellationTokenSource.h"
#include "../../DataFlow/IInputOutputBlock.h"
#include "../../AsyncPrimitives/IAsyncProducerConsumerCollection.h"
#include "../../AsyncPrimitives/OperationCanceledException.h"
#include "../../AsyncPrimitives/SimpleAsyncProducerConsumerCollection.h"
#include "../../Collections/ThreadSafeMinimalisticVector.h"
#include "../../AsyncPrimitives/FutureEx.h"
#include "../../DataFlow/IDataFlowBlock.h"
#include "../../Tasks/TaskCombinators.h"
#include "../../Utils/FinallyBlock.h"

#include <thread>
#include <memory>
#include <functional>

namespace RStein::AsyncCpp::Detail
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
  class DataFlowBlockCommon : public DataFlow::IInputOutputBlock<TInputItem, TOutputItem>,
    public std::enable_shared_from_this<DataFlowBlockCommon<TInputItem, TOutputItem, TState>>
  {
  public:

    //TODO: Detect awaitable
    using ActionFuncType = std::function<void(const TInputItem& inputItem, TState*& state)>;
    using AsyncActionFuncType = std::function<typename DataFlow::IInputBlock<TInputItem>::TaskVoidType(const TInputItem& inputItem, TState*& state)>;
    using TransformFuncType = std::function<TOutputItem(const TInputItem& inputItem, TState*& state)>;
    using AsyncTransformFuncType= std::function<typename DataFlow::IInputOutputBlock<TInputItem, TOutputItem>::TaskOutputItemType(const TInputItem& inputItem, TState*& state)>;

    using CanAcceptFuncType = std::function<bool(const TInputItem& item)>;

    using DataFlowBlockCommonPtr = std::shared_ptr<DataFlowBlockCommon<TInputItem, TOutputItem, TState>>;

    explicit DataFlowBlockCommon(AsyncTransformFuncType transformFunc, CanAcceptFuncType canAcceptFunc = [] {return true;});
    explicit DataFlowBlockCommon(TransformFuncType transformFunc, CanAcceptFuncType canAcceptFunc = [] {return true; });
    DataFlowBlockCommon(const DataFlowBlockCommon& other) = delete;
    DataFlowBlockCommon(DataFlowBlockCommon&& other) = delete;
    DataFlowBlockCommon& operator=(const DataFlowBlockCommon& other) = delete;
    DataFlowBlockCommon& operator=(DataFlowBlockCommon&& other) = delete;
    [[nodiscard]] std::string Name() const override;
    void Name(std::string name);
    [[nodiscard]] DataFlow::IDataFlowBlock::TaskVoidType Completion() const override;
    void Start() override;
    void Complete() override;
    void SetFaulted(std::exception_ptr exception) override;
    bool CanAcceptInput(const TInputItem& item) override;
    DataFlow::IDataFlowBlock::TaskVoidType AcceptInputAsync(const TInputItem& item) override;
    DataFlow::IDataFlowBlock::TaskVoidType AcceptInputAsync(TInputItem&& item) override;
    void ConnectTo(const typename DataFlow::IInputBlock<TOutputItem>::InputBlockPtr& nextBlock) override;
    virtual ~DataFlowBlockCommon();
    void removeDeadOutputNodes();
    typename DataFlowBlockCommon<TInputItem, TOutputItem, TState>::TaskVoidType propagateOutput(TOutputItem outputItem);

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
    AsyncTransformFuncType _transformAsyncFunc;
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
    AsyncPrimitives::CancellationTokenSource _processingCts;
    Collections::ThreadSafeMinimalisticVector<std::weak_ptr<DataFlow::IInputBlock<TOutputItem>>> _outputNodes;
    int _startCallsCount;

    explicit DataFlowBlockCommon(CanAcceptFuncType canAcceptFunc);
    typename DataFlowBlockCommon<TInputItem, TOutputItem, TState>::TaskVoidType runProcessingTask(
        AsyncPrimitives::CancellationToken cancellationToken);
    void completeCommon(std::exception_ptr exceptionPtr);
    void throwIfNotStarted();



  };

  template <typename TInputItem, typename TOutputItem, typename TState>
  DataFlowBlockCommon<TInputItem, TOutputItem, TState>::DataFlowBlockCommon(AsyncTransformFuncType transformFunc,
                                                                            CanAcceptFuncType canAcceptFunc) : DataFlowBlockCommon(std::move(canAcceptFunc))
  
  {
    if (!transformFunc)
    {
      throw std::invalid_argument("transformFunc");
    }

    _isAsyncNode = true;
    _transformAsyncFunc = transformFunc;

  }
  template <typename TInputItem, typename TOutputItem, typename TState>
  DataFlowBlockCommon<TInputItem, TOutputItem, TState>::DataFlowBlockCommon(TransformFuncType transformFunc,
                                                                            CanAcceptFuncType canAcceptFunc) : DataFlowBlockCommon(std:: move(canAcceptFunc))
  
  {
    if (!transformFunc)
    {
      throw std::invalid_argument("transformFunc");
    }

    _isAsyncNode = false;
    _transformSyncFunc = transformFunc;

  }

  
  template <typename TInputItem, typename TOutputItem, typename TState>
  DataFlowBlockCommon<TInputItem, TOutputItem, TState>::DataFlowBlockCommon(CanAcceptFuncType canAcceptFunc) :
                                                                            DataFlow::IInputOutputBlock<TInputItem, TOutputItem>{},
                                                                            std::enable_shared_from_this<DataFlowBlockCommon<TInputItem, TOutputItem, TState>>{},
                                                                            _isAsyncNode(),
                                                                            _transformSyncFunc{},
                                                                            _transformAsyncFunc{},
                                                                            _canAcceptFunc{std::move(canAcceptFunc)},
                                                                            _name{},
                                                                            _completedTaskPromise{},
                                                                            _completedTask{ _completedTaskPromise.GetTask()},
                                                                            _startTaskPromise{},
                                                                            _startTask{ _startTaskPromise.GetTask()},
                                                                            _processingTask{Tasks::GetCompletedTask()},
                                                                            _state{ BlockState::Created },
                                                                            _stateMutex{},
                                                                            _inputItems{},
                                                                            _processingCts{AsyncPrimitives::CancellationTokenSource{}},
                                                                            _outputNodes{ std::vector<std::weak_ptr<DataFlow::IInputBlock<TOutputItem>>>{}},
                                                                            _startCallsCount{}
  {
    if (!_canAcceptFunc)
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
  DataFlow::IDataFlowBlock::TaskVoidType DataFlowBlockCommon<
    TInputItem, TOutputItem, TState>::Completion() const
  {
    return _completedTask;
  }


  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::Start()
  {
    std::lock_guard lock{ _stateMutex };
    _startCallsCount++;
    if (_state == BlockState::Started)
    {
      return;
    }

    if (_state != BlockState::Created)
    {
      throw std::logic_error("Could not start node!");
    }

    _processingTask = runProcessingTask(_processingCts.Token());

    for (auto& nextBlock : _outputNodes.template MapSnapshot<typename DataFlow::IInputBlock<TOutputItem>::InputBlockPtr>([](auto &weakPtr){return weakPtr.lock();}))
    {
      
      if (!nextBlock)
      {    
        continue;;
      }

      nextBlock->Start();
    }

    removeDeadOutputNodes();
    _state = BlockState::Started;
    
    _startTaskPromise.SetResult();
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
  bool DataFlowBlockCommon<TInputItem, TOutputItem, TState>::CanAcceptInput(const TInputItem& item)
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
  DataFlow::IDataFlowBlock::TaskVoidType DataFlowBlockCommon<
    TInputItem, TOutputItem, TState>::AcceptInputAsync(const TInputItem& item)
  {
    //TODO: Avoid lock
    throwIfNotStarted();
    return _inputItems.AddAsync(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  DataFlow::IDataFlowBlock::TaskVoidType DataFlowBlockCommon<TInputItem,
                                                                               TOutputItem, TState>::AcceptInputAsync(
      TInputItem&& item)
  {
    //TODO: Avoid lock
    throwIfNotStarted();
    return _inputItems.AddAsync(item);
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::ConnectTo(const typename DataFlow::IInputBlock<TOutputItem>::InputBlockPtr& nextBlock)
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
  void DataFlowBlockCommon<TInputItem, TOutputItem, TState>::removeDeadOutputNodes()
  {
    _outputNodes.RemoveIf([](std::weak_ptr<DataFlow::IInputBlock<TOutputItem>> weakPtr){return !weakPtr.lock();});
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  typename DataFlowBlockCommon<TInputItem, TOutputItem, TState>::TaskVoidType DataFlowBlockCommon<
    TInputItem, TOutputItem, TState>::propagateOutput(TOutputItem outputItem)
  {
    auto outputNodesSnapshot = _outputNodes.template MapSnapshot<typename DataFlow::IInputBlock<TOutputItem>::InputBlockPtr>([](auto& weakPtr){return weakPtr.lock();});
    
    //Todo: Do not copy nodes
    //TODO: Optimize cycle
    //
    auto shouldRemoveDeadBlocks = false;
    for (auto nodeIterator = outputNodesSnapshot.begin(); nodeIterator != outputNodesSnapshot.end(); ++nodeIterator)
    {
      auto inputNodePtr = *nodeIterator;
      if (!inputNodePtr)
      {
        shouldRemoveDeadBlocks = true;
        continue;
      }

      if (!inputNodePtr->CanAcceptInput(outputItem))
      {

        *nodeIterator = typename DataFlow::IInputBlock<TOutputItem>::InputBlockPtr();
      }
    }

    if(shouldRemoveDeadBlocks)
    {
      removeDeadOutputNodes();
    }

    for (auto& node : outputNodesSnapshot)
    {
      if (node)
      {
        co_await node->AcceptInputAsync(outputItem).ConfigureAwait(false);
      }
    }
  }

  template <typename TInputItem, typename TOutputItem, typename TState>
  typename DataFlowBlockCommon<TInputItem, TOutputItem, TState>::TaskVoidType DataFlowBlockCommon<
    TInputItem, TOutputItem, TState>::runProcessingTask(
      AsyncPrimitives::CancellationToken cancellationToken)
  {
    //TODO: Use Scheduler   
    try
    {
      TState state{};
      auto statePtr = &state;     
      co_await _startTask.ConfigureAwait(false);
      TInputItem inputItem;
      while (!cancellationToken.IsCancellationRequested())
      {
        try
        {
          inputItem = co_await _inputItems.TakeAsync(cancellationToken).ConfigureAwait(false);
        }
        catch (AsyncPrimitives::OperationCanceledException&)
        {
          continue;
        }

        auto outputItem = _isAsyncNode
          ? co_await _transformAsyncFunc(inputItem, statePtr).ConfigureAwait(false)
          : _transformSyncFunc(inputItem, statePtr);

        propagateOutput(outputItem);
      }

      //refactor cycle
      auto toProcessInputItems = _inputItems.TryTakeAll();
      for (auto& item : toProcessInputItems)
      {
        auto outputRemainingItem = _isAsyncNode
          ? co_await _transformAsyncFunc(item, statePtr).ConfigureAwait(false)
          : _transformSyncFunc(item, statePtr);

        propagateOutput(outputRemainingItem);
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
    if (--_startCallsCount > 0 && exceptionPtr == nullptr)
    {
      return;
    }

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

          for (auto& nextBlock : _outputNodes.template MapSnapshot<typename DataFlow::IInputBlock<TOutputItem>::InputBlockPtr>([](auto &weakPtr){return weakPtr.lock();}))
          {
            //TODO: Handle failing output node;
            if (!nextBlock)
            {
              continue;
            }

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


      _processingCts.Cancel();
      _processingTask.Wait();
      if (exceptionPtr != nullptr)
      {
        _completedTaskPromise.TrySetException(exceptionPtr);
      }
      else
      {
        _completedTaskPromise.TrySetResult();
      }   
  }
}


