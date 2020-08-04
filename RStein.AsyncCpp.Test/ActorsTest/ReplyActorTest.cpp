#include "../../RStein.AsyncCpp/Actors/ReplyActor.h"

#include <gtest/gtest.h>
using namespace RStein::AsyncCpp::Actors;
using namespace RStein::AsyncCpp::Tasks;
using namespace std;

namespace RStein::AsyncCpp::ActorsTest
{
  TEST(ReplyActorTest, AskWhenUsingSyncStalessActorThenReturnsExpectedResponse)
  {
    const int MESSAGES_COUNT = 99;
    auto syncStatelessReplyActor = CreateReplyActor<int, string>([](const int& message)
    {
      return to_string(message);
    });

    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
      auto replyTask = syncStatelessReplyActor->Ask(i);
      auto response= replyTask.Result();

      ASSERT_EQ(to_string(i), response);
    }
  }

  TEST(ReplyActorTest, CompleteWhenUsingSyncStalessActorThenAllMessagesAreProcessed)
  {
    const int MESSAGES_COUNT = 99;
    auto syncStatelessReplyActor = CreateReplyActor<int, string>([](const int& message)
    {
      return to_string(message);
    });

    auto replyTask = Task<string>::InvalidPlaceholderTaskCreator()();
    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
       replyTask = syncStatelessReplyActor->Ask(i);
    }

      syncStatelessReplyActor->Complete();
      syncStatelessReplyActor->Completion().Wait();

      ASSERT_TRUE(replyTask.IsCompleted());
   }

  TEST(ReplyActorTest, CompleteWhenUsingAsyncStalessActorThenAllMessagesAreProcessed)
  {
    const int MESSAGES_COUNT = 99;
    auto syncStatelessReplyActor = CreateAsyncReplyActor<int, string>([](const int& message)->Task<string>
    {
      co_await GetCompletedTask().ConfigureAwait(false);
      co_return to_string(message);
    });

    auto replyTask = Task<string>::InvalidPlaceholderTaskCreator()();
    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
       replyTask = syncStatelessReplyActor->Ask(i);
    }

      syncStatelessReplyActor->Complete();
      syncStatelessReplyActor->Completion().Wait();

      ASSERT_TRUE(replyTask.IsCompleted());
   }

  TEST(ReplyActorTest, CompleteWhenUsingSyncStatefulActorThenAllMessagesAreProcessed)
  {
    const int MESSAGES_COUNT = 99;
    auto syncStatelessReplyActor = CreateReplyActor<int, string>([](const int& message, const int& state)
    {
        return std::pair{to_string(message), state};
    }, 0);

    auto replyTask = Task<string>::InvalidPlaceholderTaskCreator()();
    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
       replyTask = syncStatelessReplyActor->Ask(i);
    }

      syncStatelessReplyActor->Complete();
      syncStatelessReplyActor->Completion().Wait();

      ASSERT_TRUE(replyTask.IsCompleted());
   }

   TEST(ReplyActorTest, CompleteWhenUsingAsyncStafulActorThenAllMessagesAreProcessed)
  {
    const int MESSAGES_COUNT = 99;
    auto syncStatelessReplyActor = CreateAsyncReplyActor<int, string>([](const int& message, const int& state)->Task<pair<string, int>>
    {
      co_await GetCompletedTask().ConfigureAwait(false);
      co_return pair{to_string(message), 0};
    }, 0);

    auto replyTask = Task<string>::InvalidPlaceholderTaskCreator()();
    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
       replyTask = syncStatelessReplyActor->Ask(i);
    }

      syncStatelessReplyActor->Complete();
      syncStatelessReplyActor->Completion().Wait();

      ASSERT_TRUE(replyTask.IsCompleted());
   }
  TEST(ReplyActorTest, AskWhenUsingAsyncStalessActorThenReturnsExpectedResponse)
  {
    const int MESSAGES_COUNT = 99;
    auto asyncStatelessReplyActor = CreateAsyncReplyActor<int, string>([](const int& message)-> Task<string>
    {
      co_await GetCompletedTask().ConfigureAwait(false);
      co_return to_string(message);
    });

    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
      auto replyTask = asyncStatelessReplyActor->Ask(i);
      auto response= replyTask.Result();

      ASSERT_EQ(to_string(i), response);
    }
  }

  
  TEST(ReplyActorTest, AskWhenUsingSyncStatefulActorThenReturnsExpectedResponse)
  {
    const int MESSAGES_COUNT = 99;
    auto syncStatefulReplyActor = CreateReplyActor<int, string, int>([](const int& message, const int& state)
    {
      return make_pair(to_string(message), 0);
    }, 0);

    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
      auto replyTask = syncStatefulReplyActor->Ask(i);
      auto response= replyTask.Result();

      ASSERT_EQ(to_string(i), response);
    }
  }
  
  TEST(ReplyActorTest, AskWhenUsingAsyncStatefulActorThenReturnsExpectedResponse)
  {
    const int MESSAGES_COUNT = 99;
    auto asyncStatefulReplyActor = CreateAsyncReplyActor<int, string, int>([](const int& message, const int& state)->Task<pair<string, int>>
    {
      co_await GetCompletedTask().ConfigureAwait(false);
      co_return make_pair(to_string(message), 0);
    }, 0);

    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
      auto replyTask = asyncStatefulReplyActor->Ask(i);
      auto response= replyTask.Result();

      ASSERT_EQ(to_string(i), response);
    }
  }
     
  TEST(ReplyActorTest, AskWhenUsingSyncStatefulActorThenHasExpectedState)
  {
    const int MESSAGES_COUNT = 99;
    const int EXPECTED_STATE = MESSAGES_COUNT + 1;
    auto currentState = -1;
    auto syncStatefulReplyActor = CreateReplyActor<int, string, int>([&currentState](const int& message, const int& state)
    {
      auto newState = state + 1;
      currentState = newState;
      return make_pair(to_string(message), newState);
    }, 1);

    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
      auto replyTask = syncStatefulReplyActor->Ask(i);
      if (i == MESSAGES_COUNT -1)
      {
        replyTask.Wait();
      }
    }

    ASSERT_EQ(EXPECTED_STATE, currentState);
  }

  TEST(ReplyActorTest, AskWhenUsingAsyncStatefulActorThenHasExpectedState)
  {
    const int MESSAGES_COUNT = 99;
    const int EXPECTED_STATE = MESSAGES_COUNT + 1;
    auto currentState = -1;
    auto syncStatefulReplyActor = CreateAsyncReplyActor<int, string, int>([&currentState](const int& message, const int& state)->Task<pair<string, int>>
    {
      auto newState = state + 1;
      currentState = newState;

      co_await GetCompletedTask().ConfigureAwait(false);
      co_return make_pair(to_string(message), newState);
    }, 1);

    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
      auto replyTask = syncStatefulReplyActor->Ask(i);
      if (i == MESSAGES_COUNT -1)
      {
        replyTask.Wait();
      }
    }

    ASSERT_EQ(EXPECTED_STATE, currentState);
  }
}
