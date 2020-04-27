#include "../AsyncPrimitives/OperationCanceledException.h"
#include "../Tasks/Task.h"
#include "../Tasks/TaskFactory.h"
#include "../Tasks/TaskCompletionSource.h"


#include <gtest/gtest.h>

using namespace testing;
using namespace RStein::AsyncCpp::Tasks;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace std;

namespace RStein::AsyncCpp::TasksTest
{
  class TaskPromiseTest : public Test
  {
  public:

    Task<void> TaskPromiseVoidWhenCalledThenReturnsCompletedPromiseTaskImpl() const
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
      auto result = co_await TaskFactory::Run([expectedValue]
      {
        return expectedValue;
      });
      co_return result;
    }

    Task<string> TaskPromiseStringWhenMethodThrowsThenRethrowExceptionImpl() const
    {
      auto task = TaskFactory::Run([]
      {
        throw invalid_argument{""};
      });
      co_await task;
      co_return "";
    }
  };

  TEST_F(TaskPromiseTest, TaskPromiseVoidWhenCalledThenReturnsCompletedPromiseTask)
  {
    auto task = TaskPromiseVoidWhenCalledThenReturnsCompletedPromiseTaskImpl();
    task.Wait();
  }

  TEST_F(TaskPromiseTest, TaskPromiseStringWhenCalledThenReturnsExpectedValue)
  {
    auto expectedValue = "Hello from task promise";
    auto retPromiseValue = TaskPromiseStringWhenCalledThenReturnsExpectedValueImpl(expectedValue).Result();

    ASSERT_EQ(expectedValue, retPromiseValue);
  }

  TEST_F(TaskPromiseTest, TaskPromiseStringWhenMethodThrowsThenRethrowException)
  {
    auto retPromiseValue = TaskPromiseStringWhenMethodThrowsThenRethrowExceptionImpl();

    ASSERT_THROW(retPromiseValue.Result(), invalid_argument);
  }
}
