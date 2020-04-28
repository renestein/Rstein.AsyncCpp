#pragma once
#include "CancellationToken.h"
#include "../Tasks/Task.h"


#include <vector>

namespace RStein::AsyncCpp::AsyncPrimitives
{
  template<typename TItem>
  class IAsyncProducerConsumerCollection
  {
  public:

    IAsyncProducerConsumerCollection() = default;
    IAsyncProducerConsumerCollection(const IAsyncProducerConsumerCollection& other) = delete;
    IAsyncProducerConsumerCollection(IAsyncProducerConsumerCollection&& other) noexcept = delete;
    IAsyncProducerConsumerCollection& operator=(const IAsyncProducerConsumerCollection& other) = delete;
    IAsyncProducerConsumerCollection& operator=(IAsyncProducerConsumerCollection&& other) noexcept = delete;
    virtual ~IAsyncProducerConsumerCollection() = default;

    virtual void Add(const TItem& item) = 0;
    virtual void Add(TItem&& item) = 0;
    virtual Tasks::Task<void> AddAsync(const TItem& item) = 0;
    virtual Tasks::Task<void> AddAsync(TItem&& item) = 0;
    virtual Tasks::Task<TItem> TakeAsync()  = 0;
    virtual Tasks::Task<TItem> TakeAsync(CancellationToken cancellationToken) = 0;
    virtual std::vector<TItem> TryTakeAll() = 0;

  };

}
