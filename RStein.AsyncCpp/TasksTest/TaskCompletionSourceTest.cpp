
#include "../Tasks/TaskCompletionSource.h"
#include <exception>
#include <gtest/gtest.h>
using namespace testing;
using namespace std;
using namespace RStein::AsyncCpp::Tasks;

TEST(TaskCompletionSourceTest, TaskWhenNewInstanceThenReturnsTaskInCreatedState)
{
  TaskCompletionSource<int> tcs{};

  auto tcsTask = tcs.GetTask();
  auto taskState = tcsTask.State();
  ASSERT_EQ(taskState, TaskState::Created);
  //prevent assert in Task shared state dtor
  tcs.SetResult(0);
}

TEST(TaskCompletionSourceTest, TrySetCanceledWhenCalledThenTaskStateIsCanceled)
{
  TaskCompletionSource<int> tcs{};
  tcs.TrySetCanceled();

  auto taskState = tcs.GetTask().State();

  ASSERT_EQ(taskState, TaskState::Canceled);

}

TEST(TaskCompletionSourceTest, SetCanceledWhenCalledThenTaskIsCanceled)
{
  TaskCompletionSource<int> tcs{};
  tcs.SetCanceled();

  auto taskState = tcs.GetTask().State();

  ASSERT_EQ(taskState, TaskState::Canceled);
}

TEST(TaskCompletionSourceTest, SetExceptionWhenCalledThenTaskStateIsFaulted)
{
  TaskCompletionSource<int> tcs{};

  tcs.SetException(std::make_exception_ptr(invalid_argument{""}));

  auto taskState = tcs.GetTask().State();
  ASSERT_EQ(taskState, TaskState::Faulted);
}

TEST(TaskCompletionSourceTest, SetExceptionWhenCalledThenTaskThrowsSameException)
{
  TaskCompletionSource<int> tcs{};

  tcs.SetException(std::make_exception_ptr(invalid_argument{""}));

  ASSERT_THROW(tcs.GetTask().Result(), invalid_argument);
}

TEST(TaskCompletionSourceTest, TrySetExceptionWhenCalledThenTaskThrowsSameException)
{
  TaskCompletionSource<int> tcs{};

  tcs.TrySetException(std::make_exception_ptr(invalid_argument{""}));

  ASSERT_THROW(tcs.GetTask().Result(), invalid_argument);
}


TEST(TaskCompletionSourceTest, TrySetResultWhenCalledThenTaskStateIsRunToCompletion)
{
  const int EXPECTED_TASK_VALUE = 1000;
  TaskCompletionSource<int> tcs{};

  tcs.TrySetResult(EXPECTED_TASK_VALUE);
  auto taskState = tcs.GetTask().State();
  ASSERT_EQ(taskState, TaskState::RunToCompletion);
}

TEST(TaskCompletionSourceTest, SetResultWhenCalledThenTaskStateIsRunToCompletion)
{
  const int EXPECTED_TASK_VALUE = 1000;
  TaskCompletionSource<int> tcs{};

  tcs.SetResult(EXPECTED_TASK_VALUE);
  auto taskState = tcs.GetTask().State();
  ASSERT_EQ(taskState, TaskState::RunToCompletion);
}


TEST(TaskCompletionSourceTest, TrySetResultVoidWhenCalledThenTaskStateIsRunToCompletion)
{
  TaskCompletionSource<void> tcs{};

  tcs.TrySetResult();
  auto taskState = tcs.GetTask().State();
  ASSERT_EQ(taskState, TaskState::RunToCompletion);
}

TEST(TaskCompletionSourceTest, SetResultVoidWhenCalledThenTaskStateIsRunToCompletion)
{
  const int EXPECTED_TASK_VALUE = 1000;
  TaskCompletionSource<void> tcs{};

  tcs.SetResult();
  auto taskState = tcs.GetTask().State();
  ASSERT_EQ(taskState, TaskState::RunToCompletion);
}


TEST(TaskCompletionSourceTest, SetResultWhenCalledThenTaskHasExpectedResult)
{
  const int EXPECTED_TASK_VALUE = 1000;
  TaskCompletionSource<int> tcs{};

  tcs.SetResult(EXPECTED_TASK_VALUE);
  auto taskResult = tcs.GetTask().Result();
  ASSERT_EQ(EXPECTED_TASK_VALUE, taskResult);
}

TEST(TaskCompletionSourceTest, TrySetResultWhenCalledThenTaskHasExpectedResult)
{
  const int EXPECTED_TASK_VALUE = -100;
  TaskCompletionSource<int> tcs{};

  tcs.TrySetResult(EXPECTED_TASK_VALUE);
  auto taskResult = tcs.GetTask().Result();
  ASSERT_EQ(EXPECTED_TASK_VALUE, taskResult);
}

TEST(TaskCompletionSourceTest, TrySetCanceledWhenTaskAlreadyCompletedThenReturnsFalse)
{
  TaskCompletionSource<int> tcs{};
  tcs.SetResult(0);

  auto trySetCanceledResult = tcs.TrySetCanceled();

  ASSERT_FALSE(trySetCanceledResult);

}

TEST(TaskCompletionSourceTest, TrySetExceptionWhenTaskAlreadyCompletedThenReturnsFalse)
{
  TaskCompletionSource<int> tcs{};
  tcs.SetResult(0);

  auto trySetExceptionResult = tcs.TrySetException(make_exception_ptr(invalid_argument{""}));

  ASSERT_FALSE(trySetExceptionResult);
}

TEST(TaskCompletionSourceTest, TrySetResultWhenTaskAlreadyCompletedThenReturnsFalse)
{
  TaskCompletionSource<int> tcs{};
  tcs.SetResult(0);

  auto trySetResult = tcs.TrySetResult(0);

  ASSERT_FALSE(trySetResult);
}


TEST(TaskCompletionSourceTest, TrySetResultVoidWhenTaskAlreadyCompletedThenReturnsFalse)
{
  TaskCompletionSource<void> tcs{};
  tcs.SetResult();

  auto trySetResult = tcs.TrySetResult();

  ASSERT_FALSE(trySetResult);
}

TEST(TaskCompletionSourceTest, SetCanceledWhenTaskAlreadyCompletedThenThrowsLogicError)
{
  TaskCompletionSource<int> tcs{};
  tcs.SetResult(0);


  ASSERT_THROW(tcs.SetCanceled(), logic_error);

}

TEST(TaskCompletionSourceTest, SetExceptionWhenTaskAlreadyCompletedThenThrowsLogicError)
{
  TaskCompletionSource<int> tcs{};
  tcs.SetResult(0);

  ASSERT_THROW(tcs.SetException(make_exception_ptr(invalid_argument{""})), logic_error);
}


TEST(TaskCompletionSourceTest, SetResultWhenTaskAlreadyCompletedThenThrowsLogicError)
{
  TaskCompletionSource<int> tcs{};
  tcs.SetResult(0);


  ASSERT_THROW(tcs.SetResult(0), logic_error);
}


TEST(TaskCompletionSourceTest, SetResultVoidWhenTaskAlreadyCompletedThenThrowsLogicError)
{
  TaskCompletionSource<void> tcs{};
  tcs.SetResult();

  ASSERT_THROW(tcs.SetResult(), logic_error);
}