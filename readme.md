# RStein.AsyncCpp (C++ library)
[![Build Status](https://dev.azure.com/rene0884/RStein.AsyncCpp/_apis/build/status/renestein.Rstein.AsyncCpp?branchName=master)](https://dev.azure.com/rene0884/RStein.AsyncCpp/_build/latest?definitionId=2&branchName=master)
- The RStein.AsyncCpp library is a set of types that should be familiar for anyone who knows the Task Parallel Library (TPL) for .NET (C#). In addition, this library contains simple DataFlow, threadless actors (agents), functional combinators for the Task<T> class, useful async primitives (AsyncSemaphore, AsyncProducerConsumerCollection, CancellationToken, CancellationTokenSource, AsyncMutex, SynchronizationContext, SynchronizationContextScope ...).

- The library is my playground for testing coroutine support in C++.

- The library supports standard C++ 20 coroutines, legacy coroutines in the MSVC cl compiler (std::experimetal namespace, /await switch), and legacy coroutines (std::experimetal namespace in the shim header) in the clang compiler on Windows.

- The library supports compilation from Visual Studio 2019 and from the command line (compilers MSVC cl and clang). Support for other compilers is planned.
- **The library can be downloaded and built using the VCPKG package manager.** 

- [`How to build the library.`](#Build-Library)



## **Task&lt;T&gt;.** 
The Task class represents the result of the execution of the one (usually asynchronous) operation - an instance of the Task contains either return value of the operation or exception or information that task was canceled. Tasks created by the TaskFactory are 'hot'. 'Hot' in this context means that the Task Start method is called and the Task is immediately scheduled on Scheduler (~=executor). You can 'co_await' Task (preferred) or/and you can use methods from the Task public interface (ContinueWith, Wait, State, IsCompleted, IsFaulted, IsCanceled, Result, Unwrap...). The Task supports both the 'awaiter' and the 'promise' concepts.
* [`Create Task<T> using the TaskFactory (uses default scheduler = ThreadPoolScheduler).`](#TaskFactory-Run)
* [`Create Task<T> using the TaskFactory and explicit scheduler.`](#TaskFactory-Run-With-Scheduler)

* [`Unwrap nested Task (lambda-coroutine returns Task) using the TaskFactory - prevents some hard-to-debug bugs.`](#TaskFactory-Unwrap-Nested-Task)

* [`Task<T> ContinueWith method - register continuation function that will be called when the Task have completed ('future.then' type of the method).`](#Task-ContinueWith)

* [`Task<T> in a 'promise' role (return type of the C++ coroutine).`](#Task-Promise-Concept)

* [`WaitAll method for tasks - synchronously waits for the completion of the all tasks.`](#Task-WaitAll)

* [`WaitAny method for tasks - synchronously waits for the completion of the any Task. `](#Task-WaitAny)

* [`WhenAll method for tasks - returns Task that will be completed when all of the provided tasks have completed.`](#Task-WhenAll)

* [`WhenAny method for tasks - returns Task that will be completed when any of the provided tasks have completed.`](#Task-WhenAny)

* [`TaskFromResult method - creates a completed instance of the Task<T> with the specified value.`](#TaskFromResult)

* [`TaskFromResult method - creates a completed instance of the Task<T> with the specified value.`](#TaskFromResult)

* [`TaskFromException method - creates a completed instance of the Task<T> with the specified exception.`](#TaskFromException)

* [`TaskFromCanceled method - creates a completed instance of the Task<T> in the Canceled state.`](#TaskFromCanceled)

* [`ConfigureAwait method - configures if the 'co_await continuation' is resumed in the specific synchronization context. See also the SynchronizationContext class below.`](#ConfigureAwait)

* [`GlobalTaskSettings::UseOnlyConfigureAwaitFalseBehavior configuration key - set the key to true if you want to enforce the equivalent of the 'co_await someTask.ConfigureAwait(false)' for all 'co_await someTask{anything}' expressions in the application - synchronization context is then never used when resuming the 'co_await continuation'`](#GlobalTaskSettings-UseOnlyConfigureAwaitFalseBehavior)


  
  ## **TaskCompletionSource&lt;T&gt;.** 
  The TaskCompletionSource class explicitly controls the state and result of the Task that is provided to the consumer. TaskCompletionSource has a relation to the Task class similar to the relation which exists between std::future and std::promise types. This class is very useful in situations when you call a library that uses different asynchronous patterns and you would like to use only the Task class in your API. Different asynchronous patterns can be simply converted to the Task-based world using the TaskCompletionSource.
* [`TaskCompletion<T> SetResult method.`](#TaskCompletionSource-SetResult)
* [`TaskCompletion<T> TrySetResult method.`](#TaskCompletionSource-TrySetResult)
* [`TaskCompletion<T> SetException method.`](#TaskCompletionSource-SetException)
* [`TaskCompletion<T> TrySetException method.`](#TaskCompletionSource-TrySetException)
* [`TaskCompletion<T> SetCanceled method.`](#TaskCompletionSource-SetCanceled)
* [`TaskCompletion<T> TrySetCanceled method.`](#TaskCompletionSource-TrySetCanceled)

## **Functional map (Fmap) and bind (Fbind) methods (Task monad).**
The [`TaskFromResult method `](#TaskFromResult) can be used as a Unit (Return) method.

* [`Fmap (map, Select) method for Task<T>.`](#Task-Fmap)
* [`Fbind (bind, SelectMany, mapMany) method for Task<T> .`](#Task-Fbind)
* [`Fjoin (Unwrap Task<Task<T> and returns Task<T>.`](#Task-Fjoin)
* [`| (pipe) operator for Fbind, Fmap, Fjoin- simplified Task<T> composition.`](#Task-Pipe-Operator)
* [`Monadic laws (tests).`](#Task-Monadic-Laws)
 
 ## **Simple DataFlow**

* [`Flat DataFlow.`](#Flat-Dataflow)
* [`Fork-Join DataFlow.`](#Fork-Join-Dataflow)

## **Threadless actors (in-process agents)**
* [`Simple actor - functional style/synchronous logic/without state/no reply.`](#Actor-I)

* [`Simple actor - functional style/asynchronous logic/with state/no reply.`](#Actor-II)

* [`Reply actor - functional style/asynchronous logic/with state/returns reply task, which eventually contains the result of the message processing.`](#Actor-III)

* [`ActorPolicy base class - OOP style.`](#Actor-IV)
* [`Map/Reduce actors - counting word frequency in Shakespeare (sample).`](https://github.com/renestein/Rstein.AsyncCpp/tree/master/Samples/MapReduceActors)

![](https://snipboard.io/TSxCQb.jpg)




 ## **Async primitives**
 * [`AsyncSemaphore - asynchronous variant of the Semaphore synchronization primitive.`](#AsyncSemaphore)
 * [`CancellationTokensource and CancellationToken - types used for the cooperative cancellation.`](#CancellationToken)
 * [`AsyncMutex - asynchronous variant of the mutex synchronization primitive.`](#AsyncMutex)
 * [`SynchronizationContext - provides a mechanism to queue work to a specialized context. (useful for marshaling calls to UI thread, event loop, etc.)`](#SynchronizationContext)
 * [`SynchronizationContextScope - RAII class for SynchronizationContext. An instance of this class captures the current synchronization context in the constructor (now 'old' context), installs new synchronization context provided by the user, and restores 'old' synchronization context in the destructor.`](#SynchronizationContextScope)

 ## Build library


 **Build using the VCPKG package manager. (when PR with new port will be merged)**
* Install VCPKG.
```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install 
```
* Install Rstein.AsyncCpp library.
```
vcpkg install rsasynccpp rsasynccpp:x64-Windows
```
_Remark: If you need legacy await support (the /await compiler switch is used, coroutines are in the std::experimental namespace), install Rstein.AsyncCpp library using the following command._
```
vcpkg install rsasynccpp[lib-cl-win-legacy-await] rsasynccpp[lib-cl-win-legacy-await]:x64-Windows
```
* Enjoy the library.
``` C++

#include <iostream>
#include <asynccpp/Tasks/TaskFactory.h> //Include required headers

int main()
{
    //Create (hot) task using the TaskFactory.
    auto task = RStein::AsyncCpp::Tasks::TaskFactory::Run([]{std::cout << "Hello from Rstein.AsyncCpp library.";});
    std::cin.get();
}

```
_And little reminder. Do not forget to use C++ 20 standard (Preview from the Latest C+) in **your VS project**_.

  ![](https://snipboard.io/YvUDJp.jpg)_

**Build from the command line (Windows).**

_Remark: Only Rstein.AsyncCpp library will be built. Samples and tests cannot be built built from the command line (yet)._

* Clone the repository. ```git clone git@github.com:renestein/Rstein.AsyncCpp.git```.
* Run the ```<Project root>\build.bat``` file. Without options the batch file builds static library for the following platforms/configurations using the MSVC cl compiler with standard C++ coroutine support enabled:
    * x64/Debug 
    * x64/Release
    * x86/Debug
    * x86/Release
* All build artifacts are located in the &lt;project root&gt;\bin directory.
* Libraries are under ```<Project root>\bin\libs\<Platform>\<Configuration>``` directory.
  * For example ```<Project root>\bin\libs\x64\Release``` directory contains x64/Release version of the RStein.AsyncCpp.lib library.
* Header files are located in ```<Project root>\bin\libs\includes\asynccpp``` folder.

* Also is possible to build a static library:
    * With legacy await support (the /await compiler switch is used, coroutines are in the std::experimental namespace) using the MSVC cl compiler. Run ```build.bat lib_cl_win_legacy_await```. [More info on Visual C++ blog](https://devblogs.microsoft.com/cppblog/c-coroutines-in-visual-studio-2019-version-16-8/).
    This command builds ```Release_VSAWAIT``` and ```Debug_VSAWAIT``` solution configurations.
    * With legacy await (coroutines are in the std::experimental namespace) using the clang compiler on Windows. Run ```build.bat lib_clang_win_legacy_await```. This command builds ```Release_CLangWin``` and ```Debug_CLangWin``` solution configurations.
    
    Do not forget to use C++ 20 standard in _your clang project_!

    ![](https://snipboard.io/8M6CJQ.jpg)


**Build from Visual Studio 2019:**

* Clone the repository. ```git clone git@github.com:renestein/Rstein.AsyncCpp.git```.
* Open the ```<Project root>\RStein.AsyncCppLib.sln``` solution file in the Visual studio 2019. If you need tests and samples, open the ```<Project root>\RStein.AsyncCppFull.sln``` solution file instead. 

**Important**: Test project in the solution file RStein.AsyncCppFull.sln requires reference to the  [gtest/gmock](https://github.com/google/googletest) library and utilises [vcpkg package manager](https://github.com/microsoft/vcpkg) for dependencies.

* In the Visual Studio select desired configuration and platforms. For details about supported configurations see above the section 'Build from the command line (Windows)'.
* Build solution.
* All build artifacts are located in the ```<Project root>\bin``` directory.
* Libraries are under ```<Project root>\bin\libs\<Platform>\<Configuration>``` directory.
    * For example ```<Project root>\bin\libs\x64\Release``` directory contains x64/Release version of the RStein.AsyncCpp.lib library.
* Header files are located in ```<Project root>\bin\libs\includes\asynccpp``` folder.
* Compiled test projects (if they were built) are under ```<Project root>\bin\tests\<Platform>\<Configuration>\``` directory.
* Compiled samples (if they were built) are under ```<Project root>\bin\samples\<Platform>\<Configuration>\``` directory.

  ## TaskFactory Run
  Create Task<T> using the TaskFactory (uses default scheduler - ThreadPoolScheduler).
  ```c++


  //Using co_await
  auto result = co_await TaskFactory::Run([]
                {
                    //Do some work
                    int result = 42; //Calculate result.
                    return result;
                });    
  ```
  ```c++

  //Using synchronous Wait method on the Task.
  TEST_F(TaskTest, RunWhenHotTaskCreatedThenTaskIsCompleted)
  {
    bool taskRun = false;

    auto task = TaskFactory::Run([&taskRun]
    {
      taskRun = true;
    });

    task.Wait();

    ASSERT_TRUE(taskRun);
    auto taskState = task.State();
    ASSERT_EQ(TaskState::RunToCompletion, taskState);
  } 
  ```

## TaskFactory Run With Scheduler
Using co_await
```c++
 Task<Scheduler::SchedulerPtr> RunWhenUsingExplicitSchedulerAndCoAwaitThenExplicitSchedulerRunTaskFuncImpl(Scheduler::SchedulerPtr taskScheduler)
    {

      auto task = TaskFactory::Run([]
                  {
                    //capture used scheduler
                    return Scheduler::CurrentScheduler();
                    
                  },
                  //run on explicit scheduler
                 taskScheduler);

      co_return co_await task;
    }

TEST_F(TaskTest, RunWhenUsingExplicitSchedulerAndCoAwaitThenExplicitSchedulerRunTaskFunc)
  {
    SimpleThreadPool threadPool{1};
    auto explicitTaskScheduler{make_shared<ThreadPoolScheduler>(threadPool)};
    explicitTaskScheduler->Start();
    auto usedScheduler = RunWhenUsingExplicitSchedulerAndCoAwaitThenExplicitSchedulerRunTaskFuncImpl(explicitTaskScheduler)
                                          .Result();
    explicitTaskScheduler->Stop();

    ASSERT_EQ(explicitTaskScheduler.get(), usedScheduler.get());
  }
```
Using Task methods.
```c++
TEST_F(TaskTest, RunWhenUsingExplicitSchedulerThenExplicitSchedulerRunTaskFunc)
  {
    SimpleThreadPool threadPool{1};
    auto explicitTaskScheduler{make_shared<ThreadPoolScheduler>(threadPool)};
    explicitTaskScheduler->Start();
    Scheduler::SchedulerPtr taskScheduler{};

    auto task = TaskFactory::Run([&taskScheduler]
    {
      taskScheduler = Scheduler::CurrentScheduler();
    }, explicitTaskScheduler);

    task.Wait();
    explicitTaskScheduler->Stop();
    ASSERT_EQ(taskScheduler.get(), explicitTaskScheduler.get());
  }

```
## TaskFactory Unwrap Nested Task
```c++
  TEST_F(TaskTest, RunWhenReturnValueIsNestedTaskThenTaskIsUnwrapped)
  {
    
     int EXPECTED_VALUE  = 10;
    //TaskFactory detects that return value of the Run would be Task<Task<int>>
    //and unwraps inner Task. Real return type is Task<int>.
    Task<int> task = TaskFactory::Run([value = EXPECTED_VALUE]()->Task<int>
    {
      co_await GetCompletedTask();
      co_return value;
    });

    auto taskValue = task.Result();
    ASSERT_EQ(EXPECTED_VALUE, taskValue);
  }
```

## Task ContinueWith
```c++
//ContinueWith method registers continuation function which will be called when the Task is completed.
TEST_F(TaskTest, ContinueWithWhenAntecedentTaskCompletedThenContinuationRun)
  {
    std::promise<void> startTaskPromise;

    bool continuationRun = false;

    auto task = TaskFactory::Run([future=startTaskPromise.get_future().share()]
    {
      future.wait();
    });

    //Register continuation
    //Continuation receives instance of the (previous) completed Task (argument task)
    auto continuationTask = task.ContinueWith([&continuationRun](const auto& task)
    {
      continuationRun = true;
    });

    startTaskPromise.set_value();

    //or co_await continuation Task
    continuationTask.Wait();

    ASSERT_TRUE(continuationRun);
    auto continuationState = continuationTask.State();
    ASSERT_EQ(TaskState::RunToCompletion, continuationState);
  }
```
```c++
//Continuation function can be called on an explicitly specified Scheduler.

  TEST_F(TaskTest, ContinueWithWhenUsingExplicitSchedulerThenContinuationRunOnExplicitScheduler)
  {
    auto task = TaskFactory::Run([]
    {
      return 10;
    });
    auto capturedContinuationScheduler = Scheduler::SchedulerPtr{};
    SimpleThreadPool threadPool{1};
    auto continuationScheduler = make_shared<ThreadPoolScheduler>(threadPool);
    continuationScheduler->Start();

    task.ContinueWith([&capturedContinuationScheduler](auto _)
        {
          capturedContinuationScheduler = Scheduler::CurrentScheduler();
        }, continuationScheduler)
        .Wait();

    ASSERT_EQ(continuationScheduler.get(), capturedContinuationScheduler.get());

    continuationScheduler->Stop();
  }
```
## Task Promise Concept
```c++
    //Task<string> is a promise (return value of the async function).
    Task<string> TaskPromiseStringWhenCalledThenReturnsExpectedValueImpl(string expectedValue) const
    {
      auto result = co_await TaskFactory::Run([expectedValue]
      {
        return expectedValue;
      });
      co_return result;
    }

    TEST_F(TaskPromiseTest, TaskPromiseStringWhenCalledThenReturnsExpectedValue)
  {
    auto expectedValue = "Hello from task promise";
    auto retPromiseValue = TaskPromiseStringWhenCalledThenReturnsExpectedValueImpl(expectedValue).Result();

    ASSERT_EQ(expectedValue, retPromiseValue);
```
## Task WaitAll
```c++

TEST_F(TaskTest, WaitAllWhenReturnsThenAllTasksAreCompleted)
  {
    auto task1 = TaskFactory::Run([]
    {
      this_thread::sleep_for(100ms);
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(50ms);
    });
    
    WaitAll(task1, task2);
    ASSERT_TRUE(task1.State() == TaskState::RunToCompletion);
    ASSERT_TRUE(task2.State() == TaskState::RunToCompletion);
  }

```
If any task throws exception, then exception is propagated in the instance of the AggregateException (another library type - kind of 'composite' exception).
```c++

  TEST_F(TaskTest, WaitAllWhenTaskThrowsExceptionThenThrowsAggregateException)
  {
    auto task1 = TaskFactory::Run([]
    {
      this_thread::sleep_for(100ms);
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(50ms);
      //task2 throws
      throw std::invalid_argument{""};
    });

    try
    {
      WaitAll(task1, task2);
    }
    //Top level exception is the AggregateException
    catch (const AggregateException& exception)
    {
      try
      {
        ASSERT_EQ(exception.Exceptions().size(), 1);
        rethrow_exception(exception.FirstExceptionPtr());
      }
      //Exception thrown in the body of the task2
      catch (const invalid_argument&)
      {
        SUCCEED();
        return;
      }
    }

    FAIL();
  }
```
## Task WaitAny
```c++
TEST_F(TaskTest, WaitAnyWhenSecondTaskCompletedThenReturnsIndex1)
  {
    const int EXPECTED_TASK_INDEX = 1;
    TaskCompletionSource<void> waitFirstTaskTcs;
    auto task1 = TaskFactory::Run([waitFirstTaskTcs]
    {
      waitFirstTaskTcs.GetTask().Wait();
      return 10;
    });

    auto task2 = TaskFactory::Run([]
    {
      this_thread::sleep_for(1ms);
    });

    //WaitAny returns zero based index of the completed task
    auto taskIndex = WaitAny(task1, task2);
    waitFirstTaskTcs.TrySetResult();
    ASSERT_EQ(EXPECTED_TASK_INDEX, taskIndex);
  }
  ```
  ## Task WhenAll
  ```c++
  
    Task<bool> WhenAllWhenTaskThrowsExceptionThenAllTasksCompletedImpl()
    {
      auto task1 = TaskFactory::Run([]
      {
        this_thread::sleep_for(100ms);
        return 10;
      });

      auto task2 = TaskFactory::Run([]
      {
        this_thread::sleep_for(50ms);
        throw std::invalid_argument{""};
      });

      try
      {
        co_await WhenAll(task1, task2);
      }
      //Exception are popagated in the AggregateException
      catch (AggregateException&)
      {
      }

      co_return task1.IsCompleted() && task2.IsCompleted();
    }

    
  TEST_F(TaskTest, WhenAllWhenTaskThrowsExceptionThenAllTasksCompleted)
  {
    auto allTasksCompleted = WhenAllWhenTaskThrowsExceptionThenAllTasksCompletedImpl().Result();

    ASSERT_TRUE(allTasksCompleted);
  }

  ```
   ## Task WhenAny
  ```c++
   Task<int> WhenAnyWhenSecondTaskCompletedThenReturnsIndex1Impl()
    {
      TaskCompletionSource<void> waitFirstTaskTcs;
      auto task1 = TaskFactory::Run([waitFirstTaskTcs]
      {
        waitFirstTaskTcs.GetTask().Wait();
        return 10;
      });

      auto task2 = TaskFactory::Run([]
      {
        this_thread::sleep_for(1ms);
      });
    
      //WaitAny returns index of the completed Task wrapped in Task
      auto taskIndex = co_await WhenAny(task1, task2);
      waitFirstTaskTcs.TrySetResult();
      co_return taskIndex;
    }
    

  TEST_F(TaskTest, WhenAnyWhenSecondTaskCompletedThenReturnsIndex1)
  {
    const int EXPECTED_TASK_INDEX = 1;
    auto taskIndex = WhenAnyWhenSecondTaskCompletedThenReturnsIndex1Impl().Result();
    ASSERT_EQ(EXPECTED_TASK_INDEX, taskIndex);
  }
```
  ## TaskFromResult
  ```c++
  TEST_F(TaskTest, TaskFromResultWhenWrappingValueThenTaskContainsWrappedValue)
  {
    const int EXPECTED_TASK_VALUE = 1234;

    auto task = TaskFromResult(1234);

    auto taskValue = task.Result();
    ASSERT_EQ(EXPECTED_TASK_VALUE, taskValue);
  }
  ```
  ## TaskFromException
  ```c++
  TEST_F(TaskTest, TaskFromExceptionWhenWaitingForTaskThenThrowsExpectedException)
  {
    auto task = TaskFromException<string>(make_exception_ptr(logic_error("")));

    try
    {
      task.Wait();
    }
    catch (const logic_error&)
    {
      SUCCEED();
      return;
    }

    FAIL();
  }
```

## TaskFromCanceled
```c++

  TEST_F(TaskTest, TaskFromCanceledWhenWaitingForTaskThenThrowsOperationCanceledException)
  {
    auto task = TaskFromCanceled<string>();

    try
    {
      task.Wait();
    }
    catch (const OperationCanceledException&)
    {
      SUCCEED();
      return;
    }

    FAIL();
  }
```

## TaskCompletionSource SetResult
```c++
  TEST(TaskCompletionSourceTest, SetResultWhenCalledThenTaskHasExpectedResult)
  {
    const int EXPECTED_TASK_VALUE = 1000;
    TaskCompletionSource<int> tcs{};

    tcs.SetResult(EXPECTED_TASK_VALUE);
    auto taskResult = tcs.GetTask().Result();
    ASSERT_EQ(EXPECTED_TASK_VALUE, taskResult);
  }
```
# ConfigureAwait
```c++
//ConfigureAwait(false) ignores SynchronizationContext


  TEST_F(TaskTest,
         ConfigureAwaitWhenNonDefaultContextAndNotRunContinuationInContextThenContinuationNotRunInSynchronizationContext)
  {
    auto continuationRunOnCapturedContext =
        ConfigureAwaitWhenNonDefaultContextAndNotRunContinuationInContextThenContinuationNotRunInSynchronizationContextImpl().Result();

    ASSERT_FALSE(continuationRunOnCapturedContext);
  }


 Task<bool> ConfigureAwaitWhenNonDefaultContextAndNotRunContinuationInContextThenContinuationNotRunInSynchronizationContextImpl()
    {

      TestSynchronizationContextMock mockSyncContext;
      
      //Restore state before co_return is called. Problems with destruction of the coroutine variables? 
      {
        //Install specific synchronization context
        SynchronizationContextScope scs(mockSyncContext);
        //[...] Irrelevant code
        co_await TaskFactory::Run([]
        {
          return 42;
          //Ignore synchronization context
        }).ConfigureAwait(false);

      }
      co_return mockSyncContext.WasPostCalled();
    }
```
## GlobalTaskSettings-UseOnlyConfigureAwaitFalseBehavior

```c++

class GlobalTaskSettings
{
public:
  /// <summary>
  /// If the key has the value false (default) and SynchronizationContext.Current() returns default ('none')
  /// synchronization context when the 'co_await someTask' expression is reached,
  /// then 'co_await continuation' is resumed in the captured synchronization context.
  /// This is a good default behavior for applications that uses special synchronization context - for example synchronization context for the UI thread.
  /// It is possible to override this behavior on a per case basis using the 'co_await someTask.ConfigureAwait(false)' instead of the 'co_await someTask'.
  /// Set the key to true if you want to globally disable this behavior.
  /// In other words, setting this value to true causes that synchronization context is always ignored and resumed 
  /// 'co_await continuation' is scheduled using the scheduler returned from the Scheduler::DefaultScheduler() method.
  /// The program then behaves as if every 'co_await someTask'expression (and also 'co_await someTask.ConfigureAwait(true)'!)
  /// expression has the form 'co_await task.ConfigureAwait(false)'.
  /// </summary>
  /// <remarks>
  /// The value of the key is irrelevant for an application that does not use own synchronization context.
  /// Also, the value of the key is irrelevant for continuations registered using the Task.ContinueWith method.
  /// </remarks>
  inline static int UseOnlyConfigureAwaitFalseBehavior = false;
[...]
}

```

```c++

  TEST_F(TaskTest,
         ConfigureAwaitWhenNonDefaultContextAndNotRunContinuationInContextThenContinuationNotRunInSynchronizationContext)
  {
    auto continuationRunOnCapturedContext =
        ConfigureAwaitWhenNonDefaultContextAndNotRunContinuationInContextThenContinuationNotRunInSynchronizationContextImpl().Result();

    ASSERT_FALSE(continuationRunOnCapturedContext);
  }

  
    Task<bool> ConfigureAwaitWhenNonDefaultContextAndNotRunContinuationInContextThenContinuationNotRunInSynchronizationContextImpl()
    {
      TestSynchronizationContextMock mockSyncContext;

      //Restore state before co_return is called. Problems with destruction of the coroutine variables? 
      {
        //Specifixc 
        SynchronizationContextScope scs(mockSyncContext);

        //Restore default behavior after test
        Utils::FinallyBlock finally
        {
            []
            {
              GlobalTaskSettings::TaskAwaiterAwaitReadyAlwaysReturnsFalse = false;
            }
        };


        GlobalTaskSettings::TaskAwaiterAwaitReadyAlwaysReturnsFalse = true;

        co_await TaskFactory::Run([]
        {
          return 42;
        }).ConfigureAwait(false);
      }
      co_return mockSyncContext.WasPostCalled();
    }


```

## TaskCompletionSource SetResult

```c++

  // Trying to set already completed TaskCompletionSource throws logic_error.
  TEST(TaskCompletionSourceTest, SetResultWhenTaskAlreadyCompletedThenThrowsLogicError)
  {
    TaskCompletionSource<int> tcs{};
    tcs.SetResult(0);

    ASSERT_THROW(tcs.SetResult(0), logic_error);
  }

```
## TaskCompletionSource TrySetResult
```c++
TEST(TaskCompletionSourceTest, TrySetResultWhenCalledThenTaskHasExpectedResult)
  {
    const int EXPECTED_TASK_VALUE = -100;
    TaskCompletionSource<int> tcs{};

    tcs.TrySetResult(EXPECTED_TASK_VALUE);
    auto taskResult = tcs.GetTask().Result();
    ASSERT_EQ(EXPECTED_TASK_VALUE, taskResult);
  }
  ```
  ```c++
   TEST(TaskCompletionSourceTest, TrySetCanceledWhenTaskAlreadyCompletedThenReturnsFalse)
  {
    TaskCompletionSource<int> tcs{};
    tcs.SetResult(0);
    
    //Trying to set already completed TaskCompletionSource returns false (unlike the SetResult method that throws exception).
    auto trySetCanceledResult = tcs.TrySetCanceled();

    ASSERT_FALSE(trySetCanceledResult);
  }
  ```


## TaskCompletionSource SetException
```c++

  TEST(TaskCompletionSourceTest, SetExceptionWhenCalledThenTaskThrowsSameException)
  
  {
    TaskCompletionSource<int> tcs{};

    tcs.SetException(std::make_exception_ptr(invalid_argument{""}));

    ASSERT_THROW(tcs.GetTask().Result(), invalid_argument);
  }
  ```
```c++
  // Trying to set already completed TaskCompletionSource throws logic_error.
  TEST(TaskCompletionSourceTest, SetExceptionWhenTaskAlreadyCompletedThenThrowsLogicError)
  {
    TaskCompletionSource<int> tcs{};
    tcs.SetResult(0);
    
    ASSERT_THROW(tcs.SetException(make_exception_ptr(invalid_argument{""})), logic_error);
  }

```
## TaskCompletionSource TrySetException
```c++
TEST(TaskCompletionSourceTest, TrySetExceptionWhenCalledThenTaskThrowsSameException)
  {
    TaskCompletionSource<int> tcs{};

    tcs.TrySetException(std::make_exception_ptr(invalid_argument{""}));

    ASSERT_THROW(tcs.GetTask().Result(), invalid_argument);
  }
```

## TaskCompletionSource SetCanceled
```c++
  TEST(TaskCompletionSourceTest, SetCanceledWhenCalledThenTaskIsCanceled)
  {
    TaskCompletionSource<int> tcs{};
    tcs.SetCanceled();

    auto taskState = tcs.GetTask().State();

    ASSERT_EQ(taskState, TaskState::Canceled);
  }
  ```
  
  ```c++
  TEST(TaskCompletionSourceTest, SetCanceledWhenTaskAlreadyCompletedThenThrowsLogicError)
  {
    TaskCompletionSource<int> tcs{};
    tcs.SetResult(0);

    // Trying to set already completed TaskCompletionSource throws logic_error.
    ASSERT_THROW(tcs.SetCanceled(), logic_error);
  }

  ```
  ## TaskCompletionSource TrySetCanceled
  ```c++
  TEST(TaskCompletionSourceTest, TrySetCanceledWhenCalledThenTaskStateIsCanceled)
  {
    TaskCompletionSource<int> tcs{};
    tcs.TrySetCanceled();

    auto taskState = tcs.GetTask().State();

    ASSERT_EQ(taskState, TaskState::Canceled);
  }
  ```
  ```c++
  //Trying to set already completed TaskCompletionSource returns false 
  TEST(TaskCompletionSourceTest, TrySetCanceledWhenTaskAlreadyCompletedThenReturnsFalse)
  {
    TaskCompletionSource<int> tcs{};
    tcs.SetResult(0);

    
    auto trySetCanceledResult = tcs.TrySetCanceled();

    ASSERT_FALSE(trySetCanceledResult);
  }
  ```
## Task Fmap
```c++
TEST_F(TaskTest, FmapWhenMappingTaskThenMappedTaskHasExpectedResult)
  {
    const string EXPECTED_VALUE = "100";
    auto srcTask = TaskFromResult(10);
        
    auto mappedTask = Fmap(
                           Fmap(srcTask, [](int value)
                           {
                             return value * 10;
                           }),
                           [](int value)
                           {
                             return to_string(value);
                           });

    ASSERT_EQ(EXPECTED_VALUE, mappedTask.Result());
  }
```
## Task Fbind
```c++
TEST_F(TaskTest, FBindPipeOperatorWhenComposingThenReturnsExpectedResult)
  {
    const int initialValue= 10;
    const string EXPECTED_VALUE  = "5";
    auto initialTask = TaskFromResult(initialValue);
    auto mappedTask = initialTask 
                       | Fbind([](auto value) {return TaskFromResult(value * 2);})
                       | Fbind([](auto value) {return TaskFromResult(value / 4);})
                       | Fbind([](auto value) {return TaskFromResult(to_string(value));});

    ASSERT_EQ(EXPECTED_VALUE, mappedTask.Result());
  }
```

## Task Fjoin
```c++
TEST_F(TaskTest, FJoinWhenNestedTaskThenReturnsNestedTaskWithExpectedValue)
  {
    const int EXPECTED_RESULT = 42;

    Task<Task<int>> task{[value=EXPECTED_RESULT]()->Task<int>
                    {
                        co_await GetCompletedTask();
                        co_return value;
                    }};
    task.Start();
    auto innerTask = Fjoin(task);

    auto result = innerTask.Result();

   ASSERT_EQ(EXPECTED_RESULT, result);
  }
```

## Task Pipe Operator
```c++
 TEST_F(TaskTest, PipeOperatorWhenMixedComposingThenReturnsExpectedResult)
  {
    const int initialValue= 10;
    const string EXPECTED_VALUE  = "5";
    auto initialTask = TaskFromResult(initialValue);
    auto mappedTask = initialTask 
                       | Fbind([](auto value) {return TaskFromResult(value * 2);})
                       | Fmap([](auto value) {return value / 4;})
                       | Fbind([](auto value) {return TaskFromResult(to_string(value));});

    ASSERT_EQ(EXPECTED_VALUE, mappedTask.Result());
  }
```

```c++
  TEST_F(TaskTest, PipeOperatorWhenMixedComposingAndThrowsExceptionThenReturnsExpectedResult)
  {
    const int initialValue= 10;
    const string EXPECTED_VALUE  = "5";
    auto initialTask = TaskFromResult(initialValue);
    auto mappedTask = initialTask 
      | Fbind([](auto value) {throw std::invalid_argument{""}; return TaskFromResult(value * 2);})
                       | Fmap([](auto value) {return value / 4;})
                       | Fbind([](auto value) {return TaskFromResult(to_string(value));});

    ASSERT_THROW(mappedTask.Result(), invalid_argument);
  }
  ```
  
  ```c++
  //Fmap, Fbind, Fjoin
 
  TEST_F(TaskTest, PipeOperatorWhenUsingJoinThenReturnsExpectedResult)
  {
    const int initialValue= 10;
    const string EXPECTED_VALUE  = "5";
    auto initialTask = TaskFromResult(initialValue);
    auto mappedTask = initialTask 
                       | Fbind([](auto value) {return TaskFromResult(value * 2);})
    //Wrap Task<Task<int>>
                       | Fmap([](auto value) {return TaskFromResult(value / 4);})
    //Unwrap Task<Task<int>> -> Task<int>
                       | Fjoin()
    //Wrap Task<int> -> Task<Task<int>>
                       | Fmap([](auto value) {return TaskFromResult(value);})
    //Wrap Task<Task<int>> -> Task<Task<Task<int>>>
                       | Fmap([](auto value) {return TaskFromResult(value);})
    //Unwrap Task<Task<Task<int>>> -> Task<Task<int>>
                       | Fjoin()
    //Unwrap  Task<Task<int>> -> Task<int>
                       | Fjoin()
                       | Fbind([](auto value) {return TaskFromResult(to_string(value));});

    ASSERT_EQ(EXPECTED_VALUE, mappedTask.Result());
  }
  ```
## Task Monadic Laws
  ```c++
  //Right identity
  TEST_F(TaskTest, MonadRightIdentityLaw)
  {
    auto leftMonad = TaskFromResult(10);
    auto rightMonad = Fbind(leftMonad, [](int unwrappedValue)
    {
      return TaskFromResult(unwrappedValue);
    });

    ASSERT_EQ(leftMonad.Result(), rightMonad.Result());
  }

    //Left identity
  TEST_F(TaskTest, MonadLeftIdentityLaw)
  {
    const int initialValue = 10;

    auto selector = [](int value)
    {
      auto transformedvalue = value * 100;
      return TaskFromResult(transformedvalue);
    };

    auto rightMonad = selector(initialValue);

    auto leftMonad = Fbind(TaskFromResult(initialValue), selector);

    ASSERT_EQ(leftMonad.Result(), rightMonad.Result());
  }

  //Associativity law
  TEST_F(TaskTest, MonadAssociativityLaw)
  {
    const int initialValue = 10;

    auto initialMonad = TaskFromResult(initialValue);
    auto gTransformFunc = [](int value)
    {
      auto transformedValue = value * 10;
      return TaskFromResult(transformedValue);
    };

    auto hTransformFunc = [](int value)
    {
      auto transformedValue = value / 2;
      return TaskFromResult(transformedValue);
    };

    auto leftMonad = Fbind(Fbind(initialMonad, gTransformFunc), hTransformFunc);
    auto rightMonad = Fbind(initialMonad, [&gTransformFunc, &hTransformFunc](auto&& value)
    {
      return Fbind(gTransformFunc(value), hTransformFunc);
    });

    cout << "Result: " << leftMonad.Result();
    ASSERT_EQ(leftMonad.Result(), rightMonad.Result());
  }
  ```
## Flat DataFlow
    
```c++
    Tasks::Task<int> WhenAsyncFlatDataflowThenAllInputsProcessedImpl(int processItemsCount) const
    {
      //Create TransformBlock. As the name of the block suggests, TransformBlock transforms input to output.
      //Following block transforms int to string.
      auto transform1 = DataFlowAsyncFactory::CreateTransformBlock<int, string>([](const int& item)-> Tasks::Task<string>
                                                          {
                                                            auto message = "int: " + to_string(item) + "\n";
                                                            cout << message;
                                                            //await async operation returning standard shared_future.
                                                            co_await GetCompletedSharedFuture();
                                                            co_return to_string(item);
                                                          });

      //TransformBlock transforms a string to another string.
      auto transform2 = DataFlowAsyncFactory::CreateTransformBlock<string, string>([](const string& item)-> Tasks::Task<string>
                                                          {
                                                            auto message = "String transform: " + item + "\n";
                                                            cout << message;
                                                            //await async operation returning Task.
                                                            co_await Tasks::GetCompletedTask();
                                                            co_return item + ": {string}";
                                                          });

      //Create final (last) dataflow block. ActionBlock does not propagate output and usually performs some important "side effect".
      //For example: Save data to collection, send data to socket, write to log...
      //Following ActionBlock stores all strings which it has received from the previous block in the _processedItems collection.
      vector<string> _processedItems{};
      auto finalAction = DataFlowAsyncFactory::CreateActionBlock<string>([&_processedItems](const string& item)-> Tasks::Task<void>
                                                            {
                                                              auto message = "Final action: " + item + "\n";
                                                              cout << message;
                                                              //await async operation returning Task.
                                                              co_await Tasks::GetCompletedTask();
                                                              _processedItems.push_back(item);
                                                            });
      //Connect all dataflow nodes.
      transform1->Then(transform2)
                 ->Then(finalAction);

      //Start dataflow.
      transform1->Start();

      //Add input data to the first transform node.
      for (auto i = 0; i < processItemsCount; ++i)
      {
        co_await transform1->AcceptInputAsync(i);
      }

      //All input data are in the dataflow. Send notification that no more data will be added.
      //This does not mean that all data in the dataflow are processed!
      transform1->Complete();

      //Wait for completion. When finalAction (last block) completes, all data were processed.
      co_await finalAction->Completion();

      //_processedItems contains all transformed items.
      const auto processedItemsCount = _processedItems.size();

      co_return processedItemsCount;
    }
  };
  ```
   ## Fork-Join DataFlow
    
``` C++
Tasks::Task<int> WhenAsyncForkJoinDataflowThenAllInputsProcessedImpl(int inputItemsCount)
  {
      //Create TransformBlock. As the name of the block suggests, TransformBlock transforms input to output.
      //Following block transforms int to string.
      auto transform1 = DataFlowAsyncFactory::CreateTransformBlock<int, int>([](const int& item)-> Tasks::Task<int>
                                                            {
                                                              //Simulate work
                                                              co_await Tasks::GetCompletedTask();
                                                              auto message = "int: " + to_string(item) + "\n";
                                                              cout << message;
                                                              co_return item;
                                                            });

      //Fork dataflow (even numbers are processed in one TransformBlock, for odd numbers create another transformBlock)    
      auto transform2 =  DataFlowAsyncFactory::CreateTransformBlock<int, string>([](const int& item)->Tasks::Task<string>
                                                        {
                                                          //Simulate work
                                                          co_await Tasks::GetCompletedTask();
                                                          auto message = "Even number: " + to_string(item) + "\n";
                                                          cout << message;
                                                          co_return to_string(item);
                                                        },
                                                        //Accept only even numbers.
                                                        //Condition is evaluated for every input.
                                                        //If the condition evaluates to true, input is accepted; otherwise input is ignored.
                                                        [](const int& item)
                                                        {
                                                          return item % 2 == 0;
                                                        });

      auto transform3 = DataFlowAsyncFactory::CreateTransformBlock<int, string>([](const int& item)->Tasks::Task<string>
                                                      {
                                                         //Simulate work
                                                         co_await Tasks::GetCompletedTask();
                                                        auto message = "Odd number: " + to_string(item) + "\n";
                                                        cout << message;
                                                        co_return to_string(item);
                                                      },
                                                       //Accept only odd numbers.
                                                       //Condition is evaluated for every input.
                                                       //If the condition evaluates to true, input is accepted; otherwise input is ignored.
                                                      [](const int& item)
                                                      {
                                                        return item % 2 != 0;
                                                      });
      //End fork.

      vector<string> _processedItems{};
      auto finalAction = DataFlowSyncFactory::CreateActionBlock<string>([&_processedItems](const string& item)
                                                            {
                                                              auto message = "Final action: " + item + "\n";
                                                              cout << message;
                                                              _processedItems.push_back(item);
                                                            });
      //Fork
      transform1->ConnectTo(transform2);
      transform1->ConnectTo(transform3);
      //end fork

       //Join
      transform3->ConnectTo(finalAction);
      transform2->ConnectTo(finalAction);
      //End join

      //Start dataflow
      transform1->Start();

      //Add the input data to the first block.
      for (int i = 0; i < inputItemsCount; ++i)
      {
        co_await transform1->AcceptInputAsync(i);
      }

      
      //All input data are in the dataflow. Send notification that no more data will be added.
      //This does not mean that all data in the dataflow are processed!
      transform1->Complete();
      //Wait for last block.
      co_await finalAction->Completion();
      const auto processedItemsCount = _processedItems.size();

      co_return processedItemsCount;    
    }
  };
```
## Actor I
``` C++

  TEST(SimpleActorTest, WhenUsingSyncStatelessActorThenAllMessagesAreProcessed)
  {
    const int EXPECTED_MESSAGES = 101;
    auto seenMessages = 0;
    {
      //Create the actor (synchronous logic/without state/no reply).
      auto stateLessActor = CreateSimpleActor<int>([&seenMessages](const int& message) {seenMessages++; });
      for (int i = 0; i < EXPECTED_MESSAGES; i++)
      {
        //Send the message to the actor queue.
        stateLessActor->Tell(i);
      }

    }// The actor logic completes and the actor is destroyed. Alternatively, you can call the Complete method and wait for the actor Completion task.

    ASSERT_EQ(EXPECTED_MESSAGES, seenMessages);
  }
```

## Actor II
``` C++

  TEST(SimpleActorTest, WhenUsingAsyncStatefulActorThenHasExpectedState)
  {
    const int MESSAGES_COUNT = 101;
    const int EXPECTED_STATE = MESSAGES_COUNT;
    auto seenMessages = 0;
    auto testState = 0;
    {
      //Create the actor (asynchronous logic/with state/no reply).
      auto stateFullActor = RStein::AsyncCpp::Actors::CreateAsyncSimpleActor<int, int>([&seenMessages, &testState](const int& message, const int& state)->Task<int>
        {
          //Process the message argument.
          seenMessages++;

          //We can use the co_await keyword in the asynchronous method.
          co_await GetCompletedTask().ConfigureAwait(false);
          
          //Update the actor state, the argument state is an old (last) actor state.
          auto newState = state + 1;
          testState = newState;
          //Return the new state.
          co_return newState;
        }, testState);

      for (int i = 0; i < MESSAGES_COUNT; i++)
      {
        stateFullActor->Tell(i);
      }
    }// The actor logic completes and the actor is destroyed. Alternatively, you can call the Complete method and wait for the actor Completion task.
    
    ASSERT_EQ(EXPECTED_STATE, testState);
  }
```
## Actor III
``` C++
 
  TEST(ReplyActorTest, AskWhenUsingAsyncStatefulActorThenReturnsExpectedResponse)
  {
    const int MESSAGES_COUNT = 99;

    //Create the actor. Functional style/asynchronous logic/with state/returns reply task which eventually contains a result of the message processing.
    auto asyncStatefulReplyActor = CreateAsyncReplyActor<int, string, int>([](const int& message, const int& state)->Task<pair<string, int>>
    {
       //We can use the co_await keyword in the asynchronous method.
      co_await GetCompletedTask().ConfigureAwait(false);

      //The argument message (int) converted to a string is a result of the processing  - the first member of the returned pair.
      //Second member of the returned pair is a new actor state (always 0 in this case).
      co_return make_pair(to_string(message), 0);
    }, 0);

    for (auto i = 0; i < MESSAGES_COUNT; i++)
    {
      //Ask the actor and save the reply task.
      auto replyTask = asyncStatefulReplyActor->Ask(i);
      
      //Wait for (better co_await) the reply  task.
      auto response = replyTask.Result();
      
      ASSERT_EQ(to_string(i), response);
    }
  }// The actor logic completes and the actor is destroyed. Alternatively, you can call the Complete method and wait for the actor Completion task.
```
## Actor IV
``` C++

  //Actor inherits from the ActorPolicy class.
  class TestActor : public Actors::ActorPolicy
  {
  public:

    TestActor() :  ActorPolicy{}
    {

    }

    //Fire & forget method runs in the actor message loop.
    void RunVoidMethod(std::function<void()> scheduleFunction)
    {
      //Use the inherited ScheduleFunction method from the ActorPolicy to add the message to the actor queue.

      ScheduleFunction([scheduleFunction]
        {
          scheduleFunction();
          return Tasks::GetCompletedTask();
        });
    }

    //Method returns result of the processing wrapped in the Task<int>.
    Tasks::Task<int> ProcessInt(int number)
    {
       //Use the inherited ScheduleFunction method from the ActorPolicy to add the message to the actor queue.
       //ScheduleFunction returns Task<T>.
      return ScheduleFunction([number] {
        //Simply return the argument.
        return number;
        });
    }

     //Method returns result of the processing wrapped in the Task<int>.
    Tasks::Task<int> ProcessInt(std::function<int(int)> processIntFunc, int number)
    {
        //Use the inherited ScheduleFunction method from the ActorPolicy to add the message to the actor queue.
       //ScheduleFunction returns Task<T>.
      return ScheduleFunction([number, processIntFunc] {
        //Run the function in the actor 'logical' thread.
        return processIntFunc(number);
        });
    }

    [...] 
  };

  //Check that the message in the actor qeueue is processed.
  TEST(ActorPolicy, ScheduleFunctionWhenMethodCalledThenTaskIsCompletedWithExpectedResult)
  {
    const int EXPECTED_RESULT = 10;
    TestActor actor;
    //Add message to the actor queue.
    auto intTask = actor.ProcessInt(EXPECTED_RESULT);
    //Wait for the actor task.
    intTask.Wait();
     //Check that the actor method returns expected value.
    ASSERT_EQ(EXPECTED_RESULT, intTask.Result());
  }

  //Sum the data in the actor.
  TEST(ActorPolicy, ScheduleFunctionWhenCalledThenAllFunctionsAreProcessedSequentially)
  {
    const int SUM_TO = 100;
    const int EXPECTED_RESULT = SUM_TO * (SUM_TO + 1) / 2;
    TestActor actor;
    auto intTask = Tasks::Task<int>::InvalidPlaceholderTaskCreator()();
    auto sumResult = 0;

    for (int i = 0; i <= SUM_TO; ++i)
    {
      //Call the actor method ProcessInt (std::function<int(int)> processIntFunc, int number) overload.
      intTask = actor.ProcessInt([&sumResult](int number)
        {
          sumResult += number;
          return sumResult;
        }, i);
    }

    //Wait for the last processing result.
    intTask.Wait();

    auto lastTaskResult = intTask.Result();

    ASSERT_EQ(EXPECTED_RESULT, lastTaskResult);
    ASSERT_EQ(EXPECTED_RESULT, sumResult);
  }
```
## Actor V
``` C++
TEST(SimpleActorTest, PingPongTest)
  {
    const int PINGS_COUNT = 5;
    std::unique_ptr<IActor<string>> sigerus;
    std::unique_ptr<IActor<string>> thomasAquinas;

    //The logger is an actor
    auto logger = CreateSimpleActor<string>([](const string& message)
      {
        cout << message << endl;
      });

    //First player (actor)
    thomasAquinas = CreateSimpleActor<string, int>([PINGS_COUNT, &sigerus, &logger](const string& message, const int& pingsSent)
      {
        if (message == "start" || message.starts_with("pong"))
        {
          //Log the message.
          logger->Tell(message);

          //Update an actor state.
          auto newState = pingsSent + 1;

          //Send the 'ping' message.
          sigerus->Tell("ping " + to_string(newState));

          return newState;
        }        
        return pingsSent;
      }, 0);
      
      //Second player
      sigerus = CreateSimpleActor<string, int>([PINGS_COUNT, &thomasAquinas, &logger](const string& message, const int& pongsSent)
        {

          if (message.starts_with("ping"))
          {
             //Log the message.
            logger->Tell(message);

            //Update an actor state.
            auto newState = pongsSent + 1;

            //Send the 'pong' message or send the 'stop' message when the game is over.
            thomasAquinas->Tell(newState < 5
              ? "pong " + to_string(newState)
              : "stop");
              
            //Simulate wait.
            //missing Task.Delay
            this_thread::sleep_for(500ms);
            return newState;
          }
          return pongsSent;
        }, 0);

      //Start the game.
      thomasAquinas->Tell("start");
      this_thread::sleep_for(5s);
  }
```
## AsyncSemaphore
``` C++
[[nodiscard]] int waitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronizedImpl(int taskCount)
    {  

    //Not using our friendly Task. Simulate apocalypse with threads. :)
      cout << "start";
      const auto maxCount{1};
      const auto initialCount{0};

      AsyncSemaphore semaphore{maxCount, initialCount};
      std::vector<future<future<int>>> futures;
      futures.reserve(taskCount);

      int result = 0;
      for (auto i : generate(taskCount))
      {        

        packaged_task<future<int>(int*, AsyncSemaphore*, int)> task{
            [](int* result, AsyncSemaphore* semaphore, int i)-> future<int>
            {
              co_await semaphore->WaitAsync();
              (*result)++;
              semaphore->Release();
              co_return i;
            }
        };

        //Workaround, do not create and immediately throw away threads
        auto taskFuture = task.get_future();
        thread runner{std::move(task), &result, &semaphore, i};
        runner.detach();       
        futures.push_back(std::move(taskFuture));
      }
      semaphore.Release();
      for (auto&& future : futures)
      {
        auto nestedFuture = future.get();
        const auto taskId = nestedFuture.get();
        cout << "Task completed: " << taskId << endl;
      }

      return result;
    }
    
  TEST_F(AsyncSemaphoreTest, WaitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronized)
  {
    const auto TASKS_COUNT = 100;
    const auto EXPECTED_RESULT = 100;

    const auto result = waitAsyncWhenUsingMoreTasksThenAllTasksAreSynchronizedImpl(TASKS_COUNT);
    ASSERT_EQ(EXPECTED_RESULT, result);
  }
  ```
  ## CancellationToken
  ``` C++
  TEST_F(TaskTest, WaitWhenTaskProcessingCanceledThenThrowsOperationCanceledException)
  {
    //Create new CancellationTokenSource
    auto cts = CancellationTokenSource{};
    cts.Cancel();
    //Capture CancellationToken
    auto task = TaskFactory::Run([cancellationToken = cts.Token()]
    {
        //Task Run on default scheduler (ThreadPoolScheduler).
        while(true)
        {
          //Simulate work;
          this_thread::sleep_for(1000ms);

          //Monitor CancellationToken
          //When cancellationToken is canceled, then following call throws OperationCanceledException.
          cancellationToken.ThrowIfCancellationRequested();
        }
    }, cts.Token());

    //Signalize "Cancel operation" from another thread.
    cts.Cancel();
    ASSERT_THROW(task.Wait(), OperationCanceledException);
  }
  ```
## AsyncMutex
 ``` C++
  Task<int> LockWhenCalledThenExpectedNumberItemsAreInUnsafeCollectionImpl(int itemsCount)
    {
      std::vector<int> items;
      AsyncMutex asyncMutex;

      for (int i = 0; i < itemsCount; ++i)
      {
        //test only repeated Lock/implicit Unlock without concurrency
        auto locker = asyncMutex.Lock();
        co_await locker;
        items.push_back(i);
        //implicit locker.Unlock();
      }

      co_return items.size();
    }
 ```

 ## SynchronizationContext
 ``` C++
 using UiSynchronizationContext = Mocks::TestSynchronizationContextMock;

  TEST(Scheduler, FromSynchronizationContextReturnsSchedulerBasedOnSynchronizationContext)
  {
    //Assume that this is a special (non-default) synchronization context  - UI context, event loop, dedicated service thread...
    UiSynchronizationContext uicontext;
    
    //UI framework/Specialized service installs special context.

    Threading::SynchronizationContext::SetSynchronizationContext(&uicontext);
    auto myCompletionHandler = []
    {
      //UI controls like textbox expects access from the UI thread.
      //textbox.Text = GetAsyncResult();
      
    };

    //UI framework/Specialized service calls infrastructure code (e. g. async processor)
    //asyncProcessor.Run(heavyWorkForBackgroundThread, myCompletionHandler);
   
    //Another part of the application, typically infrastructure code (e. g. async processor) captures synchronization context for calling thread and wraps it in scheduler.  

    auto callingThreadScheduler = Schedulers::Scheduler::FromCurrentSynchronizationContext();
    auto clientHandler = myCompletionHandler;
    //Async processor executes async work, possibly in another thread (task, scheduler).

    //[time passed...]

    //Async processor completes work in another thread and then invokes clientHandler in the UI synchronization context.
    callingThreadScheduler->EnqueueItem(clientHandler);

    ASSERT_TRUE(uicontext.WasPostCalled());
  }
}
 ```
## SynchronizationContextScope
``` C++
  TEST(SynchronizationContextScope, DtorWhenCalledThenOldContextIsRestored)
  {

    auto oldContext = SynchronizationContext::Current();
    TestSynchronizationContextMock synchronizationContextMock;         
    {
      //synchronizationContextMock is the SynchronizationContext::Current()
      SynchronizationContextScope syncScope(synchronizationContextMock);
      SynchronizationContext::Current()->Post([]{});
      SynchronizationContext::Current()->Send([]{});
    } //synchronizationContextMock is removed and previous context is restored

    auto restoredContext = SynchronizationContext::Current();
    
    ASSERT_EQ(oldContext, restoredContext);

  }
```
``` C++
//Nesting synchronization contexts

  TEST(SynchronizationContextScope, CtorWhenNestedScopeThenNestedContextIsUsed)
  {

    TestSynchronizationContextMock synchronizationContextMock;
    TestSynchronizationContextMock nestedSynchronizationContextMock;

    {
    //synchronizationContextMock is the SynchronizationContext::Current()      
      SynchronizationContextScope syncScope(synchronizationContextMock);
      {
        //nestedSynchronizationContextMock is the SynchronizationContext::Current()
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
```
