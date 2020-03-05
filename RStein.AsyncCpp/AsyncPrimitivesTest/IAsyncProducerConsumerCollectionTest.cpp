#include "../AsyncPrimitives/SimpleAsyncProducerConsumerCollection.h"

#include <experimental/coroutine>
#include <gtest/gtest.h>

using namespace testing;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace std;

//using namespace std::experimental;

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
  };

  using Collections = Types<SimpleAsyncProducerConsumerCollection<int>>;
  TYPED_TEST_SUITE(AsyncProducerConsumerCollectionTest, Collections);

  TYPED_TEST(AsyncProducerConsumerCollectionTest, TakeAsyncWhenCollectionHaveValueThenReturnValue)
  {
    const int EXPECTED_ITEM = 10;
    auto retItem = this->takeAsyncWhenCollectionHaveValueThenReturnValueImpl(EXPECTED_ITEM).get();

    ASSERT_EQ(EXPECTED_ITEM, retItem);
  }
}
