#include "../Tasks/Task.h"
#include "../Tasks/TaskFactory.h"
#include "../Tasks/TaskCompletionSource.h"


#include <gtest/gtest.h>

using namespace testing;
using namespace RStein::AsyncCpp::Tasks;
using namespace std;


class TaskPromiseTest : public Test
{
public:

  Task<void> TaskPromiseVoidWhenCalledThenReturnsImpl() const
  {
    auto func = []
    {
      this_thread::sleep_for(100ms);
    };

    auto task = TaskFactory::Run(func);

    co_await task;
  }

  Task<string> TaskPromiseStringWhenCalledThenReturnsExpectedValueImpl(string expectedValue) const
  {
    auto result = co_await TaskFactory::Run([expectedValue] {return expectedValue; });
    co_return result;
  }
};

TEST_F(TaskPromiseTest, TaskPromiseVoidWhenCalledThenReturns)
{
  auto task = TaskPromiseVoidWhenCalledThenReturnsImpl();
  task.Wait();
}

TEST_F(TaskPromiseTest, TaskPromiseStringWhenCalledThenReturnsExpectedValue)
{
  auto expectedValue = "Hello from task promise aaaaa";
  auto retPromiseValue = TaskPromiseStringWhenCalledThenReturnsExpectedValueImpl(expectedValue).Result();

  ASSERT_EQ(expectedValue, retPromiseValue);
}