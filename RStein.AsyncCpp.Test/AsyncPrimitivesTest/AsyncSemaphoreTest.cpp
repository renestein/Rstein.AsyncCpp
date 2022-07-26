#include <gtest/gtest.h>
#if defined(__clang__)
#include "../../RStein.AsyncCpp/ClangWinSpecific/Coroutine.h"
#elif defined(__cpp_impl_coroutine)
#include <coroutine>
#else
#include <experimental/coroutine>
#endif

#include "../../RStein.AsyncCpp/AsyncPrimitives/AsyncSemaphore.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/OperationCanceledException.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/CancellationTokenSource.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/FutureEx.h"
#include <future>




using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace testing;
using namespace std;
#if defined(__cpp_impl_coroutine) || defined(__clang__)
using namespace std;
#else
using namespace std::experimental;
#endif


namespace RStein::AsyncCpp::AsyncPrimitivesTest
{
  class AsyncSemaphoreTest : public testing::Test
  {
  protected:
    [[nodiscard]] shared_future<void> waitAsyncWhenSemaphoreIsReadyThenReturnsReadyFutureImpl() const
    {
      const auto maxCount{1};
      const auto initialCount{1};

      AsyncSemaphore semaphore{maxCount, initialCount};

      auto future = semaphore.WaitAsync();

      co_await future;
      semaphore.Release();
    }

    [[nodiscard]] shared_future<void> waitAsyncWhenSemaphoreIsNotReadyThenFutureIsCompletedLaterImpl() const
    {
      const auto maxCount{1};
      const auto initialCount{0};

      AsyncSemaphore semaphore{maxCount, initialCount};

      auto waitFuture = semaphore.WaitAsync();

      //Only for tests - do not use capturing lambdas that are coroutines
      co_await async(launch::async, [&semaphore]()
      {
        semaphore.Release();
      }).share();
      co_await waitFuture;
      semaphore.Release();
    }

    [[nodiscard]] int waitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronizedImpl(int taskCount)
    {
      //struct Action
      //{
      //  AsyncSemaphore* Semaphore;
      //  int* Result;
      //  int Id;
      //  future<int> operator()() const
      //  {
      //     co_await Semaphore->WaitAsync();
      //     (*Result)++;
      //     Semaphore->Release();
      //     co_return Id;
      //  }
      //};
      cout << "start";
      const auto maxCount{1};
      const auto initialCount{0};

      AsyncSemaphore semaphore{maxCount, initialCount};
      std::vector<future<shared_future<int>>> futures;
      futures.reserve(taskCount);

      auto result = 0;
      for (auto i = 0; i < taskCount; i++)
      {

        packaged_task<shared_future<int>(int*, AsyncSemaphore*, int)> task{
            [](int* result, AsyncSemaphore* semaphore, int i)-> shared_future<int>
            {
              co_await semaphore->WaitAsync();
              (*result)++;
              semaphore->Release();
              co_return i;
            }
        };

        //Workaround, do not create and immediately throw away threads
        auto taskFuture = task.get_future();
        thread runner{std::move(task), &result, &semaphore, i};
        runner.detach();

        futures.push_back(std::move(taskFuture));
      }
      semaphore.Release();
      for (auto&& future : futures)
      {
        auto nestedFuture = future.get();
        const auto taskId = nestedFuture.get();
        cout << "Task completed: " << taskId << endl;
      }


      return result;
    }

    [[nodiscard]] shared_future<void> disposeWhenCalledThenAllWaitersAreReleasedImpl() const
    {
      const auto WAITERS = 1000;
      const auto maxCount{1};
      const auto initialCount{0};
      AsyncSemaphore semaphore{maxCount, initialCount};
      std::vector<Tasks::Task<void>> waiterTasks;

      for (auto i = 0; i < WAITERS; i++)
      {
        waiterTasks.push_back(semaphore.WaitAsync());
      }
      semaphore.Dispose();
      for (auto& waiterTask : waiterTasks)
      {
        try
        {
          co_await waiterTask; 
        }
        catch (const OperationCanceledException&)
        {
          cerr << "Task cancelled:" << endl;
        }
      }
    }

    [[nodiscard]] shared_future<void> waitAsyncWhenOneWaiterCanceledThenNextWaiterSucceedImpl()
    {
      const auto maxCount{100};
      const auto initialCount{0};
      AsyncSemaphore semaphore{maxCount, initialCount};
      auto cts = CancellationTokenSource{};
      auto cts2 = CancellationTokenSource{};

      auto firstWaiter = semaphore.WaitAsync(cts.Token());
      auto secondWaiter = semaphore.WaitAsync(cts2.Token());

      cts.Cancel();
      try
      {
        co_await firstWaiter;
      }
      catch(OperationCanceledException&)
      {
        cerr << "First task canceled.\n";
      }
      semaphore.Release();
      co_await secondWaiter;
      cerr << "Second task succeeded.\n";
      semaphore.Dispose();
    }
      
  };

  TEST_F(AsyncSemaphoreTest, CtorWhenMaxCountLessThanZeroThenThrowsInvalidArgument)
  {
    const auto validInitialCount{1};
    const auto invalidMaxCount{-1};

    ASSERT_THROW(AsyncSemaphore semaphore(invalidMaxCount, validInitialCount), invalid_argument);
  }


  TEST_F(AsyncSemaphoreTest, CtorWhenInitialCountGreaterThanMaxCountThenThrowsInvalidArgument)
  {
    const auto invalidInitialCount{2};
    const auto validMaxCount{1};

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


  TEST_F(AsyncSemaphoreTest, WaitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronized)
  {
    const auto TASKS_COUNT = 100;
    const auto EXPECTED_RESULT = 100;

    const auto result = waitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronizedImpl(TASKS_COUNT);
    ASSERT_EQ(EXPECTED_RESULT, result);
  }

  TEST_F(AsyncSemaphoreTest, DisposeWhenCalledThenAllWaitersAreReleased)
  {
    disposeWhenCalledThenAllWaitersAreReleasedImpl().get();
  }

  //TODO: Problem in Release mode. Compiler?
#ifdef DEBUG
  TEST_F(AsyncSemaphoreTest, WaitAsyncWhenOneWaiterCanceledThenNextWaiterSucceed)
  {
    waitAsyncWhenOneWaiterCanceledThenNextWaiterSucceedImpl().get();
  }
#endif
}
