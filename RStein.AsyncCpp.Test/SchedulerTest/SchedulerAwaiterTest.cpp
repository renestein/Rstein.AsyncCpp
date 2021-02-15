#include "../../RStein.AsyncCpp/Schedulers/CurrentThreadScheduler.h"
#include "../../RStein.AsyncCpp/Schedulers/Scheduler.h"
#include "../../RStein.AsyncCpp/Schedulers/SimpleThreadPool.h"
#include "../../RStein.AsyncCpp/Schedulers/ThreadPoolScheduler.h"
#include <future>
#include "../../RStein.AsyncCpp/Tasks/Task.h"
#include "../../RStein.AsyncCpp/Tasks/TaskCompletionSource.h"
#include "../../RStein.AsyncCpp/AsyncPrimitives/FutureEx.h"

#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace RStein::AsyncCpp::Schedulers;

namespace RStein::AsyncCpp::SchedulersTest
{
  template<typename TSchedulerFactory>
  class SchedulerTest : public Test
  {
  public:
    TSchedulerFactory _schedulerFactory{};

    std::shared_ptr<Scheduler> CreateScheduler()
    {
      return _schedulerFactory.Create();
    }

    void logCurrentThreadId()
    {
      cout << this_thread::get_id() << endl;
    }

    Tasks::Task<bool> WhenUsingAwaiterThenOperationIsCompletedImpl(Scheduler& scheduler)
    {
      cout << "\nBefore awaiter: ";
      logCurrentThreadId();
      co_await scheduler;
      cout << "\nAfter awaiter: ";
      logCurrentThreadId();
      co_return true;
    }

    std::shared_future<bool> WhenUsingAwaiterThenRepeatingOfTheAwaitOperationWorks(Scheduler& scheduler)
    {
      cout << "\n Before awaiter: ";
      logCurrentThreadId();
      co_await scheduler;
      cout << "\n After awaiter: ";
      logCurrentThreadId();
      co_await scheduler;
      cout << "\n After awaiter 2: ";
      logCurrentThreadId();
      co_await scheduler;
      cout << "\n After awaiter 3 (return): ";
      logCurrentThreadId();
      co_return true;
    }

    ~SchedulerTest() = default;


  };

  class CurrentThreadSchedulerFactory
  {
    private:
      std::shared_ptr<CurrentThreadScheduler> _currentThreadScheduler;

    public:
      std::shared_ptr<Scheduler> Create()
      {
        if (!_currentThreadScheduler)
        {
          _currentThreadScheduler = std::make_shared<CurrentThreadScheduler>();
        }
        return _currentThreadScheduler;
      }

      ~CurrentThreadSchedulerFactory()
      {
        if (_currentThreadScheduler)
        {
          _currentThreadScheduler->Stop();
        }
      }
  };

  class ThreadPoolSchedulerFactory
  {
    private:
      SimpleThreadPool _simpleThreadPool{2};
      std::shared_ptr<ThreadPoolScheduler> _threadPoolScheduler;

    public:
      std::shared_ptr<Scheduler> Create()
      {
        if (!_threadPoolScheduler)
        {         
          _threadPoolScheduler = std::make_shared<ThreadPoolScheduler>(_simpleThreadPool);
          _threadPoolScheduler->Start();
        }
        return _threadPoolScheduler;
      }
      ~ThreadPoolSchedulerFactory()
      {
        if (_threadPoolScheduler)
        {
          _threadPoolScheduler->Stop();
         
        }
      }
  };
  using MyTypes = Types<CurrentThreadSchedulerFactory, ThreadPoolSchedulerFactory>;
  TYPED_TEST_SUITE(SchedulerTest, MyTypes);


  TYPED_TEST(SchedulerTest, WhenUsingAwaiterThenSimpleOperationIsCompleted)
  {
    auto scheduler = this->CreateScheduler();
    
    auto awaiterRun = this->WhenUsingAwaiterThenOperationIsCompletedImpl(*scheduler).Result();
    ASSERT_TRUE(awaiterRun);  
    
    

  }

  TYPED_TEST(SchedulerTest, WhenUsingAwaiterThenCanRepeatAwaitOperation)
  {
    auto scheduler = this->CreateScheduler();
    auto awaiterCompleted = this->WhenUsingAwaiterThenRepeatingOfTheAwaitOperationWorks(*scheduler).get();
    ASSERT_TRUE(awaiterCompleted);

  }
}
