#include <gtest/gtest.h>

#include "../AsyncPrimitives/AsyncSemaphore.h"
#include <experimental/resumable>
#include <experimental/generator>
#include <xutility>

using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace testing;
using namespace std;
using namespace std::experimental;

namespace RStein::AsyncCpp::AsyncPrimitivesTest
{

  class AsyncSemaphoreTest : public testing::Test
  {
  protected:
    [[nodiscard]] future<void> waitAsyncWhenSemaphoreIsReadyThenReturnsReadyFutureImpl() const
    {
      const auto maxCount{ 1 };
      const auto initialCount{ 1 };

      AsyncSemaphore semaphore{ maxCount, initialCount };

      auto future = semaphore.WaitAsync();
      
      co_await future;
      semaphore.Release();
    }

    [[nodiscard]] future<void> waitAsyncWhenSemaphoreIsNotReadyThenFutureIsCompletedLaterImpl() const
    {
      const auto maxCount{ 1 };
      const auto initialCount{ 0 };

      AsyncSemaphore semaphore{ maxCount, initialCount };

      auto waitFuture = semaphore.WaitAsync();

      co_await async(launch::async, [&semaphore]() {semaphore.Release(); });
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
      const auto maxCount{ 1 };
      const auto initialCount{ 0 };

      AsyncSemaphore semaphore{ maxCount, initialCount };
      std::vector<future<future<int>>> futures;
      futures.reserve(taskCount);

      int result = 0;
      for (auto i : generate(taskCount))
      {
        /*Action action {&semaphore, &result, i};*/
        //lambda and nested futures causes access violation

        packaged_task<future<int> (int*, AsyncSemaphore*, int)> task{ [](int* result, AsyncSemaphore* semaphore, int i)->future<int>
          {
           
            co_await semaphore->WaitAsync();
            (*result)++;
            semaphore->Release();
            co_return i;

          } };

        //Workaround, do not create and immediately throw away threads
        auto taskFuture = task.get_future();
        thread runner{std::move(task), &result, &semaphore, i};
        runner.detach();
        /*async(launch::async, [&result, &semaphore, i]()->future<int>
        {
          cout << "running " << i;
          co_await semaphore.WaitAsync();
          cout << "WaitAsync " << i;
          result++;
          semaphore.Release();
          co_return i;

        });*///Unwrap nested future

        //auto taskFuture = async(launch::async, std::move(action));//Unwrap nested future
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

    generator<int> generate(int count)
    {
      for (int i = 0; i < count; ++i)
      {
        co_yield i;
      }
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


  TEST_F(AsyncSemaphoreTest, WaitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronized)
  {
    const auto TASKS_COUNT = 100;
    const auto EXPECTED_RESULT = 100;

    const auto result = waitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronizedImpl(TASKS_COUNT);
    ASSERT_EQ(EXPECTED_RESULT, result);
  }

}