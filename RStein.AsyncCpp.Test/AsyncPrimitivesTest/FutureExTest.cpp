#include "../../RStein.AsyncCpp/AsyncPrimitives/OperationCanceledException.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/FutureEx.h"

#include <future>
#include <gtest/gtest.h>
#include <chrono>

using namespace testing;
using namespace std;
using namespace RStein::AsyncCpp::AsyncPrimitives;

namespace RStein::AsyncCpp::AsyncPrimitivesTest
{
  class SharedFutureTest : public Test
  {
  protected:
    [[nodiscard]] shared_future<void> awaiterWhenUsingThenCoAwaitWorksImpl() const
    {
      promise<void> promise;
      auto sharedFuture = promise.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise]
      {
        promise.set_value();
      }).share();
      co_await sharedFuture;
    }

    [[nodiscard]] shared_future<void> awaiterWhenExceptionalVoidFutureThenCoAwaitThrowsInvalidArgumentExceptionImpl() const
    {
      promise<void> promise;
      auto sharedFuture = promise.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise]
      {
        promise.set_exception(make_exception_ptr(std::invalid_argument{"test exception"}));
      }).share();
      co_return co_await sharedFuture;
    }

    [[nodiscard]] shared_future<int> awaiterWhenResultFutureThenCoAwaitReturnsFutureValue(int expectedValue) const
    {
      promise<int> promise;
      auto sharedFuture = promise.get_future().share();

      cerr << "Creating promise thread...\n";
      std::thread setPromiseThread([&promise, expectedValue]()
      {
        this_thread::sleep_for(1s);
        promise.set_value(expectedValue);
      });

      cerr << "Awaiting shared future...\n";
      auto retValue = co_await sharedFuture;
      setPromiseThread.join();
      co_return retValue;
    }

    [[nodiscard]] shared_future<int> awaiterWhenExceptionalIntFutureThenCoAwaitThrowsOperationCanceledExceptionImpl()
    {
      promise<int> promise;
      auto sharedFuture = promise.get_future().share();

      cerr << "Creating promise thread...\n";
      std::thread setPromiseThread([&promise]()
      {
        promise.set_exception(make_exception_ptr(OperationCanceledException{}));
      });

      cerr << "Awaiting shared future...\n";
      setPromiseThread.join();

      auto retValue = co_await sharedFuture;
      co_return retValue;
    }

    [[nodiscard]] std::shared_future<void> sharedFutureWhenUsingPromiseReturnThenDoesNotThrowImpl() const
    {
      promise<void> promise1;
      auto sharedFuture = promise1.get_future().share();
      co_await async([&promise1]
      {
        promise1.set_value();
      });

      co_await sharedFuture;
      promise<int> promise2;
      auto future = promise2.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise2]
      {
        promise2.set_value(101);
      });

      co_await future;
    }

    [[nodiscard]] std::shared_future<void> sharedFutureWhenUsingPromiseReturnAndCoroutineThrowsExceptionThenPromiseThrowsSameExceptionImpl() const
    {
      promise<void> promise1;
      auto sharedFuture = promise1.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise1]
      {
        promise1.set_value();
      });

      co_await sharedFuture;
      promise<int> promise2;
      auto uniqueFuture = promise2.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise2]
      {
        promise2.set_exception(make_exception_ptr(OperationCanceledException{}));
      });

      co_await uniqueFuture;
    }
    [[nodiscard]] std::shared_future<int> sharedFutureTWhenUsingPromiseReturnAndCoroutineThrowsExceptionThenPromiseThrowsSameExceptionImpl() const
    {
      promise<void> promise1;
      auto sharedFuture = promise1.get_future().share();
      co_await async([&promise1]
      {
        promise1.set_value();
      });

      co_await sharedFuture;
      promise<int> promise2;
      auto uniqueFuture = promise2.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise2]
      {
        promise2.set_exception(make_exception_ptr(OperationCanceledException{}));
      });

      auto retValue = co_await uniqueFuture;

      co_return retValue;
    }

    [[nodiscard]] std::shared_future<int> sharedFutureWhenUsingPromiseReturnThenReturnExpectedValueImpl(int expectedValue)
    {
      promise<void> promise1;
      auto sharedFuture = promise1.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise1]
      {
        promise1.set_value();
      });

      co_await sharedFuture;
      promise<int> promise2;
      auto future = promise2.get_future().share();
      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async([&promise2, expectedValue]
      {
        promise2.set_value(expectedValue);
      });

      auto retValue = co_await future;

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

  TEST_F(SharedFutureTest, SharedFutureWhenUsingPromiseReturnThenDoesNotThrow)
  {
    ASSERT_NO_THROW(sharedFutureWhenUsingPromiseReturnThenDoesNotThrowImpl().get());
  }

  TEST_F(SharedFutureTest, SharedFutureWhenUsingPromiseReturnAndCoroutineThrowsExceptionThenPromiseThrowsSameException)
  {
    ASSERT_THROW(sharedFutureWhenUsingPromiseReturnAndCoroutineThrowsExceptionThenPromiseThrowsSameExceptionImpl().get(),
                 OperationCanceledException);
  }

  TEST_F(SharedFutureTest, SharedFutureTWhenUsingPromiseReturnAndCoroutineThrowsExceptionThenPromiseThrowsSameException)
  {
    ASSERT_THROW(sharedFutureTWhenUsingPromiseReturnAndCoroutineThrowsExceptionThenPromiseThrowsSameExceptionImpl().get(),
                 OperationCanceledException);
  }

  
  TEST_F(SharedFutureTest, SharedFutureWhenUsingPromiseReturnThenReturnExpectedValue)
  {
    const int EXPECTED_VALUE = 42;
    auto retValue = sharedFutureWhenUsingPromiseReturnThenReturnExpectedValueImpl(EXPECTED_VALUE).get();
    ASSERT_EQ(EXPECTED_VALUE, retValue);
  }



}
