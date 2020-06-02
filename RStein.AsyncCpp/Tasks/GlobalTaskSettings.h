#pragma once

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

  //This key is for internal use.
  inline static int TaskAwaiterAwaitReadyAlwaysReturnsFalse = false;
private:

  GlobalTaskSettings() = default;
  
};
