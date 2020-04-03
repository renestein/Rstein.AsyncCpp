#include "../AsyncPrimitives/CancellationTokenSource.h"
#include "../AsyncPrimitives/OperationCanceledException.h"
#include "../Tasks/Task.h"
#include <gtest/gtest.h>
#include <future>

using namespace testing;
using namespace RStein::AsyncCpp::Tasks;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace std;

TEST(TaskTest, RunWhenHotTaskCreatedThenTaskIsCompleted)
{
  bool taskRun = false;

  auto task = TaskFactory::Run([&taskRun]{taskRun = true;});
  task.Wait();

  ASSERT_TRUE(taskRun);
  auto taskState = task.State();
  ASSERT_EQ(TaskState::RunToCompletion, taskState);
}

TEST(TaskTest, ContinueWithWhenAntecedentTaskCompletedThenContinuationRun)
{

  std::promise<void> startTaskPromise;

  bool continuationRun = false;

  auto task = TaskFactory::Run([future=startTaskPromise.get_future().share()]{future.wait();});

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


TEST(TaskTest, ContinueWithWhenAntecedentTaskAlreadyCompletedThenContinuationRun)
{

  bool continuationRun = false;

  auto task = TaskFactory::Run([]{});

  auto continuationTask = task.ContinueWith([&continuationRun](const auto& task)
  {
    continuationRun = true;  
  });

  continuationTask.Wait();

  ASSERT_TRUE(continuationRun);
  auto continuationState = continuationTask.State();
  ASSERT_EQ(TaskState::RunToCompletion, continuationState);
}


TEST(TaskTest, IsFaultedWhenTaskThrowsExceptionThenReturnsTrue)
{
  auto task = TaskFactory::Run([]
  {
    throw invalid_argument{"bad arg"};
  });

  try
  {
    task.Wait();
  }
  catch(const invalid_argument&)
  {
    
  }

  auto isFaulted = task.IsFaulted();
  ASSERT_TRUE(isFaulted);
  auto taskState = task.State();
  ASSERT_EQ(TaskState::Faulted, taskState);
}

TEST(TaskTest, WaitWhenTaskThrowsExceptionThenRethrowsException)
{
  auto task = TaskFactory::Run([]
  {
    throw invalid_argument{"bad arg"};
  });

  ASSERT_THROW(task.Wait(), invalid_argument);
  
}

TEST(TaskTest, WaitWhenTaskCanceledThenThrowsOperationCanceledException)
{
  auto cts = CancellationTokenSource::Create();
  cts->Cancel();
  auto task = TaskFactory::Run([]
  {
    throw invalid_argument{"bad arg"};
  }, cts->Token());

  ASSERT_THROW(task.Wait(), OperationCanceledException);
  
}

TEST(TaskTest, IsCanceledWhenTaskCanceledThenReturnsTrue)
{
  auto cts = CancellationTokenSource::Create();
  cts->Cancel();
  auto task = TaskFactory::Run([]
  {
    throw invalid_argument{"bad arg"};
  }, cts->Token());

  auto isCanceled= task.IsCanceled();
  ASSERT_TRUE(isCanceled);
  auto taskState = task.State();
  ASSERT_EQ(TaskState::Canceled, taskState);
  
}

TEST(TaskTest, ContinueWithWhenAntecedentTaskAlreadyCompletedThenContinuationSeesExpectedException)
{

  auto task = TaskFactory::Run([]{throw invalid_argument{"invalid arg in test"};});

  auto continuationTask = task.ContinueWith([](const auto& task)
  {
     task.Wait();
  });

  ASSERT_THROW(continuationTask.Wait(), invalid_argument);
}


TEST(TaskTest, ResultWhenTaskTCompletedThenReturnExpectedValue)
{
  const int EXPECTED_VALUE = 42;

  auto task = TaskFactory::Run([EXPECTED_VALUE](){return EXPECTED_VALUE;});
  auto result = task.Result();

  
  ASSERT_EQ(EXPECTED_VALUE, result);
}

TEST(TaskTest, ResultWhenTaskTCompletedThenContinuationSeesExpectedValue)
{
  const int EXPECTED_VALUE = 42;

  auto continuationTask = TaskFactory::Run([EXPECTED_VALUE](){return EXPECTED_VALUE;})
              .ContinueWith([](const auto& previous){return previous.Result();});

  auto result = continuationTask.Result();

  
  ASSERT_EQ(EXPECTED_VALUE, result);
}
