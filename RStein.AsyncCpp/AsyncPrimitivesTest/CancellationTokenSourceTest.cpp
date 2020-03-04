#include "../AsyncPrimitives/CancellationTokenSource.h"

#include <gtest/gtest.h>


using namespace testing;
using namespace RStein::AsyncCpp::AsyncPrimitives;

namespace RStein::AsyncCpp::AsyncPrimitivesTest
{
  TEST(CancellationTokenSourceTest, CtorWhenCreatedThenIsCancellationRequestedReturnsFalse)
  {
    auto cts = CancellationTokenSource::Create();
    auto isCanceled = cts->IsCancellationRequested();
    ASSERT_FALSE(isCanceled);
  }

  TEST(CancellationTokenSourceTest, CancelWhenCalledThenIsCancellationRequestedReturnsTrue)
  {
    auto cts = CancellationTokenSource::Create();
    cts->Cancel();
    auto isCanceled = cts->IsCancellationRequested();
    ASSERT_TRUE(isCanceled);
  }

  TEST(CancellationTokenSourceTest, CtorWhenCreatedThenTokenIsNotNull)
  {
    auto cts = CancellationTokenSource::Create();
    auto token = cts->Token();
    
    ASSERT_TRUE(token);
  }

}
