#include "../AsyncPrimitives/FutureEx.h"
#include "../AsyncPrimitives/OperationCanceledException.h"

#include <gtest/gtest.h>
#include <experimental/resumable>
#include <future>
#include <chrono>

using namespace testing;
using namespace std;
using namespace RStein::AsyncCpp::AsyncPrimitives;
namespace RStein::AsyncCpp::AsyncPrimitivesTest
{
  class SharedFutureTest : public Test
  {
    protected:
    [[nodiscard]] future<void> awaiterWhenUsingThenCoAwaitWorksImpl() const
    {
      promise<void> promise;
      auto sharedFuture = promise.get_future().share();
      co_await async([&promise]{promise.set_value();});
      co_await sharedFuture;
    }

    [[nodiscard]] future<void> awaiterWhenExceptionalVoidFutureThenCoAwaitThrowsInvalidArgumentExceptionImpl() const
    {
      promise<void> promise;
      auto sharedFuture = promise.get_future().share();
      co_await async([&promise]{promise.set_exception(make_exception_ptr(std::invalid_argument{"test exception"}));});
      co_return co_await sharedFuture;
    }

    [[nodiscard]] future<int> awaiterWhenResultFutureThenCoAwaitReturnsFutureValue(int expectedValue) const
    {
      promise<int> promise;
      auto sharedFuture = promise.get_future().share();

      cerr << "Creating promise thread...\n";
      std::thread setPromiseThread( [&promise, expectedValue]()
      {
        this_thread::sleep_for(1s);
        promise.set_value(expectedValue);
      });
    
      cerr << "Awaiting shared future...\n";
      auto retValue = co_await sharedFuture;
      setPromiseThread.join();
      co_return retValue;

  }

    [[nodiscard]] future<int> awaiterWhenExceptionalIntFutureThenCoAwaitThrowsOperationCanceledExceptionImpl()
    {
      promise<int> promise;
      auto sharedFuture = promise.get_future().share();

      cerr << "Creating promise thread...\n";
      std::thread setPromiseThread( [&promise]()
      {
        promise.set_exception(make_exception_ptr(OperationCanceledException{}));
      });
    
      cerr << "Awaiting shared future...\n";             
      setPromiseThread.join();

      auto retValue = co_await sharedFuture;
      co_return retValue;
    }

  };

  TEST_F(SharedFutureTest, AwaiterWhenVoidFutureThenCoAwaitWorks)
  {
    awaiterWhenUsingThenCoAwaitWorksImpl().get();
  }

  TEST_F(SharedFutureTest, AwaiterWhenExceptionalVoidFutureThenCoAwaitThrowsInvalidArgumentException)
  {
    ASSERT_THROW(awaiterWhenExceptionalVoidFutureThenCoAwaitThrowsInvalidArgumentExceptionImpl().get(),
                 invalid_argument);
  }

  
  TEST_F(SharedFutureTest, AwaiterWhenResultFutureThenCoAwaitReturnsFutureValue)
  {
    const int EXPECTED_VALUE = 42;
    auto futureValue = awaiterWhenResultFutureThenCoAwaitReturnsFutureValue(EXPECTED_VALUE).get();

    ASSERT_EQ(EXPECTED_VALUE, futureValue);
  }

  TEST_F(SharedFutureTest, AwaiterWhenExceptionalIntFutureThenCoAwaitThrowsOperationCanceledException)
  {
    ASSERT_THROW(awaiterWhenExceptionalIntFutureThenCoAwaitThrowsOperationCanceledExceptionImpl().get(),
                OperationCanceledException);
  }

  
}