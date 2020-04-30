# RStein.AsyncCpp (C++ library)
- The RStein.AsyncCpp library is a set of types that should be familiar for anyone who know the Task Parallel Library (TPL) for .NET (C#). In addition, this library contains simple DataFlow, functional combinators for the Task<T> class, useful async primitives (AsyncSemaphore, AsyncProducerConsumerCollection, CancellationToken, CancellationTokenSource ...).

- The library is my playground for testing coroutine support in the C++.
- The library supports compilation in the VS 2019. Support for other compilers is planned.
## **Task&lt;T&gt;.** 
The Task class represents result of the execution of the one (usually asynchronous) operation - an instance of the Task contains either return value of the operation or exception or information that task was cancelled. Tasks created by the TaskFactory are 'hot'. 'Hot' in this context means that the Task Start method is called and Task is immediately scheduled on Scheduler (~=executor). You can 'co_await' Task (preferred) or/and you can use methods from the Task public interface (ContinueWith, Wait, State, IsCompleted, IsFaulted, IsCanceled, Result...). Task supports both the 'awaiter' and the 'promise' concepts.
* [`Create Task<T> using the TaskFactory (uses default scheduler = ThreadPoolScheduler).`](#TaskFactory-Run)
* [`Create Task<T> using the TaskFactory and explicit scheduler.`](#TaskFactory-Run-With-Scheduler)
* [`Task<T> ContinueWith method - register continuation function that will be called when the Task have completed ('future.then' type of the method).`](#Task-ContinueWith)

* [`Task<T> in a 'promise' role (return type of the C++ coroutine).`](#Task-Promise-Concept)

* [`WaitAll method for tasks - synchronously waits for the completion of the all tasks.`](#Task-WaitAll)

* [`WaitAny method for tasks - Synchronously waits for the completion of the any Task. `](#Task-WaitAny)

* [`WhenAll method for tasks - returns Task that will be completed when all of the provided tasks have completed.`](#Task-WhenAll)

* [`WhenAny method for tasks - returns Task that will be completed when any of the provided tasks have completed.`](#Task-WhenAny)

* [`TaskFromResult method - creates a completed instance of the Task<T> with the specified value.`](#TaskFromResult)

* [`TaskFromResult method - creates a completed instance of the Task<T> with the specified value.`](#TaskFromResult)

* [`TaskFromException method - creates a completed instance of the Task<T> with the specified exception.`](#TaskFromException)

* [`TaskFromCanceled method - creates a completed instance of the Task<T> in the Canceled state.`](#TaskFromCanceled)
  
  ## **TaskCompletionSource&lt;T&gt;.** 
  The TaskCompletionSource class explicitly controls the state and result of the Task that is provided to the consumer. TaskCompletionSource has relation to the Task class similar to relation between std::future and std::promise types. This class is very useful in situations when you must call code that uses different asynchronous patterns and you would like only the Task class in your API. Different asynchronous patterns may be simply converted to Task based world using the TaskCompletionSource.
  * [`TaskCompletion<T> SetResult method).`](#TaskCompletionSource-SetResult)
  * [`TaskCompletion<T> TrySetResult method).`](#TaskCompletionSource-TrySetResult)
  * [`TaskCompletion<T> SetException method).`](#TaskCompletionSource-SetException)
  * [`TaskCompletion<T> TrySetException method).`](#TaskCompletionSource-TrySetException)
  * [`TaskCompletion<T> SetCanceled method).`](#TaskCompletionSource-SetCanceled)
  * [`TaskCompletion<T> TrySetCanceled method).`](#TaskCompletionSource-TrySetCanceled)

## **Functional map (Fmap) and bind (Fbind) methods (Task monad).**
The [`TaskFromResult method `](#TaskFromResult) can be used as a Unit (Return) method.

 * [`Task<T> Fmap (map, Select) method.`](#Task-Fmap)
 * [`Task<T> Fbind (bind, SelectMany, mapMany) method.`](#Task-Fbind)
 * [`| (pipe) operator for Fbind and FMap` - simple composition](#Task-Pipe-Operator)]
 * [`Monadic laws (tests)`](#Task-Monadic-Laws)]
 
 

  ## TaskFactory Run
  Create Task<T> using the TaskFactory (uses default scheduler - ThreadPoolScheduler).
  ```c++

  //Using co_await
  auto result = co_await TaskFactory::Run([expectedValue]
      {
        return expectedValue;
      });
      co_return result;
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

      auto task = TaskFactory::Run([&taskScheduler]
                  {
                    //capture used scheduler
                    return taskScheduler;
                    
                  },
                  //run on explicit scheduler
                 taskSchSynceduler);

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
## Task ContinueWith
```c++
ContinueWith method registers continuation function which will be called when the Task is completed.
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
Continuation function can be called on an explicitly specified Scheduler.

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
Task<string> is a promise
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