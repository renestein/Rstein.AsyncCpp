#include "../../RStein.AsyncCpp/Schedulers/Scheduler.h"
#include "../../RStein.AsyncCpp/Threading/SynchronizationContext.h"
#include "../../RStein.AsyncCpp/Threading/SynchronizationContextScope.h"
#include "../Mocks/SynchronizationContextMock.h"

#include <gtest/gtest.h>
namespace RStein::AsyncCpp::SchedulersTest
{
  using UiSynchronizationContext = Mocks::TestSynchronizationContextMock;

  TEST(Scheduler, FromSynchronizationContextReturnsSchedulerBasedOnSynchronizationContext)
  {
    //Assume that this is a special (non-default) synchronization context  - UI context, event loop, dedicated service thread etc.
    UiSynchronizationContext uicontext;

    Threading::SynchronizationContextScope synchronizationContextScope(uicontext);

    //UI framework/Specialized service installs special context.

    Threading::SynchronizationContext::SetSynchronizationContext(&uicontext);
    auto myCompletionHandler = []
    {
      //UI controls like textbox expects access from the UI thread.
      //textbox.Text = GetAsyncResult();
      
    };

    //And call infrastructure code (e. g. async processor)
    //asyncProcessor.Run(heavyWorkForBackgroundThread, myCompletionHandler);
   
    //Another part of the application, typically infrastructure code (e. g. async processor) captures synchronization context for calling thread and wraps it in scheduler.  

    auto callingThreadScheduler = Schedulers::Scheduler::FromCurrentSynchronizationContext();
    auto clientHandler = myCompletionHandler;
    //Async processor executes async work, possibly in another thread (task, scheduler).

    //[...time passed...]

    //Async processor completes work in *another (non-UI/non-special service) thread* and then invokes clientHandler in the UI synchronization context.
    callingThreadScheduler->EnqueueItem(clientHandler);

    ASSERT_TRUE(uicontext.WasPostCalled());
  }
}
