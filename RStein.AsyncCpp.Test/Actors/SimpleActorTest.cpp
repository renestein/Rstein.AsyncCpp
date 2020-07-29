#include "../../RStein.AsyncCpp/Actors/SimpleActor.h"
#include "../../RStein.AsyncCpp/Tasks/Task.h"
#include "../../RStein.AsyncCpp/Tasks/TaskCompletionSource.h"


#include <gtest/gtest.h>
#include <string>

using namespace RStein::AsyncCpp::Actors;
using namespace RStein::AsyncCpp::Tasks;
using namespace std;
namespace RStein::AsyncCpp::SimpleActorTest
{
  TEST(SimpleActorTest, WhenUsingSyncStatelessActorThenAllMessagesAreProcessed)
  {
    const int EXPECTED_MESSAGES = 101;
    auto seenMessages = 0;
    {
      auto stateLessActor = CreateSimpleActor<int>([&seenMessages](const int& message) {seenMessages++; });
      for (int i = 0; i < EXPECTED_MESSAGES; i++)
      {
        stateLessActor->Tell(i);
      }
    }
    ASSERT_EQ(EXPECTED_MESSAGES, seenMessages);
  }


  TEST(SimpleActorTest, WhenUsingAsyncStatelessActorThenAllMessagesAreProcessed)
  {
    const int EXPECTED_MESSAGES = 101;
    auto seenMessages = 0;
    {
      auto stateLessActor = CreateAsyncSimpleActor<int>([&seenMessages](const int& message)->Task<void>
        {
          co_await GetCompletedTask().ConfigureAwait(false);
          seenMessages++;
        });

      for (int i = 0; i < EXPECTED_MESSAGES; i++)
      {
        stateLessActor->Tell(i);
      }
    }
    ASSERT_EQ(EXPECTED_MESSAGES, seenMessages);
  }

  TEST(SimpleActorTest, WhenUsingSyncStatefulActorThenAllMessagesAreProcessed)
  {
    const int EXPECTED_MESSAGES = 101;
    auto seenMessages = 0;
    {
      auto stateFullActor = CreateSimpleActor<int, int>([&seenMessages](const int& state, const int& message)
        {
          seenMessages++;
          return state;
        }, 0);

      for (int i = 0; i < EXPECTED_MESSAGES; i++)
      {
        stateFullActor->Tell(i);
      }
    }
    ASSERT_EQ(EXPECTED_MESSAGES, seenMessages);
  }


  TEST(SimpleActorTest, WhenUsingASyncStatefulActorThenAllMessagesAreProcessed)
  {
    const int EXPECTED_MESSAGES = 101;
    auto seenMessages = 0;
    {
      auto stateFullActor = CreateAsyncSimpleActor<int, int>([&seenMessages](const int& state, const int& message)->Task<int>
        {
          seenMessages++;
          co_await GetCompletedTask().ConfigureAwait(false);
          co_return state;
        }, 0);

      for (int i = 0; i < EXPECTED_MESSAGES; i++)
      {
        stateFullActor->Tell(i);
      }
    }
    ASSERT_EQ(EXPECTED_MESSAGES, seenMessages);
  }


  TEST(SimpleActorTest, WhenUsingSyncStatefulActorThenHasExpectedState)
  {
    const int MESSAGES_COUNT = 101;
    const int EXPECTED_STATE = MESSAGES_COUNT;
    auto seenMessages = 0;
    auto testState = 0;
    {
      auto stateFullActor = CreateSimpleActor<int, int>([&seenMessages, &testState](const int& state, const int& message)
        {
          seenMessages++;
          auto newState = state + 1;
          testState = newState;
          return newState;
        }, testState);

      for (int i = 0; i < MESSAGES_COUNT; i++)
      {
        stateFullActor->Tell(i);
      }
    }

    ASSERT_EQ(EXPECTED_STATE, testState);
  }


  TEST(SimpleActorTest, WhenUsingAsyncStatefulActorThenHasExpectedState)
  {
    const int MESSAGES_COUNT = 101;
    const int EXPECTED_STATE = MESSAGES_COUNT;
    auto seenMessages = 0;
    auto testState = 0;
    {
      auto stateFullActor = RStein::AsyncCpp::Actors::CreateAsyncSimpleActor<int, int>([&seenMessages, &testState](const int& state, const int& message)->Task<int>
        {
          seenMessages++;
          co_await GetCompletedTask().ConfigureAwait(false);
          auto newState = state + 1;
          testState = newState;
          co_return newState;
        }, testState);

      for (int i = 0; i < MESSAGES_COUNT; i++)
      {
        stateFullActor->Tell(i);
      }
    }

    ASSERT_EQ(EXPECTED_STATE, testState);
  }

  TEST(SimpleActorTest, PingPongTest)
  {
    const int PINGS_COUNT = 5;
    std::unique_ptr<IActor<string>> sigerus;
    std::unique_ptr<IActor<string>> thomasAquinas;
    auto logger = CreateSimpleActor<string>([](const string& message)
    {
      cout << message << endl;
    });

    thomasAquinas = CreateSimpleActor<string, int>([PINGS_COUNT, &sigerus, &logger](const int& pingsSent, const string& message)
      {
        if (message == "start" || message.starts_with("pong"))
        {
          logger->Tell(message);
          auto newState = pingsSent + 1;

          sigerus->Tell("ping " + to_string(newState));
          return newState;
        }
        cout << message << endl;
        return pingsSent;
      }, 0)
    ;
    sigerus = CreateSimpleActor<string, int>([PINGS_COUNT, &thomasAquinas, &logger](const int& pongsSent, const string& message)
      {

        if (message.starts_with("ping"))
        {
          logger->Tell(message);
          auto newState = pongsSent + 1;

          thomasAquinas->Tell(newState < 5
            ? "pong " + to_string(newState)
            : "stop");
          //missing Task.Delay
          this_thread::sleep_for(500ms);
          return newState;
        }
        return pongsSent;
      }, 0);

    thomasAquinas->Tell("start");
    this_thread::sleep_for(5s);
  }

}