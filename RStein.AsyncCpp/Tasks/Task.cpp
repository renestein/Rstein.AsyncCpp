#include "Task.h"


#include "../AsyncPrimitives/CancellationToken.h"
#include "../AsyncPrimitives/OperationCanceledException.h"
#include "../Collections/ThreadSafeMinimalisticVector.h"
#include "../Schedulers/Scheduler.h"
#include "../Utils/FinallyBlock.h"
#include <cassert>
#include <utility>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <iostream>

using namespace RStein::AsyncCpp::Collections;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace RStein::AsyncCpp::Schedulers;
using namespace std;

namespace RStein::AsyncCpp::Tasks
{

  struct NONE_RESULT
  {
    
  };

  template <typename TFunc>
  struct FunctionWrapper
  {
    public:
    
      using Function_Ret_Type = decltype(declval<TFunc>()());
      using Is_Void_Return_Function = is_same<Function_Ret_Type, void>;
      using Ret_Type =  conditional_t<Is_Void_Return_Function::value, NONE_RESULT, Function_Ret_Type>;

      FunctionWrapper(TFunc&& func,
                      bool isTaskReturnFunc,
                      CancellationToken::CancellationTokenPtr cancellationToken)
        : _func(std::move(func)),
          _isTaskReturnFunc(isTaskReturnFunc),
          _cancellationToken(std::move(cancellationToken)),
          _retValue{},
          _hasRetValue{false}
      {
        if (!_func)
        {
          throw std::invalid_argument{"func"};
        }
          
      }

      void Run()
      {
        assert(!_hasRetValue);
        if constexpr(Is_Void_Return_Function::value)
        {
          _func();
        }
        else
        {
          _retValue = _func();  
        }
        _hasRetValue = true;
      }

      bool IsTaskBasedReturnFunc() const
      {
        return _isTaskReturnFunc;
      }

      constexpr bool IsVoidFunc()
      {
        return Is_Void_Return_Function::value;
      }

      Ret_Type ReturnValue() const
      {
        assert(_hasRetValue);
        return _retValue;
      }

      CancellationToken::CancellationTokenPtr CancellationToken() const
      {
        return _cancellationToken;
      }

      bool IsCancellationRequested() const
      {
        return _cancellationToken->IsCancellationRequested();
      }


  private:
    TFunc _func;
    bool _isTaskReturnFunc;
    CancellationToken::CancellationTokenPtr _cancellationToken;
    Ret_Type _retValue;
    bool _hasRetValue;
  };

  struct Task::TaskSharedState : public enable_shared_from_this<TaskSharedState>
  {
  public:

    TaskSharedState() : enable_shared_from_this<TaskSharedState>{},
                        _lockObject{},
                        _waitTaskCv{},
                        _scheduler{Scheduler::DefaultScheduler()},
                        _taskId{_idGenerator++},    
                        _state{TaskState::Created},
                        _continuations{vector<Task>{}},
                        _exceptionPtr{nullptr}

    {      
    }

    unsigned long Id() const
    {
      return _taskId;
    }

    TaskState State()
    {
      lock_guard lock{_lockObject};
      return _state;
    }
    bool HasException() const
    {
      return _exceptionPtr != nullptr;
    }

    exception_ptr Exception() const
    {
      return _exceptionPtr;
    }


    virtual ~TaskSharedState()
    {
        lock_guard lock{_lockObject};
        if (_state != TaskState::RunToCompletion &&
            _state != TaskState::Faulted &&
            _state != TaskState::Canceled)
        {
          throw std::logic_error("Broken Task shared state");
        }
    }
   
    void RunTaskFunc()
    {     
        auto isCtCanceled = IsCtCanceled();
        if (isCtCanceled)
        {
          {
            lock_guard lock{_lockObject};
            _state = TaskState::Canceled;
          }

          _waitTaskCv.notify_all();
          runContinuations();
         
          return;
        }

        {
          lock_guard lock{_lockObject};
          
          if (_state != TaskState::Created)
          {
            throw std::logic_error("Task already started.");
          }


          _state = TaskState::Scheduled;
        }

      _scheduler->EnqueueItem([this, sharedThis = shared_from_this()]{

                  Utils::FinallyBlock finally
                  {
                    
                    [this]
                    {
                     
                       _waitTaskCv.notify_all();                      
                      runContinuations();
                    }
                  };
                    try
                    {
                      CancellationToken()->ThrowIfCancellationRequested();

                      {
                        lock_guard lock{_lockObject};
                        assert(_state == TaskState::Scheduled);
                        _state = TaskState::Running;
                      }

                      DoRunTaskNow();
                     _state = TaskState::RunToCompletion;
                    }      
                    catch(const OperationCanceledException&)
                  {
                      _exceptionPtr = current_exception();
                      lock_guard lock{_lockObject};
                       if (_state == TaskState::Canceled)
                       {
                         __debugbreak();
                       }
                      assert(_state == TaskState::Scheduled || _state == TaskState::Running);
                      _state = TaskState::Canceled;
                    }
                    catch (...)
                    {
                      _exceptionPtr = current_exception();
                      lock_guard lock{_lockObject};
                      assert(_state == TaskState::Running);
                      _state = TaskState::Faulted;
                     
                    }
            });
    }
 
    void Wait() const
    {
       unique_lock lock{_lockObject};
      while(_state != TaskState::RunToCompletion &&
            _state != TaskState::Faulted &&
            _state != TaskState::Canceled)
      {
        _waitTaskCv.wait(lock);
      }

      if (_state != TaskState::RunToCompletion)
      {
        auto exceptionPtr = Exception();
        if (exceptionPtr == nullptr && _state == TaskState::Canceled)
        {
          exceptionPtr = make_exception_ptr(OperationCanceledException());
        }

        rethrow_exception(exceptionPtr);
      }
    }

    void AddContinuation(Task& continuationTask)
    {
         //TODO inline task if possible
         lock_guard lock{_lockObject};
         if (_state ==  TaskState::RunToCompletion ||
            _state == TaskState::Faulted ||
            _state == TaskState::Canceled)
         {
           continuationTask.Start();
           return;
         }

        _continuations.Add(continuationTask);
    }

    virtual bool IsTaskBasedReturnFunc() = 0;
    virtual CancellationToken::CancellationTokenPtr CancellationToken() = 0;
    virtual bool IsCtCanceled() = 0;
    virtual any GetTaskResult() const = 0;
     
  protected:
    mutable mutex _lockObject;
    mutable condition_variable _waitTaskCv;
    Scheduler::SchedulerPtr _scheduler;
    virtual void DoRunTaskNow() = 0;
  private:
    //TODO Check overflow.
    inline static std::atomic<unsigned long> _idGenerator{};
    unsigned long _taskId;
    TaskState _state;
    ThreadSafeMinimalisticVector<Task> _continuations;
    exception_ptr _exceptionPtr;
    void runContinuations()
    {
      vector<Task> continuations;
      {
         lock_guard lock{_lockObject};
        assert(_state ==  TaskState::RunToCompletion ||
              _state == TaskState::Faulted ||
              _state == TaskState::Canceled);

        continuations = _continuations.GetSnapshot();
        _continuations.Clear();
      }

      for(auto& continuationTask : continuations)
      {
        continuationTask.Start();
      }
    }

  };

  template <typename TFunc>
  struct TypedTaskSharedState : Task::TaskSharedState
  {
  public:

    TypedTaskSharedState(TFunc func, bool isTaskReturnFunc, CancellationToken::CancellationTokenPtr cancellationToken): TaskSharedState{},
                                                                                                                       _func(std::move(func),
                                                                                                                             isTaskReturnFunc,
                                                                                                                              cancellationToken)
    {

    }

    bool IsTaskBasedReturnFunc() override
    {
      return _func.IsTaskBasedReturnFunc();
    }

    CancellationToken::CancellationTokenPtr CancellationToken() override
    {
      return _func.CancellationToken();
    }

    typename FunctionWrapper<TFunc>::Ret_Type GetResult() const
    {
      Wait();
      return _func.ReturnValue();
    }

    bool IsCtCanceled() override
    {
      return _func.IsCancellationRequested();
    }

    
    virtual ~TypedTaskSharedState() = default;
    
  protected:

    void DoRunTaskNow() override
    {   
      _func.Run();
    }

    any GetTaskResult() const override
    {
      return std::forward<FunctionWrapper<TFunc>::Ret_Type>(GetResult());
    }

  private:
    FunctionWrapper<TFunc> _func;
  };


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


  template <typename TR>
  Task::Task(std::function<TR()>&& func) : std::make_shared<TypedTaskSharedState<TR>>(std::move(func),
                                            false,
                                            CancellationToken::None())
  {

  }

  template <typename TR>
  Task::Task(std::function<TR()>&& func, AsyncPrimitives::CancellationToken::CancellationTokenPtr cancellationToken) : std::make_shared<TypedTaskSharedState<TR>>(std::move(func),
                                                                                                                       false,
                                                                                                                        cancellationToken)
  {
  }

  template <typename TR>
  Task::Task(std::function<TaskT<TR>>&& func) : std::make_shared<TypedTaskSharedState<TR>>(std::move(func),
                                                true,
                                                CancellationToken::None())
  {
  }

  template <typename TR>
  Task::Task(std::function<TaskT<TR>>&& func, AsyncPrimitives::CancellationToken::CancellationTokenPtr) : std::make_shared<TypedTaskSharedState<TR>>(std::move(func),
                                                                                                          true,
                                                                                                          CancellationToken::None())
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

  std::any Task::GetTaskResult() const
  {
    return _sharedTaskState->GetTaskResult();
  }

  void Task::addContinuation(Task& continuationTask) const
  {
    _sharedTaskState->AddContinuation(continuationTask);
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
