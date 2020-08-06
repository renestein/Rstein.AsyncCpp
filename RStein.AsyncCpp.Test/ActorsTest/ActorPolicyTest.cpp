#include "../../RStein.AsyncCpp/Actors/ActorPolicy.h"
#include "../../RStein.AsyncCpp/Tasks/TaskCompletionSource.h"
#include "../../RStein.AsyncCpp/Tasks/TaskFactory.h"



#include <gtest/gtest.h>
namespace RStein::AsyncCpp::ActorsTest
{
  class TestActor : public Actors::ActorPolicy
  {
  public:

    TestActor(std::function<bool()> canCompleteNow = [] {return true; }, std::function<void()> completeLogic = [] {}) : _canCompleteNow(std::move(canCompleteNow)),
      _completeLogic(std::move(completeLogic))
    {

    }

    TestActor(const TestActor& other) = delete;

    TestActor(TestActor&& other) noexcept = delete;

    TestActor& operator=(const TestActor& other) = delete;

    TestActor& operator=(TestActor&& other) noexcept = delete;

    void RunVoidMethod(std::function<void()> scheduleFunction)
    {
      ScheduleFunction([scheduleFunction]
        {
          scheduleFunction();
          return Tasks::GetCompletedTask();
        });
    }

    Tasks::Task<int> ProcessInt(int number)
    {
      return ScheduleFunction([number] {
        return number;
        });
    }


    Tasks::Task<int> ProcessInt(std::function<int(int)> processIntFunc, int number)
    {
      return ScheduleFunction([number, processIntFunc] {
        return processIntFunc(number);
        });
    }
  protected:


    bool CanCompleteNow() override
    {
      return _canCompleteNow();
    }

    void OnCompleted() override
    {
      return _completeLogic();
    }

  private:
    std::function<bool()> _canCompleteNow;
    std::function<void()> _completeLogic;
  };

  TEST(ActorPolicy, ScheduleFunctionWhenVoidMethodThenTaskIsCompleted)
  {
    Tasks::TaskCompletionSource<void> waitForTaskTcs;
    std::function<void()> myFunc = [waitForTaskTcs]() mutable
    {
      waitForTaskTcs.SetResult();
    };
    TestActor actor;
    actor.RunVoidMethod(myFunc);
    waitForTaskTcs.GetTask().Wait();

    ASSERT_TRUE(waitForTaskTcs.GetTask().IsCompleted());
  }

  TEST(ActorPolicy, ScheduleFunctionWhenMethodCalledThenTaskIsCompletedWithExpectedResult)
  {
    const int EXPECTED_RESULT = 10;
    TestActor actor;
    auto intTask = actor.ProcessInt(EXPECTED_RESULT);
    intTask.Wait();

    ASSERT_EQ(EXPECTED_RESULT, intTask.Result());
  }


  TEST(ActorPolicy, ScheduleFunctionWhenCalledThenAllFunctionsAreProcessedSequentially)
  {
    const int SUM_TO = 100;
    const int EXPECTED_RESULT = SUM_TO * (SUM_TO + 1) / 2;
    TestActor actor;
    auto intTask = Tasks::Task<int>::InvalidPlaceholderTaskCreator()();
    auto sumResult = 0;

    for (int i = 0; i <= SUM_TO; ++i)
    {
      intTask = actor.ProcessInt([&sumResult](int number)
        {
          sumResult += number;
          return sumResult;
        }, i);
    }
    intTask.Wait();

    auto lastTaskResult = intTask.Result();

    ASSERT_EQ(EXPECTED_RESULT, lastTaskResult);
    ASSERT_EQ(EXPECTED_RESULT, sumResult);
  }

  TEST(ActorPolicy, CompleteWhenCompleteIsNotAllowedThenReturnsFalse)
  {
    auto completeAllowed = false;
    TestActor actor{ [&completeAllowed] {return completeAllowed; } };

    auto completeResult = actor.Complete();
    completeAllowed = true;

    ASSERT_FALSE(completeResult);
  }

  TEST(ActorPolicy, CompleteWhenCompleteAllowedThenReturnsTrue)
  {
    TestActor actor{ [] {return true; } };

    auto completeResult = actor.Complete();

    ASSERT_TRUE(completeResult);
  }

  TEST(ActorPolicy, CompleteWhenCompleteIsNotAllowedThenCompleteLogicDoesNotRun)
  {
    auto completeAllowed = false;
    auto completeLogicRun = false;
    TestActor actor{ [&completeAllowed] {return completeAllowed; }, [&completeLogicRun] {completeLogicRun = true; } };

    actor.Complete();
    completeAllowed = true;
    ASSERT_FALSE(completeLogicRun);
  }


  TEST(ActorPolicy, CompleteWhenCompleteAllowedThenCompleteLogicRun)
  {
    auto completeLogicRun = false;
    TestActor actor{ [] {return true; }, [&completeLogicRun] {completeLogicRun = true; } };

    actor.Complete();
    ASSERT_TRUE(completeLogicRun);
  }


  TEST(ActorPolicy, CompleteWhenRepeatingCompleteThenCompleteLogicRunOnlyOnce)
  {
    const int EXPECTED_ONCE_COMPLETE_LOGIC_CALL = 1;
    auto completeLogicRunCounter = 0;
    TestActor actor{ [] {return true; }, [&completeLogicRunCounter] {++completeLogicRunCounter; } };

    actor.Complete();
    actor.Complete();
    actor.Complete();

    ASSERT_EQ(EXPECTED_ONCE_COMPLETE_LOGIC_CALL, completeLogicRunCounter);
  }

  TEST(ActorPolicy, CompleteWhenActorHasUnprocessedMessagesThenCompleteRunAfterAllMessagesAreProcessed)
  {
    const int EXPECTED_PROCESSED_MESSAGES = 1000;
    Tasks::TaskCompletionSource<void> waitTcs;
    TestActor actor{};
    auto lastMessageTask = Tasks::Task<int>::InvalidPlaceholderTaskCreator()();
    for (int i = 0; i <= 1000; ++i)
    {
      lastMessageTask = actor.ProcessInt([waitForTask = waitTcs.GetTask()](auto number)
      {
        waitForTask.Wait();
        return number;
      }, i);
    }

    Tasks::TaskFactory::Run([&actor]{actor.Complete();});

    waitTcs.SetResult();
    auto processedMessages = lastMessageTask.Result();
    actor.Completion().Wait();

    ASSERT_EQ(EXPECTED_PROCESSED_MESSAGES, processedMessages);
  }

  TEST(ActorPolicy, CompletionWhenActorCreatedThenTaskIsNotCompleted)
  {
      TestActor actor{};

      ASSERT_FALSE(actor.Completion().IsCompleted());
  }

  
  TEST(ActorPolicy, CompletionWhenCompleteCalledThenTaskIsCompleted)
  {
      TestActor actor{};

      actor.Complete();

      ASSERT_TRUE(actor.Completion().IsCompleted());
  }
}