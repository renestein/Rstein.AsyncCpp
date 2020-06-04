#pragma once
#include "AsyncSemaphore.h"
#include "IAsyncProducerConsumerCollection.h"
#include "../Collections/ThreadSafeMinimalisticQueue.h"
#include "../Tasks/TaskCombinators.h"


#include <stdexcept>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  template <typename TItem>
  class SimpleAsyncProducerConsumerCollection : public IAsyncProducerConsumerCollection<TItem>
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
    Tasks::Task<void> AddAsync(const TItem& item) override;
    Tasks::Task<void> AddAsync(TItem&& item) override;
    Tasks::Task<TItem> TakeAsync() override;
    Tasks::Task<TItem> TakeAsync(CancellationToken cancellationToken) override;
    std::vector<TItem> TryTakeAll() override;
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
  _innerCollection.Push(std::forward<TItem>(item));
  _asyncSemaphore.Release();
}

template <typename TItem>
RStein::AsyncCpp::Tasks::Task<void> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::AddAsync(const TItem& item)
{
   Add(item);
  return Tasks::GetCompletedTask();
}

template <typename TItem>
RStein::AsyncCpp::Tasks::Task<void> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::AddAsync(TItem&& item)
{
  Add(std::forward<TItem>(item));
  return Tasks::GetCompletedTask();
}

template <typename TItem>
RStein::AsyncCpp::Tasks::Task<TItem> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::TakeAsync()
{
  co_await _asyncSemaphore.WaitAsync().ConfigureAwait(false);
  auto retValue = _innerCollection.TryPop();
  if (!retValue)
  {
    throw std::logic_error("Could not take item");
  }

  co_return retValue.value();
}

template <typename TItem>
RStein::AsyncCpp::Tasks::Task<TItem> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::TakeAsync(CancellationToken cancellationToken)
{
  co_await _asyncSemaphore.WaitAsync(cancellationToken).ConfigureAwait(false);
  auto retValue = _innerCollection.TryPop();
  if (!retValue)
  {
    throw std::logic_error("Could not take item");
  }

  co_return retValue.value();
}

template <typename TItem>
std::vector<TItem> RStein::AsyncCpp::AsyncPrimitives::SimpleAsyncProducerConsumerCollection<TItem>::TryTakeAll()
{
  return _innerCollection.PopAll();
}
