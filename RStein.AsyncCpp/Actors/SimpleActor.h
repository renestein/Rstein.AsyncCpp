#pragma once
#include "IActor.h"
#include "../DataFlow/ActionBlock.h"
#include "../DataFlow/DataflowAsyncFactory.h"
#include "../DataFlow/DataFlowSyncFactory.h"


#include <functional>

namespace RStein::AsyncCpp::Actors
{
  template <typename TMessage>
  class StatelessActor : public IActor<TMessage>
  {
  public:
    using Sync_Func_Type = std::function<void(const TMessage&)>;
    using Async_Func_Type = std::function<Tasks::Task<void>(const TMessage&)>;

    explicit StatelessActor(Sync_Func_Type processMessageFunc);
    explicit StatelessActor(Async_Func_Type processMessageFunc);

    StatelessActor(const StatelessActor& other) = delete;
    StatelessActor(StatelessActor&& other) noexcept = delete;
    StatelessActor& operator=(const StatelessActor& other) = delete;
    StatelessActor& operator=(StatelessActor&& other) noexcept = delete;
    virtual ~StatelessActor();

    void Tell(TMessage message) override;

    Tasks::Task<void> Completion() override;
    void Complete() override;
  private:
    typename DataFlow::ActionBlock<TMessage>::InputBlockPtr _actorQueue;
  };


  template <typename TMessage>
  StatelessActor<TMessage>::StatelessActor(Sync_Func_Type processMessageFunc) : IActor<TMessage>{},
                                                                                _actorQueue{DataFlow::DataFlowSyncFactory::CreateActionBlock<TMessage>(processMessageFunc)}
  {
    _actorQueue->Start();
  }


  template <typename TMessage>
  StatelessActor<TMessage>::StatelessActor(Async_Func_Type processMessageFunc) : IActor<TMessage>{},
                                                                                _actorQueue{DataFlow::DataFlowAsyncFactory::CreateActionBlock(processMessageFunc)}
  {
    _actorQueue->Start();
  }
  
  template <typename TMessage>
  StatelessActor<TMessage>::~StatelessActor()
  {
    try
    {
      _actorQueue->Complete();
      _actorQueue->Completion().Wait();
    }
    catch (...)
    {
    }
  }

  template <typename TMessage>
  void StatelessActor<TMessage>::Tell(TMessage message)
  {
    _actorQueue->AcceptInputAsync(std::move(message)).Wait();
  }

  template <typename TMessage>
  Tasks::Task<void> StatelessActor<TMessage>::Completion()
  {
    return _actorQueue->Completion();
  }

  template <typename TMessage>
  void StatelessActor<TMessage>::Complete()
  {
    _actorQueue->Complete();
  }

  template <typename TMessage, typename TState>
  class StatefulActor : public IActor<TMessage>
  {
  public:
    using Sync_Func_Type = std::function<TState(const TMessage&, const TState&)>;
    using Async_Func_Type = std::function<Tasks::Task<TState>(const TMessage&, const TState&)>;

    StatefulActor(Sync_Func_Type processMessageFunc, TState initialState);
    StatefulActor(Async_Func_Type processMessageFunc, TState initialState);


    StatefulActor(const StatefulActor& other) = delete;
    StatefulActor(StatefulActor&& other) noexcept = delete;
    StatefulActor& operator=(const StatefulActor& other) = delete;
    StatefulActor& operator=(StatefulActor&& other) noexcept = delete;
    virtual ~StatefulActor();


    Tasks::Task<void> Completion() override;
    void Complete() override;
    void Tell(TMessage message) override;
  private:
    typename DataFlow::ActionBlock<TMessage>::InputBlockPtr _actorQueue;
    TState _state;
  };

  template <typename TMessage, typename TState>
  StatefulActor<TMessage, TState>::StatefulActor(Sync_Func_Type processMessageFunc,
                                                 TState initialState) : IActor<TMessage>{},
                                                                        _actorQueue{DataFlow::DataFlowSyncFactory::CreateActionBlock<TMessage>([processMessageFunc, this](const TMessage& message)
                                                                              {
                                                                                _state = processMessageFunc(message, _state);

                                                                              })},
                                                                        _state{initialState}
  {
    _actorQueue->Start();
  }

  template <typename TMessage, typename TState>
  StatefulActor<TMessage, TState>::StatefulActor(Async_Func_Type processMessageFunc,
                                                 TState initialState) : IActor<TMessage>{},
                                                                        _actorQueue{ DataFlow::DataFlowAsyncFactory::CreateActionBlock<TMessage>([processMessageFunc, this](const TMessage& message) -> Tasks::Task<void>
                                                                        {
                                                                          _state = co_await  processMessageFunc(message, _state).ConfigureAwait(false);

                                                                        })},
                                                                        _state{initialState}
  {
    _actorQueue->Start();
  }

  template <typename TMessage, typename TState>
  StatefulActor<TMessage, TState>::~StatefulActor()
  {
    try
    {
      _actorQueue->Complete();
      _actorQueue->Completion().Wait();
    }
    catch (...)
    {
    }
  }

  template <typename TMessage, typename TState>
  Tasks::Task<void> StatefulActor<TMessage, TState>::Completion()
  {
    return _actorQueue->Completion();
  }

  template <typename TMessage, typename TState>
  void StatefulActor<TMessage, TState>::Complete()
  {
    _actorQueue->Complete();
  }

  template <typename TMessage, typename TState>
  void StatefulActor<TMessage, TState>::Tell(TMessage message)
  {
    _actorQueue->AcceptInputAsync(std::move(message)).Wait();
  }

  template<typename TMessage>
  std::unique_ptr<IActor<TMessage>> CreateSimpleActor(typename StatelessActor<TMessage>::Sync_Func_Type processMessageFunc)
  {
    return std::make_unique<StatelessActor<TMessage>>(processMessageFunc);
  }
   
  template<typename TMessage>
  std::unique_ptr<IActor<TMessage>> CreateAsyncSimpleActor(typename StatelessActor<TMessage>::Async_Func_Type processMessageFunc)
  {
    return std::make_unique<StatelessActor<TMessage>>(processMessageFunc);
  }

  template<typename TMessage, typename TState>
  std::unique_ptr<IActor<TMessage>> CreateSimpleActor(typename StatefulActor<TMessage, TState>::Sync_Func_Type processMessageFunc, TState initialState)
  {
    return std::make_unique<StatefulActor<TMessage, TState>>(processMessageFunc, initialState);
  }

  template<typename TMessage, typename TState>
  std::unique_ptr<IActor<TMessage>> CreateAsyncSimpleActor(typename StatefulActor<TMessage, TState>::Async_Func_Type processMessageFunc, TState initialState)
  {
    return std::make_unique<StatefulActor<TMessage, TState>>(processMessageFunc, initialState);
  }
}
