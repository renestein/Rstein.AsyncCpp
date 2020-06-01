#include "../../RStein.AsyncCpp/Threading/SynchronizationContext.h"
#include "../../RStein.AsyncCpp/Utils/FinallyBlock.h"


#include <gtest/gtest.h>

namespace RStein::AsyncCpp::Threading::Test
{
  TEST(SynchronizationContextTest, SetSynchronizationContextWhenCalledThenCurrentReturnsExpectedSynchronizationcontext)
  {
    auto oldContext = SynchronizationContext::Current();
    Utils::FinallyBlock finally{[oldContext]
    {
      auto toDeleteContext = SynchronizationContext::SetSynchronizationContext(oldContext);
      delete toDeleteContext;
    }};

    auto expectedSyncContext = new SynchronizationContext;

    SynchronizationContext::SetSynchronizationContext(expectedSyncContext);

    auto currentSynchronizationContext = SynchronizationContext::Current();
    ASSERT_EQ(expectedSyncContext, currentSynchronizationContext);
  }

  
  TEST(SynchronizationContextTest, PostWhenCalledOnDefaultContextThenFunctionIsCompleted)
  {
    
    auto wasFuncCalled = false;

    SynchronizationContext::Current()->Post([&wasFuncCalled]{wasFuncCalled = true;});

    ASSERT_TRUE(wasFuncCalled);
  }

   
  TEST(SynchronizationContextTest, SendWhenCalledOnDefaultContextThenFunctionIsCompleted)
  {    
    auto wasFuncCalled = false;

    SynchronizationContext::Current()->Send([&wasFuncCalled]{wasFuncCalled = true;});

    ASSERT_TRUE(wasFuncCalled);
  }

   TEST(SynchronizationContextTest, IsDefaultWhenCalledOnDefaultContextThenFunctionIsCompleted)
  {    

    auto defaultContext = SynchronizationContext::Current();

    ASSERT_TRUE(defaultContext->IsDefault());
  }

  TEST(SynchronizationContextTest, IsDefaultWhenCalledOnOnNonDefaultInstanceThenReturnsFalse)
  {
    auto oldContext = SynchronizationContext::Current();
    Utils::FinallyBlock finally{[oldContext]
    {
      auto toDeleteContext = SynchronizationContext::SetSynchronizationContext(oldContext);
      delete toDeleteContext;
    }};

    SynchronizationContext::SetSynchronizationContext(new SynchronizationContext);
    auto  usedContext = SynchronizationContext::Current();

    ASSERT_FALSE(usedContext->IsDefault());
  }

}