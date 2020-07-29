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
    explicit StatelessActor(std::function<void(const TMessage&)> processMessageFunc);
    explicit StatelessActor(std::function<Tasks::Task<void>(const TMessage&)> processMessageFunc);


    StatelessActor(const StatelessActor& other) = delete;
    StatelessActor(StatelessActor&& other) noexcept = delete;
    StatelessActor& operator=(const StatelessActor& other) = delete;
    StatelessActor& operator=(StatelessActor&& other) noexcept = delete;
    virtual ~StatelessActor();

    void Tell(TMessage message) override;
  private:
    typename DataFlow::ActionBlock<TMessage>::InputBlockPtr _actorQueue;
  };

  template <typename TMessage>
  StatelessActor<TMessage>::StatelessActor(std::function<void(const TMessage&)> processMessageFunc) : IActor<TMessage>{},
                                                                                               _actorQueue{DataFlow::DataFlowSyncFactory::CreateActionBlock<TMessage>(processMessageFunc)}
  {
    _actorQueue->Start();
  }

  template <typename TMessage>
  StatelessActor<TMessage>::StatelessActor(std::function<Tasks::Task<void>(const TMessage&)> processMessageFunc) : IActor<TMessage>{},
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

  template <typename TMessage, typename TState>
  class StatefulActor : public IActor<TMessage>
  {
  public:
    StatefulActor(std::function<TState(const TState&, const TMessage&)> processMessageFunc, TState initialState);
    StatefulActor(std::function<Tasks::Task<TState>(const TState&, const TMessage&)> processMessageFunc, TState initialState);


    StatefulActor(const StatefulActor& other) = delete;
    StatefulActor(StatefulActor&& other) noexcept = delete;
    StatefulActor& operator=(const StatefulActor& other) = delete;
    StatefulActor& operator=(StatefulActor&& other) noexcept = delete;
    virtual ~StatefulActor();


    void Tell(TMessage message) override;
  private:
    typename DataFlow::ActionBlock<TMessage>::InputBlockPtr _actorQueue;
    TState _state;
  };

  template <typename TMessage, typename TState>
  StatefulActor<TMessage, TState>::StatefulActor(std::function<TState(const TState&, const TMessage&)> processMessageFunc,
                                                   TState initialState) : IActor<TMessage>{},
                                                                          _actorQueue{DataFlow::DataFlowSyncFactory::CreateActionBlock<TMessage>([processMessageFunc, this](const TMessage& message)
                                                                                                                               {
                                                                                                                                 _state = processMessageFunc(_state, message);
                                                                                                                               })
                                                                          },
                                                                          _state{initialState}
  {
    _actorQueue->Start();
  }

  template <typename TMessage, typename TState>
  StatefulActor<TMessage, TState>::StatefulActor(std::function<Tasks::Task<TState>(const TState&, const TMessage&)> processMessageFunc,
                                                   TState initialState) : IActor<TMessage>{},
                                                   _actorQueue{ DataFlow::DataFlowAsyncFactory::CreateActionBlock<TMessage>([processMessageFunc, this](const TMessage& message) -> Tasks::Task<void>
                                                                                                         {
                                                                                                           _state = co_await  processMessageFunc(_state,message);
                                                                                                         })
                                                   },
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
  void StatefulActor<TMessage, TState>::Tell(TMessage message)
  {
    _actorQueue->AcceptInputAsync(std::move(message)).Wait();
  }

  template<typename TMessage>
  std::unique_ptr<IActor<TMessage>> CreateSimpleActor(std::function<void(const TMessage&)> processMessageFunc)
  {
    return std::make_unique<StatelessActor<TMessage>>(processMessageFunc);
  }
   
  template<typename TMessage>
  std::unique_ptr<IActor<TMessage>> CreateAsyncSimpleActor(std::function<Tasks::Task<void>(const TMessage&)> processMessageFunc)
  {
    return std::make_unique<StatelessActor<TMessage>>(processMessageFunc);
  }

  template<typename TMessage, typename TState>
  std::unique_ptr<IActor<TMessage>> CreateSimpleActor(std::function<TState(const TState&, const TMessage&)> processMessageFunc, TState initialState)
  {
    return std::make_unique<StatefulActor<TMessage, TState>>(processMessageFunc, initialState);
  }

  template<typename TMessage, typename TState>
  std::unique_ptr<IActor<TMessage>> CreateAsyncSimpleActor(std::function<Tasks::Task<TState>(const TState&, const TMessage&)> processMessageFunc, TState initialState)
  {
    return std::make_unique<StatefulActor<TMessage, TState>>(processMessageFunc, initialState);
  }
}
