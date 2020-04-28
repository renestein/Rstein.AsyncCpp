#include "../AsyncPrimitives/CancellationTokenSource.h"
#include "../AsyncPrimitives/OperationCanceledException.h"
#include "../Schedulers/SimpleThreadPool.h"
#include "../Schedulers/ThreadPoolScheduler.h"
#include "../Tasks/Task.h"
#include "../Tasks/TaskCombinators.h"
#include "../Tasks/TaskFactory.h"
#include <gtest/gtest.h>
#include <future>
#include <string>

using namespace testing;
using namespace RStein::AsyncCpp::Tasks;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace RStein::AsyncCpp::Schedulers;
using namespace std;

namespace RStein::AsyncCpp::TasksTest
{
  class TaskTest : public Test
  {
  public:

    future<void> ContinueWithWhenUsingAwaiterThenTaskIsResumedImpl()
    {
      auto func = []
      {
        this_thread::sleep_for(100ms);
      };

      auto task = TaskFactory::Run(func);

      co_await task;
    }

    future<string> ContinueWithWhenUsingTaskTAwaiterThenTaskIsCompletedWithExpectedValueImpl(string expectedValue)
    {
      auto result = co_await TaskFactory::Run([expectedValue]
      {
        return expectedValue;
      });
      co_return result;
    }

    Task<bool> WhenAllWhenTaskThrowsExceptionThenAllTasksCompletedImpl()
    {
      auto task1 = TaskFactory::Run([]
      {
        this_thread::sleep_for(100ms);
        return 10;
      });

      auto task2 = TaskFactory::Run([]
      {
        this_thread::sleep_for(50ms);
        throw std::invalid_argument{""};
      });

      try
      {
        co_await WhenAll(task1, task2);
      }
      catch (AggregateException&)
      {
      }

      co_return task1.IsCompleted() && task2.IsCompleted();
    }

    Task<void> WhenAllWhenTaskThrowsExceptionThenThrowsAggregateExceptionImpl()
    {
      auto task1 = TaskFactory::Run([]
      {
        this_thread::sleep_for(100ms);
        return 10;
      });

      auto task2 = TaskFactory::Run([]
      {
        this_thread::sleep_for(50ms);
        throw std::invalid_argument{""};
      });

      co_await WhenAll(task1, task2);
    }

    Task<int> WhenAnyWhenFirstTaskCompletedThenRetunsIndex0Impl()
    {
      TaskCompletionSource<void> waitSecondTaskTcs;
      auto task1 = TaskFactory::Run([]
      {
        this_thread::sleep_for(1ms);
        return 10;
      });

      auto task2 = TaskFactory::Run([waitSecondTaskTcs]
      {
        waitSecondTaskTcs.GetTask().Wait();
      });

      auto taskIndex = co_await WhenAny(task1, task2);
      waitSecondTaskTcs.TrySetResult();
      co_return taskIndex;
    }

    Task<int> WhenAnyWhenSecondTaskCompletedThenReturnsIndex1Impl()
    {
      TaskCompletionSource<void> waitFirstTaskTcs;
      auto task1 = TaskFactory::Run([waitFirstTaskTcs]
      {
        waitFirstTaskTcs.GetTask().Wait();
        return 10;
      });

      auto task2 = TaskFactory::Run([]
      {
        this_thread::sleep_for(1ms);
      });

      auto taskIndex = co_await WhenAny(task1, task2);
      waitFirstTaskTcs.TrySetResult();
      co_return taskIndex;
    }
  };

  TEST_F(TaskTest, RunWhenHotTaskCreatedThenTaskIsCompleted)
  {
    bool taskRun = false;

    auto task = TaskFactory::Run([&taskRun]
    {
      taskRun = true;
    });
    task.Wait();

    ASSERT_TRUE(taskRun);
    auto taskState = task.State();
    ASSERT_EQ(TaskState::RunToCompletion, taskState);
  }

  TEST_F(TaskTest, RunWhenUsingExplicitSchedulerThenExplicitSchedulerRunTaskFunc)
  {
    SimpleThreadPool threadPool{1};
    auto explicitTaskScheduler{make_shared<ThreadPoolScheduler>(threadPool)};
    explicitTaskScheduler->Start();
    Scheduler::SchedulerPtr taskScheduler{};

    auto task = TaskFactory::Run([&taskScheduler]
    {
      taskScheduler = Scheduler::CurrentScheduler();
    }, explicitTaskScheduler);

    task.Wait();
    explicitTaskScheduler->Stop();
    ASSERT_EQ(taskScheduler.get(), explicitTaskScheduler.get());
  }


  TEST_F(TaskTest, RunWhenUnspecifiedSchedulerThenDefaultSchedulerRunTaskFunc)
  {
    Scheduler::SchedulerPtr taskScheduler{};

    auto task = TaskFactory::Run([&taskScheduler]
    {
      taskScheduler = Scheduler::CurrentScheduler();
    });

    task.Wait();
    ASSERT_EQ(taskScheduler.get(), Scheduler::DefaultScheduler().get());
  }


  TEST_F(TaskTest, ContinueWithWhenAntecedentTaskCompletedThenContinuationRun)
  {
    std::promise<void> startTaskPromise;

    bool continuationRun = false;

    auto task = TaskFactory::Run([future=startTaskPromise.get_future().share()]
    {
      future.wait();
    });

    auto continuationTask = task.ContinueWith([&continuationRun](const auto& task)
    {
      continuationRun = true;
    });

    startTaskPromise.set_value();
    continuationTask.Wait();

    ASSERT_TRUE(continuationRun);
    auto continuationState = continuationTask.State();
    ASSERT_EQ(TaskState::RunToCompletion, continuationState);
  }


  TEST_F(TaskTest, ContinueWithWhenAntecedentTaskAlreadyCompletedThenContinuationRun)
  {
    bool continuationRun = false;

    auto task = TaskFactory::Run([]
    {
    });

    auto continuationTask = task.ContinueWith([&continuationRun](const auto& task)
    {
      continuationRun = true;
    });

    continuationTask.Wait();

    ASSERT_TRUE(continuationRun);
    auto continuationState = continuationTask.State();
    ASSERT_EQ(TaskState::RunToCompletion, continuationState);
  }


  TEST_F(TaskTest, IsFaultedWhenTaskThrowsExceptionThenReturnsTrue)
  {
    auto task = TaskFactory::Run([]
    {
      throw invalid_argument{"bad arg"};
    });

    try
    {
      task.Wait();
    }
    catch (const invalid_argument&)
    {
    }

    auto isFaulted = task.IsFaulted();
    ASSERT_TRUE(isFaulted);
    auto taskState = task.State();
    ASSERT_EQ(TaskState::Faulted, taskState);
  }


  TEST_F(TaskTest, WaitWhenTaskThrowsExceptionThenRethrowsException)
  {
    auto task = TaskFactory::Run([]
    {
      throw invalid_argument{"bad arg"};
    });

    ASSERT_THROW(task.Wait(), invalid_argument);
  }


  TEST_F(TaskTest, WaitWhenTaskCanceledThenThrowsOperationCanceledException)
  {
    auto cts = CancellationTokenSource::Create();
    cts->Cancel();
    auto task = TaskFactory::Run([]
    {
      throw invalid_argument{"bad arg"};
    }, cts->Token());

    ASSERT_THROW(task.Wait(), OperationCanceledException);
  }


  TEST_F(TaskTest, IsCanceledWhenTaskCanceledThenReturnsTrue)
  {
    auto cts = CancellationTokenSource::Create();
    cts->Cancel();
    auto task = TaskFactory::Run([]
    {
      throw invalid_argument{"bad arg"};
    }, cts->Token());

    auto isCanceled = task.IsCanceled();
    ASSERT_TRUE(isCanceled);
    auto taskState = task.State();
    ASSERT_EQ(TaskState::Canceled, taskState);
  }


  TEST_F(TaskTest, ContinueWithWhenAntecedentTaskAlreadyCompletedThenContinuationSeesExpectedException)
  {
    auto task = TaskFactory::Run([]
    {
      throw invalid_argument{"invalid arg in test"};
    });

    auto continuationTask = task.ContinueWith([](const auto& task)
    {
      task.Wait();
    });

    ASSERT_THROW(continuationTask.Wait(), invalid_argument);
  }


  TEST_F(TaskTest, ResultWhenTaskTCompletedThenReturnExpectedValue)
  {
    const int EXPECTED_VALUE = 42;

    auto task = TaskFactory::Run([EXPECTED_VALUE]()
    {
      return EXPECTED_VALUE;
    });
    auto result = task.Result();


    ASSERT_EQ(EXPECTED_VALUE, result);
  }


  TEST_F(TaskTest, ResultWhenTaskTCompletedThenContinuationSeesExpectedValue)
  {
    const int EXPECTED_VALUE = 42;

    auto continuationTask = TaskFactory::Run([EXPECTED_VALUE]()
        {
          return EXPECTED_VALUE;
        })
        .ContinueWith([](const auto& previous)
        {
          return previous.Result();
        });

    auto result = continuationTask.Result();


    ASSERT_EQ(EXPECTED_VALUE, result);
  }

  TEST_F(TaskTest, ContinueWithWhenUsingAwaiterThenTaskIsResumed)
  {
    ContinueWithWhenUsingAwaiterThenTaskIsResumedImpl().get();
    SUCCEED();
  }

  TEST_F(TaskTest, ContinueWithWhenUsingTaskTAwaiterThenTaskIsCompletedWithExpectedValue)
  {
    const string EXPECTED_VALUE = "Test String Awaiter";

    auto result = ContinueWithWhenUsingTaskTAwaiterThenTaskIsCompletedWithExpectedValueImpl(EXPECTED_VALUE).get();

    ASSERT_EQ(EXPECTED_VALUE, result);
  }

  TEST_F(TaskTest, ContinueWithWhenUsingExplicitSchedulerThenContinuationRunOnExplicitScheduler)
  {
    auto task = TaskFactory::Run([]
    {
      return 10;
    });
    auto capturedContinuationScheduler = Scheduler::SchedulerPtr{};
    SimpleThreadPool threadPool{1};
    auto continuationScheduler = make_shared<ThreadPoolScheduler>(threadPool);
    continuationScheduler->Start();

    task.ContinueWith([&capturedContinuationScheduler](auto _)
        {
          capturedContinuationScheduler = Scheduler::CurrentScheduler();
        }, continuationScheduler)
        .Wait();

    ASSERT_EQ(continuationScheduler.get(), capturedContinuationScheduler.get());

    continuationScheduler->Stop();
  }

  TEST_F(TaskTest, ContinueWithWhenSchedulerNotSetThenContinuationRunOnDefaultScheduler)
  {
    auto task = TaskFactory::Run([]
    {
      return 10;
    });
    auto capturedContinuationScheduler = Scheduler::SchedulerPtr{};
    task.ContinueWith([&capturedContinuationScheduler](auto _)
        {
          capturedContinuationScheduler = Scheduler::CurrentScheduler();
        })
        .Wait();

    ASSERT_EQ(Scheduler::DefaultScheduler().get(), capturedContinuationScheduler.get());
  }

  TEST_F(TaskTest, WaitAllWhenReturnsThenAllTasksAreCompleted)
  {
    auto task1 = TaskFactory::Run([]
    {
      this_thread::sleep_for(100ms);
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(50ms);
    });

    WaitAll(task1, task2);
    ASSERT_TRUE(task1.State() == TaskState::RunToCompletion);
    ASSERT_TRUE(task2.State() == TaskState::RunToCompletion);
  }


  TEST_F(TaskTest, WaitAllWhenTaskThrowsExceptionThenThrowsAggregateException)
  {
    auto task1 = TaskFactory::Run([]
    {
      this_thread::sleep_for(100ms);
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(50ms);
      throw std::invalid_argument{""};
    });

    try
    {
      WaitAll(task1, task2);
    }
    catch (const AggregateException& exception)
    {
      try
      {
        ASSERT_EQ(exception.Exceptions().size(), 1);
        rethrow_exception(exception.FirstExceptionPtr());
      }
      catch (const invalid_argument&)
      {
        SUCCEED();
        return;
      }
    }

    FAIL();
  }

  TEST_F(TaskTest, WaitAllWhenTaskThrowsExceptionThenAllTasksCompleted)
  {
    auto task1 = TaskFactory::Run([]
    {
      this_thread::sleep_for(100ms);
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(50ms);
      throw std::invalid_argument{""};
    });

    try
    {
      WaitAll(task1, task2);
    }
    catch (const AggregateException&)
    {
    }

    ASSERT_TRUE(task1.IsCompleted());
    ASSERT_TRUE(task2.IsCompleted());
  }


  TEST_F(TaskTest, WhenAllWhenReturnsThenAllTasksAreCompleted)
  {
    auto task1 = TaskFactory::Run([]
    {
      this_thread::sleep_for(100ms);
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(50ms);
    });

    WhenAll(task1, task2).Wait();
    ASSERT_TRUE(task1.State() == TaskState::RunToCompletion);
    ASSERT_TRUE(task2.State() == TaskState::RunToCompletion);
  }

  TEST_F(TaskTest, WhenAllWhenTaskThrowsExceptionThenAllTasksCompleted)
  {
    auto allTasksCompleted = WhenAllWhenTaskThrowsExceptionThenAllTasksCompletedImpl().Result();

    ASSERT_TRUE(allTasksCompleted);
  }


  TEST_F(TaskTest, WhenAllWhenTaskThrowsExceptionThenThrowsAggregateException)
  {
    try
    {
      WhenAllWhenTaskThrowsExceptionThenThrowsAggregateExceptionImpl().Wait();
    }
    catch (const AggregateException& exception)
    {
      try
      {
        ASSERT_EQ(exception.Exceptions().size(), 1);
        rethrow_exception(exception.FirstExceptionPtr());
      }
      catch (const invalid_argument&)
      {
        SUCCEED();
        return;
      }
    }
    FAIL();
  }


  TEST_F(TaskTest, WaitAnyWhenFirstTaskCompletedThenRetunsIndex0)
  {
    const int EXPECTED_TASK_INDEX = 0;
    TaskCompletionSource<void> waitSecondTaskTcs;
    auto task1 = TaskFactory::Run([]
    {
      this_thread::sleep_for(1ms);
      return 10;
    });

    auto task2 = TaskFactory::Run([waitSecondTaskTcs]
    {
      waitSecondTaskTcs.GetTask().Wait();
    });

    auto taskIndex = WaitAny(task1, task2);
    waitSecondTaskTcs.TrySetResult();

    ASSERT_EQ(EXPECTED_TASK_INDEX, taskIndex);
  }


  TEST_F(TaskTest, WaitAnyWhenSecondTaskCompletedThenReturnsIndex1)
  {
    const int EXPECTED_TASK_INDEX = 1;
    TaskCompletionSource<void> waitFirstTaskTcs;
    auto task1 = TaskFactory::Run([waitFirstTaskTcs]
    {
      waitFirstTaskTcs.GetTask().Wait();
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(1ms);
    });

    auto taskIndex = WaitAny(task1, task2);
    waitFirstTaskTcs.TrySetResult();
    ASSERT_EQ(EXPECTED_TASK_INDEX, taskIndex);
  }

  TEST_F(TaskTest, WhenAnyWhenFirstTaskCompletedThenRetunsIndex0)
  {
    const int EXPECTED_TASK_INDEX = 0;

    auto taskIndex = WhenAnyWhenFirstTaskCompletedThenRetunsIndex0Impl().Result();
    ASSERT_EQ(EXPECTED_TASK_INDEX, taskIndex);
  }


  TEST_F(TaskTest, WhenAnyWhenSecondTaskCompletedThenReturnsIndex1)
  {
    const int EXPECTED_TASK_INDEX = 1;
    auto taskIndex = WhenAnyWhenSecondTaskCompletedThenReturnsIndex1Impl().Result();
    ASSERT_EQ(EXPECTED_TASK_INDEX, taskIndex);
  }

  TEST_F(TaskTest, TaskFromResultWhenWrappingValueThenTaskContainsWrappedValue)
  {
    const int EXPECTED_TASK_VALUE = 1234;

    auto task = TaskFromResult(1234);

    auto taskValue = task.Result();
    ASSERT_EQ(EXPECTED_TASK_VALUE, taskValue);
  }

  TEST_F(TaskTest, GetCompletedWhenCalledThenReturnCompletedTask)
  {
    auto task = GetCompletedTask();
    ASSERT_TRUE(task.IsCompleted());
  }

  TEST_F(TaskTest, TaskFromExceptionWhenWaitingForTaskThenThrowsExpectedException)
  {
    auto task = TaskFromException<string>(make_exception_ptr(logic_error("")));

    try
    {
      task.Wait();
    }
    catch (const logic_error&)
    {
      SUCCEED();
      return;
    }

    FAIL();
  }


  TEST_F(TaskTest, TaskFromCanceledWhenWaitingForTaskThenThrowsOperationCanceledException)
  {
    auto task = TaskFromCanceled<string>();

    try
    {
      task.Wait();
    }
    catch (const OperationCanceledException&)
    {
      SUCCEED();
      return;
    }

    FAIL();
  }

  TEST_F(TaskTest, FmapWhenMappingTaskThenMappedTaskHasExpectedResult)
  {
    const string EXPECTED_VALUE = "100";
    auto srcTask = TaskFromResult(10);

    auto mappedTask = Fmap(
                           Fmap(srcTask, [](int value)
                           {
                             return value * 10;
                           }),
                           [](int value)
                           {
                             return to_string(value);
                           });

    ASSERT_EQ(EXPECTED_VALUE, mappedTask.Result());
  }


  TEST_F(TaskTest, FmapWhenMappingTaskAndThrowsExceptionThenMappedTaskHasCorectException)
  {
    const string EXPECTED_VALUE = "100";
    auto srcTask = TaskFromResult(10);

    auto mappedTask = Fmap(
                           Fmap(srcTask, [](int value)
                           {
                             throw std::invalid_argument{""};
                             return value * 10;
                           }),
                           [](int value)
                           {
                             return to_string(value);
                           });

    ASSERT_THROW(mappedTask.Result(), invalid_argument);
  }

  TEST_F(TaskTest, FmapWhenMappingAndTaskIsCanceledThenMappedTaskIsCanceled)
  {
    const string EXPECTED_VALUE = "100";
    auto srcTask = TaskFromResult(10);
    auto cts = CancellationTokenSource::Create();
    cts->Cancel();
    auto mappedTask = Fmap(
                           Fmap(srcTask, [ct=cts->Token()](int value)
                           {
                             ct->ThrowIfCancellationRequested();
                             return value * 10;
                           }),
                           [](int value)
                           {
                             return to_string(value);
                           });

    ASSERT_TRUE(mappedTask.IsCanceled());
    ASSERT_THROW(mappedTask.Result(), OperationCanceledException);
  }

  TEST_F(TaskTest, MonadRightIdentityLaw)
  {
    auto leftMonad = TaskFromResult(10);
    auto rightMonad = Fbind(leftMonad, [](int unwrappedValue)
    {
      return TaskFromResult(unwrappedValue);
    });

    ASSERT_EQ(leftMonad.Result(), rightMonad.Result());
  }


  TEST_F(TaskTest, MonadLeftIdentityLaw)
  {
    const int initialValue = 10;

    auto selector = [](int value)
    {
      auto transformedvalue = value * 100;
      return TaskFromResult(transformedvalue);
    };

    auto rightMonad = selector(initialValue);

    auto leftMonad = Fbind(TaskFromResult(initialValue), selector);

    ASSERT_EQ(leftMonad.Result(), rightMonad.Result());
  }

  TEST_F(TaskTest, MonadAssociativityLaw)
  {
    const int initialValue = 10;

    auto initialMonad = TaskFromResult(initialValue);
    auto gTransformFunc = [](int value)
    {
      auto transformedValue = value * 10;
      return TaskFromResult(transformedValue);
    };

    auto hTransformFunc = [](int value)
    {
      auto transformedValue = value / 2;
      return TaskFromResult(transformedValue);
    };

    auto leftMonad = Fbind(Fbind(initialMonad, gTransformFunc), hTransformFunc);
    auto rightMonad = Fbind(initialMonad, [&gTransformFunc, &hTransformFunc](auto&& value)
    {
      return Fbind(gTransformFunc(value), hTransformFunc);
    });

    cout << "Result: " << leftMonad.Result();
    ASSERT_EQ(leftMonad.Result(), rightMonad.Result());
  }
}
