#include "../../RStein.AsyncCpp/AsyncPrimitives/AsyncMutex.h"
#include "../../RStein.AsyncCpp/Tasks/Task.h"
#include "../../RStein.AsyncCpp/Tasks/TaskCompletionSource.h"
#include "../../RStein.AsyncCpp/Tasks/TaskFactory.h"


#include <gtest/gtest.h>


using namespace testing;
using namespace RStein::AsyncCpp::Tasks;

using namespace RStein::AsyncCpp::AsyncPrimitives;
namespace RStein::AsyncCpp::AsyncPrimitivesTest
{
  class AsyncMutexTest : public Test
  {
  public:

    Task<void> UnlockWhenDifferentMutexThenThrowsLogicErrorImpl()
    {
      AsyncMutex asyncMutex;
      AsyncMutex unrelatedMutex;
      auto locker = asyncMutex.Lock();
      co_await locker;

      unrelatedMutex.Unlock(locker);

    }

    Task<bool> MoveWhenUnlockingMoveTargetLockerThenLockIsReleasedImpl()
    {
      AsyncMutex asyncMutex;
      auto locker = asyncMutex.Lock();
      co_await locker;

      auto afterMoveLocker = std::move(locker);
      afterMoveLocker.Unlock();

      auto nextLocker = asyncMutex.Lock();
      co_await nextLocker;
      co_return true;
    }

    Task<void> MoveWhenUnlockingMoveSourceLockerThenThrowsLogicErrorImpl()
    {
      AsyncMutex asyncMutex;
      auto locker = asyncMutex.Lock();
      co_await locker;

      auto afterMoveLocker = std::move(locker);

      locker.Unlock();  // NOLINT(hicpp-invalid-access-moved)
    }


    Task<size_t> LockWhenCalledThenExpectedNumberItemsAreInUnsafeCollectionImpl(int itemsCount)
    {
      std::vector<int> items;
      AsyncMutex asyncMutex;

      for (int i = 0; i < itemsCount; ++i)
      {
        //test only repeated Lock/Unlock without concurrency
        auto locker = asyncMutex.Lock();
        co_await locker;
        items.push_back(i);
      }

      co_return items.size();
    }

    Task<bool> UnlockWhenExplicitUnlockThenNextLockCallSucceedImpl()
    {
      AsyncMutex asyncMutex;
      auto locker = asyncMutex.Lock();
      co_await locker;

      asyncMutex.Unlock(locker);
      auto nextLocker = asyncMutex.Lock();
      co_await nextLocker;

      co_return true;
    }

    
    Task<bool> UnlockWhenExplicitUnlockOnLockerThenNextLockCallSucceedImpl()
    {
      AsyncMutex asyncMutex;
      auto locker = asyncMutex.Lock();
      co_await locker;

      locker.Unlock();
      auto nextLocker = asyncMutex.Lock();
      co_await nextLocker;

      co_return true;
    }

    Task<int> WhenUsingThreadPoolAndSumVariableThenAccessToSumVariableIsSynchronizedImpl(int tasksCount)
    {

      struct TaskDataHolder
      {
        TaskCompletionSource<void>* StartTasksTcs;
        AsyncMutex* AsyncMutex;
        long* Result;
      };

        TaskCompletionSource<void> startTasksTcs{};
        AsyncSemaphore waitForTasksSemaphore{tasksCount, 0};
        AsyncMutex asyncMutex{};
        long result = 0;
        TaskDataHolder dataHolder
        {
          &startTasksTcs,
          &asyncMutex,
          &result
        };

      std::vector<Task<void>> tasks;

      for (int i = 0; i < tasksCount; ++i)
      {
      //Only for tests - do not use capturing lambdas that are coroutines  
        auto task = TaskFactory::Run([dataHolder=dataHolder]()-> Task<void>
                                {
                                  co_await dataHolder.StartTasksTcs->GetTask();
                                  auto locker =  dataHolder.AsyncMutex->Lock();
                                  co_await locker;
                                  (*dataHolder.Result)++;
                                  *dataHolder.Result += 2;
                                  *dataHolder.Result += 2;

                                  *dataHolder.Result -= 2;
                                  *dataHolder.Result -= 2;
                                  *dataHolder.Result += 2;
                                  *dataHolder.Result += 2;
                                  *dataHolder.Result -= 2;
                                  *dataHolder.Result -= 2;
                                });
        
        tasks.push_back(task);
      }      

      startTasksTcs.TrySetResult();
      for (int i = 0; i < tasksCount; ++i)
      {
        co_await tasks[i];
      }

      co_return *dataHolder.Result;
    }
  };

  TEST_F(AsyncMutexTest, LockWhenCalledThenExpectedNumberItemsAreInUnsafeCollection)
  {
    const size_t ADD_ITEMS_COUNT = 10;

    auto itemsCount = LockWhenCalledThenExpectedNumberItemsAreInUnsafeCollectionImpl(ADD_ITEMS_COUNT).Result();

    ASSERT_EQ(ADD_ITEMS_COUNT, itemsCount);
  }

  TEST_F(AsyncMutexTest, UnlockWhenExplicitUnlockThenNextLockCallSucceed)
  {
    
    auto nextLockSucceeded = UnlockWhenExplicitUnlockThenNextLockCallSucceedImpl().Result();

    ASSERT_TRUE(nextLockSucceeded);
  }

  TEST_F(AsyncMutexTest, UnlockWhenExplicitUnlockOnLockerThenNextLockCallSucceed)
  {
    auto nextLockSucceeded = UnlockWhenExplicitUnlockOnLockerThenNextLockCallSucceedImpl().Result();

    ASSERT_TRUE(nextLockSucceeded);
  }

  TEST_F(AsyncMutexTest, UnlockWhenDifferentMutexThenThrowsLogicError)
  {

    auto resultTask = UnlockWhenDifferentMutexThenThrowsLogicErrorImpl();

    ASSERT_THROW(resultTask.Wait(), std::logic_error);
  }

  TEST_F(AsyncMutexTest, MoveWhenUnlockingMoveTargetLockerThenLockIsReleased)
  {

    auto nextLockSucceeded = MoveWhenUnlockingMoveTargetLockerThenLockIsReleasedImpl().Result();

    ASSERT_TRUE(nextLockSucceeded);
  }

  TEST_F(AsyncMutexTest, MoveWhenUnlockingMoveSourceLockerThenThrowsLogicError)
  {

    auto afterMoveTask = MoveWhenUnlockingMoveSourceLockerThenThrowsLogicErrorImpl();

    ASSERT_THROW(afterMoveTask.Wait(), std::logic_error);
  }

  TEST_F(AsyncMutexTest, WhenUsingThreadPoolAndSumVariableThenAccessToSumVariableIsSychronized)
  {
    const int TasksCount = 1000;
    const int EXPECTED_SUM = TasksCount;

    auto result = WhenUsingThreadPoolAndSumVariableThenAccessToSumVariableIsSynchronizedImpl(TasksCount).Result();

    ASSERT_EQ(EXPECTED_SUM, result);
  }

  

}
