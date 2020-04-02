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

  auto task = Task::Run([&taskRun]{taskRun = true;});
  task.Wait();

  ASSERT_TRUE(taskRun);
  auto taskState = task.State();
  ASSERT_EQ(TaskState::RunToCompletion, taskState);
}

TEST(TaskTest, ContinueWithWhenAntecedentTaskCompletedThenContinuationRun)
{

  std::promise<void> startTastPromise;

  bool continuationRun = false;

  auto task = Task::Run([future=startTastPromise.get_future().share()]{future.wait();});

  auto continuationTask = task.ContinueWith([&continuationRun](const Task& task)
  {
    continuationRun = true;  
  });

  startTastPromise.set_value();
  continuationTask.Wait();

  ASSERT_TRUE(continuationRun);
  auto continuationState = continuationTask.State();
  ASSERT_EQ(TaskState::RunToCompletion, continuationState);
}

TEST(TaskTest, ContinueWithWhenAntecedentTaskAlreadyCompletedThenContinuationRun)
{

  bool continuationRun = false;

  auto task = Task::Run([]{});

  auto continuationTask = task.ContinueWith([&continuationRun](const Task& task)
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
  auto task = Task::Run([]
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
  auto task = Task::Run([]
  {
    throw invalid_argument{"bad arg"};
  });

  ASSERT_THROW(task.Wait(), invalid_argument);
  
}

TEST(TaskTest, WaitWhenTaskCanceledThenThrowsOperationCanceledException)
{
  auto cts = CancellationTokenSource::Create();
  cts->Cancel();
  auto task = Task::Run([]
  {
    throw invalid_argument{"bad arg"};
  }, cts->Token());

  ASSERT_THROW(task.Wait(), OperationCanceledException);
  
}

TEST(TaskTest, IsCanceledWhenTaskCanceledThenReturnsTrue)
{
  auto cts = CancellationTokenSource::Create();
  cts->Cancel();
  auto task = Task::Run([]
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

  auto task = Task::Run([]{throw invalid_argument{"invalid arg in test"};});

  auto continuationTask = task.ContinueWith([](const Task& task)
  {
     task.Wait();
  });

  ASSERT_THROW(continuationTask.Wait(), invalid_argument);
}

