#pragma once
#include "../Collections/ThreadSafeMinimalisticQueue.h"
#include "AsyncSemaphore.h"
#include "FutureEx.h"
#include "IAsyncProducerConsumerCollection.h"

#include <stdexcept>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  template <typename TItem>
  class SimpleAsyncProducerConsumerCollection : public IAsyncProducerConsumerCollection<TItem
      >
  {
  public:
    SimpleAsyncProducerConsumerCollection();
    SimpleAsyncProducerConsumerCollection(const SimpleAsyncProducerConsumerCollection& other) = delete;
    SimpleAsyncProducerConsumerCollection(SimpleAsyncProducerConsumerCollection&& other) noexcept = delete;
    SimpleAsyncProducerConsumerCollection& operator=(const SimpleAsyncProducerConsumerCollection& other) = delete;
    SimpleAsyncProducerConsumerCollection& operator=(SimpleAsyncProducerConsumerCollection&& other) noexcept = delete;
    virtual ~SimpleAsyncProducerConsumerCollection() = default;

    void Add(TItem&& item) override;
    void Add(const TItem& item) override;
    std::future<void> AddAsync(const TItem& item) override;
    std::future<void> AddAsync(TItem&& item) override;
    std::future<TItem> TakeAsync() override;
    std::future<TItem> TakeAsync(CancellationToken::CancellationTokenPtr cancellationToken) override;

  private:

    Collections::ThreadSafeMinimalisticQueue<TItem> _innerCollection;
    AsyncSemaphore _asyncSemaphore;
  };
}


template <typename TItem>
RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::
SimpleAsyncProducerConsumerCollection() :
  IAsyncProducerConsumerCollection<TItem>(),
  _innerCollection(),
  _asyncSemaphore(std::numeric_limits<int>::max(), 0)
{
}

template <typename TItem>
void RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::Add(const TItem& item)
{
  _innerCollection.Push(item);
  _asyncSemaphore.Release();
}

template <typename TItem>
void RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::Add(TItem&& item)
{
  _innerCollection.Push(std::move(item));
  _asyncSemaphore.Release();
}

template <typename TItem>
std::future<void> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::AddAsync(const TItem& item)
{
   Add(item);
  return GetCompletedFuture();
}

template <typename TItem>
std::future<void> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::AddAsync(TItem&& item)
{
  Add(std::move(item));
  return GetCompletedFuture();
}

template <typename TItem>
std::future<TItem> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::TakeAsync()
{
  co_await _asyncSemaphore.WaitAsync();
  auto retValue = _innerCollection.TryPop();
  if (!retValue)
  {
    throw std::logic_error("Could not take item");
  }

  co_return retValue.value();
}

template <typename TItem>
std::future<TItem> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::TakeAsync(
    CancellationToken::CancellationTokenPtr cancellationToken)
{
  co_await _asyncSemaphore.WaitAsync(cancellationToken);
  auto retValue = _innerCollection.TryPop();
  if (!retValue)
  {
    throw std::logic_error("Could not take item");
  }

  co_return retValue.value();
}
