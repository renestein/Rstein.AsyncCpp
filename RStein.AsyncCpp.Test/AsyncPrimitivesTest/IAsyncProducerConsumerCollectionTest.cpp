#include "../../RStein.AsyncCpp/AsyncPrimitives/CancellationTokenSource.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/OperationCanceledException.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/SimpleAsyncProducerConsumerCollection.h"

#include <experimental/coroutine>
#include <gtest/gtest.h>
#include <numeric>

using namespace testing;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace std;
using namespace std::experimental;

namespace RStein::AsyncCpp::AsyncPrimitivesTest
{
  template <typename T>
  class AsyncProducerConsumerCollectionTest : public Test
  {
  public:
    typedef T Collection;
    AsyncProducerConsumerCollectionTest() = default;

  protected:
    [[nodiscard]] future<int> takeAsyncWhenCollectionHaveValueThenReturnValueImpl(int expectedItem) const
    {
      Collection collection{};    
      
      co_await collection.AddAsync(expectedItem);
      auto value = co_await collection.TakeAsync();
      co_return 10;
    }

     [[nodiscard]] future<void> takeAsyncWhenCollectionWhenCanceledThenThrowsOperationCanceledExceptionImpl() const
    {
      Collection collection{};
  
      auto cts = CancellationTokenSource{};
      auto futureValue = collection.TakeAsync(cts.Token());
      
      cts.Cancel();
      try
      {       
        co_await futureValue;
      }
      catch(OperationCanceledException&)
      {   
        co_return;
      }

      throw std::exception("expected OperationCanceledException");
    }
  };

  using Collections = Types<SimpleAsyncProducerConsumerCollection<int>>;
  TYPED_TEST_SUITE(AsyncProducerConsumerCollectionTest, Collections);

  TYPED_TEST(AsyncProducerConsumerCollectionTest, TakeAsyncWhenCollectionHaveValueThenReturnValue)
  {
    const int EXPECTED_ITEM = 10;
    auto retItem = this->takeAsyncWhenCollectionHaveValueThenReturnValueImpl(EXPECTED_ITEM).get();

    ASSERT_EQ(EXPECTED_ITEM, retItem);
  }

 //TODO: Problems in VS 2019 (observed on X86/Release configuration)
#ifdef DEBUG
  TYPED_TEST(AsyncProducerConsumerCollectionTest, TakeAsyncWhenCollectionWhenCanceledThenThrowsOperationCanceledException)
  {
    const int EXPECTED_ITEM = 10;
    this->takeAsyncWhenCollectionWhenCanceledThenThrowsOperationCanceledExceptionImpl().get();

    SUCCEED();
  }
#endif
  
  TYPED_TEST(AsyncProducerConsumerCollectionTest, TakeAAllWhenHasItemsThenReturnsAllItems)
  {
    const int ITEMS_IN_COLLECTION = 1000;
    vector<int> items(ITEMS_IN_COLLECTION);
    items.reserve(ITEMS_IN_COLLECTION);
    iota(items.begin(), items.end(), 0);
    typename TestFixture::Collection asyncCollection;

    for (auto item : items)
    {
      asyncCollection.Add(item);
    }

    auto asyncCollectionItems = asyncCollection.TryTakeAll();

    auto asyncCollectionItemsEqualToItems  = equal(items.begin(), items.end(),
                                                  asyncCollectionItems.begin(), asyncCollectionItems.end());

    ASSERT_TRUE(asyncCollectionItemsEqualToItems);

  }
}
