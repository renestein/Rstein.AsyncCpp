#include "../../RStein.AsyncCpp/Threading/SynchronizationContext.h"
#include "../../RStein.AsyncCpp/Threading/SynchronizationContextScope.h"
#include "../Mocks/SynchronizationContextMock.h"



#include <gtest/gtest.h>
using namespace RStein::AsyncCpp::Mocks;

namespace RStein::AsyncCpp::Threading::Test
{  
  
  TEST(SynchronizationContextScope, CtorWhenCalledThenNewContextIsUsed)
  {
    TestSynchronizationContextMock synchronizationContextMock;
    SynchronizationContextScope syncScope(synchronizationContextMock);

    SynchronizationContext::Current()->Post([]{});
    SynchronizationContext::Current()->Send([]{});

    ASSERT_TRUE(synchronizationContextMock.WasPostCalled());
    ASSERT_TRUE(synchronizationContextMock.WaSendCalled());
  }

   
  TEST(SynchronizationContextScope, DtorWhenCalledThenOldContextIsRestored)
  {

    auto oldContext = SynchronizationContext::Current();
    TestSynchronizationContextMock synchronizationContextMock;     
    //synchronizationContextMock is the SynchronizationContext::Current()
    {
      SynchronizationContextScope syncScope(synchronizationContextMock);
      SynchronizationContext::Current()->Post([]{});
      SynchronizationContext::Current()->Send([]{});
    } //synchronizationContextMock is removed and previous context is restored

    auto restoredContext = SynchronizationContext::Current();
    
    ASSERT_EQ(oldContext, restoredContext);

  }

  TEST(SynchronizationContextScope, CtorWhenNestedScopeThenNestedContextIsUsed)
  {

    TestSynchronizationContextMock synchronizationContextMock;
    TestSynchronizationContextMock nestedSynchronizationContextMock;

    //synchronizationContextMock is the SynchronizationContext::Current()
    {
      //nestedSynchronizationContextMock is the SynchronizationContext::Current()
      SynchronizationContextScope syncScope(synchronizationContextMock);
      {
        SynchronizationContextScope nestedSyncScope(nestedSynchronizationContextMock);
        SynchronizationContext::Current()->Post([]{});
        SynchronizationContext::Current()->Send([]{});
      }//nestedSynchronizationContextMock is removed and synchronizationContextMock is restored.

    } //synchronizationContextMock is removed and previous context is restored
    
    
    ASSERT_TRUE(nestedSynchronizationContextMock.WasPostCalled());
    ASSERT_TRUE(nestedSynchronizationContextMock.WaSendCalled());
    ASSERT_FALSE(synchronizationContextMock.WaSendCalled());
    ASSERT_FALSE(synchronizationContextMock.WasPostCalled());

  }
}