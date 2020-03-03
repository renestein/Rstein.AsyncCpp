#include <gtest/gtest.h>

#include "../AsyncPrimitives/AsyncSemaphore.h"
#include <experimental/resumable>

using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace testing;
using namespace std;
using namespace std::experimental;

namespace RStein::AsyncCpp::AsyncPrimitivesTest
{

  class AsyncSemaphoreTest : public testing::Test
  {
  protected:
    [[nodiscard]] std::future<void> waitAsyncWhenSemaphoreIsReadyThenReturnsReadyFutureImpl() const
    {
      const auto maxCount{1};
      const auto initialCount{1};

      AsyncSemaphore semaphore{maxCount, initialCount};
      
      auto future = semaphore.WaitAsync();
      co_await future;
      semaphore.Release();
    }

    [[nodiscard]] std::future<void> waitAsyncWhenSemaphoreIsNotReadyThenFutureIsCompletedLaterImpl() const
    {
      const auto maxCount{1};
      const auto initialCount{0};

      AsyncSemaphore semaphore{maxCount, initialCount};
      
      auto waitFuture = semaphore.WaitAsync();

      co_await async(launch::async, [&semaphore]() {semaphore.Release();});
      co_await waitFuture;
      semaphore.Release();
    }
  };

  TEST_F(AsyncSemaphoreTest, CtorWhenMaxCountLessThanZeroThenThrowsInvalidArgument)
  {
    const auto validInitialCount{ 1 };
    const auto invalidMaxCount{ -1 };

    ASSERT_THROW(AsyncSemaphore semaphore(invalidMaxCount, validInitialCount), invalid_argument);
  }


  TEST_F(AsyncSemaphoreTest, CtorWhenInitialCountGreaterThanMaxCountThenThrowsInvalidArgument)
  {
    const auto invalidInitialCount{ 2 };
    const auto validMaxCount{ 1 };

    ASSERT_THROW(AsyncSemaphore semaphore(validMaxCount, invalidInitialCount), invalid_argument);
  }

  TEST_F(AsyncSemaphoreTest, WaitAsyncWhenSemaphoreIsReadyThenReturnsReadyFuture)
  {
    waitAsyncWhenSemaphoreIsReadyThenReturnsReadyFutureImpl().get();
  }

  TEST_F(AsyncSemaphoreTest, WaitAsyncWhenSemaphoreIsNotReadyThenFutureIsCompletedLater)
  {
    waitAsyncWhenSemaphoreIsNotReadyThenFutureIsCompletedLaterImpl().get();
  }

}