#pragma once
#include "IReplyActor.h"
#include "../DataFlow/ActionBlock.h"
#include "../DataFlow/DataflowAsyncFactory.h"
#include "../DataFlow/DataFlowSyncFactory.h"
#include "../Tasks/TaskCompletionSource.h"


namespace RStein::AsyncCpp::Actors
{
  template<typename TMessage, typename TResult>
  class StatelessReplyActor : public IReplyActor<TMessage, TResult>
  {
  public:

    using Async_Func_Type = std::function<Tasks::Task<TResult>(TMessage)>;
    using Sync_Func_Type = std::function<TResult(TMessage)>;

    explicit StatelessReplyActor(Async_Func_Type processMessageFunc);
    explicit StatelessReplyActor(Sync_Func_Type processMessageFunc);

    StatelessReplyActor(const StatelessReplyActor& other) = delete;
    StatelessReplyActor(StatelessReplyActor&& other) noexcept = delete;
    StatelessReplyActor& operator=(const StatelessReplyActor& other) = delete;
    StatelessReplyActor& operator=(StatelessReplyActor&& other) noexcept = delete;
    virtual ~StatelessReplyActor();
    Tasks::Task<TResult> Ask(TMessage message) override;
    Tasks::Task<void> Completion() override;
    void Complete() override;

  private:
    typename DataFlow::ActionBlock<std::pair<TMessage, Tasks::TaskCompletionSource<TResult>>>::InputBlockPtr _actorQueue;
  };

  template <typename TMessage, typename TResult>
  StatelessReplyActor<TMessage, TResult>::StatelessReplyActor(Async_Func_Type processMessageFunc) : IReplyActor<TMessage, TResult>{},
                                                                                                    _actorQueue{ DataFlow::DataFlowAsyncFactory::CreateActionBlock<std::pair<TMessage, Tasks::TaskCompletionSource<TResult>>>([processMessageFunc](const std::pair<TMessage, Tasks::TaskCompletionSource<TResult>>& message)-> Tasks::Task<void>
                                                                                                     {
                                                                                                       auto& [innerMessage, tcs] = message;
                                                                                                       //non-const object
                                                                                                       //TODO: Handle error in copy ctor
                                                                                                       auto tcsCopy = tcs;
                                                                                                       try
                                                                                                       {
                                                                                                         auto result = co_await processMessageFunc(innerMessage).ConfigureAwait(false);
                                                                                                         tcsCopy.SetResult(std::move(result));
                                                                                                       }
                                                                                                       catch (...)
                                                                                                       {
                                                                                                         tcsCopy.SetException(std::current_exception());
                                                                                                       }
                                                                                                     })}
  {
    _actorQueue->Start();
  }

  template <typename TMessage, typename TResult>
  StatelessReplyActor<TMessage, TResult>::StatelessReplyActor(Sync_Func_Type processMessageFunc) :  IReplyActor<TMessage, TResult>{},
                                                                                                    _actorQueue{ DataFlow::DataFlowSyncFactory::CreateActionBlock<std::pair<TMessage, Tasks::TaskCompletionSource<TResult>>>([processMessageFunc](const std::pair<TMessage, Tasks::TaskCompletionSource<TResult>>& message)
                                                                                                    {
                                                                                                        auto& [innerMessage, tcs] = message;
                                                                                                        //non-const object
                                                                                                        //TODO: Handle error in copy ctor
                                                                                                        auto tcsCopy = tcs;
                                                                                                        try
                                                                                                        {
                                                                                                          auto result = processMessageFunc(innerMessage);
                                                                                                          tcsCopy.SetResult(result);
                                                                                                        }
                                                                                                        catch (...)
                                                                                                        {
                                                                                                          tcsCopy.SetException(std::current_exception());
                                                                                                        }

                                                                                                      })}
  {
    _actorQueue->Start();
  }

  template <typename TMessage, typename TResult>
  StatelessReplyActor<TMessage, TResult>::~StatelessReplyActor()
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

  template <typename TMessage, typename TResult>
  Tasks::Task<TResult> StatelessReplyActor<TMessage, TResult>::Ask(TMessage message)
  {
    Tasks::TaskCompletionSource<TResult> tcs{};
    _actorQueue->AcceptInputAsync(std::make_pair(message, tcs)).Wait();
    return tcs.GetTask();

  }

  template <typename TMessage, typename TResult>
  Tasks::Task<void> StatelessReplyActor<TMessage, TResult>::Completion()
  {
    return _actorQueue->Completion();
  }

  template <typename TMessage, typename TResult>
  void StatelessReplyActor<TMessage, TResult>::Complete()
  {
    return _actorQueue->Complete();
  }

  template<typename TMessage, typename TResult, typename TState>
  class StatefulReplyActor : public IReplyActor<TMessage, TResult>
  {
   public:

    using Sync_Func_Type = std::function<std::pair<TResult, TState>(TMessage, TState)>;
    using Async_Func_Type = std::function<Tasks::Task<std::pair<TResult, TState>>(TMessage, TState)>;

    StatefulReplyActor(Sync_Func_Type processMessageFunc, TState initialState);
    StatefulReplyActor(Async_Func_Type processMessageFunc, TState initialState);
    StatefulReplyActor(const StatefulReplyActor& other) = delete;
    StatefulReplyActor(StatefulReplyActor&& other) noexcept = delete;
    StatefulReplyActor& operator=(const StatefulReplyActor& other) = delete;
    StatefulReplyActor& operator=(StatefulReplyActor&& other) noexcept = delete;
    virtual ~StatefulReplyActor();

    Tasks::Task<TResult> Ask(TMessage message) override;

    Tasks::Task<void> Completion() override;
    void Complete() override;
   private:
     typename DataFlow::ActionBlock<std::tuple<TMessage, Tasks::TaskCompletionSource<TResult>>>::InputBlockPtr _actorQueue;
     TState _state;

  };

  template <typename TMessage, typename TResult, typename TState>
  StatefulReplyActor<TMessage, TResult, TState>::StatefulReplyActor(Sync_Func_Type processMessageFunc, TState initialState) : _actorQueue{DataFlow::DataFlowSyncFactory::CreateActionBlock<std::tuple<TMessage, Tasks::TaskCompletionSource<TResult>>>([processMessageFunc, this](const std::tuple<TMessage, Tasks::TaskCompletionSource<TResult>>& message)
                                                                                                                              {
                                                                                                                                  auto& [innerMessage, tcs] = message;
                                                                                                                                  //non-const object
                                                                                                                                  //TODO: Handle error in copy ctor
                                                                                                                                  auto tcsCopy = tcs;
                                                                                                                                  try
                                                                                                                                  {
                                                                                                                                     auto [result, newState] = processMessageFunc( innerMessage, _state);
                                                                                                                                   
                                                                                                                                    _state = std::move(newState);
                                                                                                                                    tcsCopy.SetResult(std::move(result));
                                                                                                                                  }
                                                                                                                                  catch (...)
                                                                                                                                  {
                                                                                                                                    tcsCopy.SetException(std::current_exception());
                                                                                                                                  }
                                                                                                                              })},
                                                                                                                              _state{initialState}
  {
    _actorQueue->Start();
  }

  template <typename TMessage, typename TResult, typename TState>
  StatefulReplyActor<TMessage, TResult, TState>::StatefulReplyActor( Async_Func_Type processMessageFunc, TState initialState) : _actorQueue{DataFlow::DataFlowAsyncFactory::CreateActionBlock<std::tuple<TMessage, Tasks::TaskCompletionSource<TResult>>>([processMessageFunc, this](const std::tuple<TMessage, Tasks::TaskCompletionSource<TResult>>& message)-> Tasks::Task<void>
                                                                                                                                {
                                                                                                                                const auto& [innerMessage, tcs] = message;
                                                                                                                                //non-const object
                                                                                                                                //TODO: Handle error in copy ctor
                                                                                                                                auto tcsCopy = tcs;
                                                                                                                                try
                                                                                                                                {
                                                                                                                                  auto [result, newState] = co_await processMessageFunc(innerMessage, _state).ConfigureAwait(false);
                                                                                                                                 
                                                                                                                                  _state = std::move(newState);
                                                                                                                                  tcsCopy.SetResult(std::move(result));
                                                                                                                                }
                                                                                                                                catch (...)
                                                                                                                                {
                                                                                                                                  tcsCopy.SetException(std::current_exception());
                                                                                                                                }
                                                                                                                                
                                                                                                                              })},
                                                                                                                              _state{initialState}
  {  
      _actorQueue->Start();
  }

  template <typename TMessage, typename TResult, typename TState>
  StatefulReplyActor<TMessage, TResult, TState>::~StatefulReplyActor()
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

  template <typename TMessage, typename TResult, typename TState>
  Tasks::Task<TResult> StatefulReplyActor<TMessage, TResult, TState>::Ask(TMessage message)
  {
     Tasks::TaskCompletionSource<TResult> tcs{};
     auto messageTcsPair = std::make_tuple(message, tcs);
    _actorQueue->AcceptInputAsync(messageTcsPair).Wait();
    return tcs.GetTask();
  }

  template <typename TMessage, typename TResult, typename TState>
  Tasks::Task<void> StatefulReplyActor<TMessage, TResult, TState>::Completion()
  {
    return _actorQueue->Completion();
  }

  template <typename TMessage, typename TResult, typename TState>
  void StatefulReplyActor<TMessage, TResult, TState>::Complete()
  {
    return _actorQueue->Complete();
  }


  template<typename TMessage, typename TResult>
  std::unique_ptr<IReplyActor<TMessage, TResult>> CreateReplyActor(typename StatelessReplyActor<TMessage, TResult>::Sync_Func_Type processMessageFunc)
  {
    return std::make_unique<StatelessReplyActor<TMessage, TResult>>(processMessageFunc);
  }
   
  template<typename TMessage, typename TResult>
   std::unique_ptr<IReplyActor<TMessage, TResult>> CreateAsyncReplyActor(typename StatelessReplyActor<TMessage, TResult>::Async_Func_Type processMessageFunc)
  {
    return std::make_unique<StatelessReplyActor<TMessage, TResult>>(processMessageFunc);
  }

  template<typename TMessage, typename TResult, typename TState>
  std::unique_ptr<IReplyActor<TMessage, TResult>>CreateReplyActor(typename StatefulReplyActor<TMessage, TResult, TState>::Sync_Func_Type processMessageFunc, TState initialState)
  {
    return std::make_unique<StatefulReplyActor<TMessage, TResult, TState>>(processMessageFunc, initialState);
  }

  template<typename TMessage, typename TResult, typename TState>
  std::unique_ptr<IReplyActor<TMessage, TResult>> CreateAsyncReplyActor(typename StatefulReplyActor<TMessage, TResult, TState>::Async_Func_Type processMessageFunc, TState initialState)
  {
    return std::make_unique<StatefulReplyActor<TMessage, TResult, TState>>(processMessageFunc, initialState);
  }
}

