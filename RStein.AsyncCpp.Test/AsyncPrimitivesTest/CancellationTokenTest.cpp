#include "../../RStein.AsyncCpp/AsyncPrimitives/CancellationTokenSource.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/OperationCanceledException.h"


#include <gtest/gtest.h>


using namespace testing;
using namespace RStein::AsyncCpp::AsyncPrimitives;

namespace RStein::AsyncCpp::AsyncPrimitivesTest
{
  TEST(CancellationTokenTest, ThrowIfCancellationRequestedWhenNotCanceledThenDoesNotThrow)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();

    ASSERT_NO_THROW(token.ThrowIfCancellationRequested());
  }

  TEST(CancellationTokenTest, IsCancellationRequestedWhenNotCanceledThenReturnsFalse)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    auto cancellationRequested = token.IsCancellationRequested();
    ASSERT_FALSE(cancellationRequested);
  }

  TEST(CancellationTokenTest, IsCancellationRequestedWhenCanceledThenReturnsTrue)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();

    cts.Cancel();

    auto cancellationRequested = token.IsCancellationRequested();
    ASSERT_TRUE(cancellationRequested);
  }

  TEST(CancellationTokenTest, ThrowIfCancellationRequestedWhenCanceledThenThrowOperatinCanceledException)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();

    cts.Cancel();

    ASSERT_THROW(token.ThrowIfCancellationRequested(), RStein::AsyncCpp::AsyncPrimitives::OperationCanceledException);
  }

  TEST(CancellationTokenTest, NoneWhenUsedIsCancellationRequestedReturnsFalse)
  {
    auto noneToken = CancellationToken::None();

    auto isCanceled = noneToken.IsCancellationRequested();

    ASSERT_FALSE(isCanceled);
  }

  TEST(CancellationTokenTest, CanBeCanceledWhenNoneTokenThenReturnsFalse)
  {
    auto noneToken = CancellationToken::None();

    auto canBeCanceled = noneToken.CanBeCanceled();

    ASSERT_FALSE(canBeCanceled);
  }

  TEST(CancellationTokenTest, CanBeCanceledWhenStandardTokenThenReturnsTrue)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();

    auto canBeCanceled = token.CanBeCanceled();

    ASSERT_TRUE(canBeCanceled);
  }

  TEST(CancellationTokenTest, RegisterWhenTokenCanceledLaterThenCancellationActionIsCalled)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    bool wasCancelActionCalled = false;

    auto _ = token.Register([&wasCancelActionCalled]
      {
        wasCancelActionCalled = true;
      });
    cts.Cancel();

    ASSERT_TRUE(wasCancelActionCalled);
  }


  
  TEST(CancellationTokenTest, RegisterWhenTokenNeverCanceledThenCancellationActionIsNotCalled)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    bool wasCancelActionCalled = false;

    auto _ = token.Register([&wasCancelActionCalled]
      {
        wasCancelActionCalled = true;
      });

    
    ASSERT_FALSE(wasCancelActionCalled);
  }

  TEST(CancellationTokenTest, RegisterWhenTokenCanceledAndCancellationActionThrowsExceptionThenDoesNotThrow)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    bool wasCancelActionCalled = false;

    auto _ = token.Register([&wasCancelActionCalled]
      {
        wasCancelActionCalled = true;
        throw std::bad_alloc();
      });

    ASSERT_NO_THROW(cts.Cancel());
    ASSERT_TRUE(wasCancelActionCalled);
  }

  TEST(CancellationTokenTest, RegisterWhenTokenCanceledAndOneCanceledActionThenAllActionsAreCalled)
  {
    const int CANCEL_ACTION_COUNT = 3;
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    int cancelActionCalls = 0;

    auto t1 = token.Register([&cancelActionCalls]
      {
        cancelActionCalls++;
      });

    auto t2 = token.Register([&cancelActionCalls]
      {
        cancelActionCalls++;
        throw std::bad_alloc();
      });

    auto t3 = token.Register([&cancelActionCalls]
      {
        cancelActionCalls++;
      });


    ASSERT_NO_THROW(cts.Cancel());
    ASSERT_EQ(CANCEL_ACTION_COUNT, cancelActionCalls);
  }

  
  TEST(CancellationTokenTest, RegisterWhenTokenCanceledAndRegisteredMoreActionsThenAllActionsAreCalled)
  {
    const int CANCEL_ACTION_COUNT = 3;
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    int cancelActionCalls = 0;

    auto cancellationAction = [&cancelActionCalls]
    {
      cancelActionCalls++;
    };

    auto t1 = token.Register(cancellationAction);
    auto t2 = token.Register(cancellationAction);
    auto t3 = token.Register(cancellationAction);

    cts.Cancel();

    ASSERT_EQ(CANCEL_ACTION_COUNT, cancelActionCalls);
  }

  
  TEST(CancellationTokenTest, RegisterWhenTokenCanceledAndRegisteredMoreActionsAndOneActionUnregisteredThenCallsRemainingActions)
  {
    const int CANCEL_ACTION_COUNT = 2;
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    int cancelActionCalls = 0;

    auto cancellationAction = [&cancelActionCalls]
    {
      cancelActionCalls++;
    };

    auto t1 = token.Register(cancellationAction);
    auto t2 = token.Register([&cancelActionCalls]{--cancelActionCalls;});
    auto t3 = token.Register(cancellationAction);

    auto movedT2 = std::move(t2);
    movedT2.Dispose();

    t2.Dispose();

    cts.Cancel();

    ASSERT_EQ(CANCEL_ACTION_COUNT, cancelActionCalls);
  }



  TEST(CancellationTokenTest, RegisterWhenTokenAlreadyCanceledThenCancellationActionIsCalled)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    bool wasCancelActionCalled = false;

    cts.Cancel();
    auto _ = token.Register([&wasCancelActionCalled]
      {
        wasCancelActionCalled = true;
      });

    ASSERT_TRUE(wasCancelActionCalled);
  }


  TEST(CancellationTokenTest, RegisterWhenTokenCanceledAfterDisposeOfTheRegistrationThenCancellationActionIsNotCalled)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    bool wasCancelActionCalled = false;

    auto registration = token.Register([&wasCancelActionCalled]
      {
        wasCancelActionCalled = true;
      });
    registration.Dispose();
    cts.Cancel();

    ASSERT_FALSE(wasCancelActionCalled);
  }

  TEST(CancellationTokenTest, RegisterWhenTokenCanceledAfterMultipleDisposeOfTheRegistrationThenCancellationActionIsNotCalled)
  {
    auto cts = CancellationTokenSource{};
    auto token = cts.Token();
    bool wasCancelActionCalled = false;

    auto registration = token.Register([&wasCancelActionCalled]
      {
        wasCancelActionCalled = true;
      });

    registration.Dispose();
    registration.Dispose();
    cts.Cancel();
    registration.Dispose();
    ASSERT_FALSE(wasCancelActionCalled);
  }
}
