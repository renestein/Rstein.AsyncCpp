#include "Task.h"

#include "../AsyncPrimitives/OperationCanceledException.h"
#include "../Collections/ThreadSafeMinimalisticVector.h"
#include "../Schedulers/Scheduler.h"
#include <cassert>
#include <utility>
#include <iostream>

using namespace RStein::AsyncCpp::Collections;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace RStein::AsyncCpp::Schedulers;
using namespace RStein::AsyncCpp::Tasks::Detail;
using namespace std;

namespace RStein::AsyncCpp::Tasks
{

 


  Task::Task(std::function<void()>&& action) : _sharedTaskState{std::make_shared<TypedTaskSharedState<std::function<void()>>>(std::move(action),
                                                                                                                                       false,
                                                                                                                                       CancellationToken::None())}
  {

  }

  Task::Task(std::function<void()>&& action, const CancellationToken::CancellationTokenPtr& cancellationToken) : _sharedTaskState{std::make_shared<TypedTaskSharedState<std::function<void()>>>(std::move(action),
                                                                                                                                                                                                false,
                                                                                                                                                                                                cancellationToken)}
  {
    
  }

  Task::Task(std::function<Task()>&& action): _sharedTaskState{std::make_shared<TypedTaskSharedState<std::function<void()>>>(std::move(action),
                                                                                                                                       true,
                                                                                                                                       CancellationToken::None())}
  {
    
  }

  Task::Task(std::function<Task()>&& action, const CancellationToken::CancellationTokenPtr& cancellationToken) : _sharedTaskState{std::make_shared<TypedTaskSharedState<std::function<void()>>>(std::move(action),
                                                                                                                                                                                                false,
                                                                                                                                                                                                cancellationToken)}
  {
  }

  unsigned long Task::Id() const
  {
    return _sharedTaskState->Id();
  }

  void Task::Start()
  {
    _sharedTaskState->RunTaskFunc();
  }

  bool Task::IsCanceled() const
  {
    return _sharedTaskState->State() == TaskState::Canceled;
  }

  bool Task::IsCompleted() const
  {
    auto taskState = _sharedTaskState->State();
    return taskState == TaskState::RunToCompletion ||
           taskState == TaskState::Faulted ||
            taskState == TaskState::Canceled;

  }

  bool Task::IsFaulted() const
  {
    return _sharedTaskState->State() == TaskState::Faulted;
  }

  TaskState Task::State() const
  {
    return _sharedTaskState->State();
  }

  void Task::Wait() const
  {
    return _sharedTaskState->Wait();
  }

  Task Task::ContinueWith(std::function<void(const Task& task)>&& continuation)
  {
    if (!continuation)
    {
      throw std::invalid_argument("continuation");
    }

    Task continuationTask{[continuation = std::move(continuation), thisCopy=*this]{continuation(thisCopy);}};
    addContinuation(continuationTask);  
    return continuationTask;
  }

  std::exception_ptr Task::Exception() const
  {
    return _sharedTaskState->Exception();
  }

  Task::Task() : _sharedTaskState{}
  {

  }

  void Task::addContinuation(Task& continuationTask) const
  {
    _sharedTaskState->AddContinuation([continuationTask] () mutable{continuationTask.Start();});
  }


  Task Task::Run(std::function<void()>&& action)
  {
    return Run(std::move(action), CancellationToken::None());
  }

  Task Task::Run(std::function<void()>&& action, const CancellationToken::CancellationTokenPtr& cancellationToken)
  {
     auto task = Task{std::move(action), cancellationToken};
     task.Start();
     return task;
  }

 /* Task Task::Run(std::function<Task()>&& action)
  {
  }

  Task Task::Run(std::function<Task()>&& action, const CancellationToken::CancellationTokenPtr& cancellationToken)
  {
  }*/
}
