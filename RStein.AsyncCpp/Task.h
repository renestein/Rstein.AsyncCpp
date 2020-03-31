#pragma once
#include <exception>

namespace RStein::AsyncCpp::Tasks
{
  enum TaskState
  {
    Created,
    Scheduled,
    Running,
    Faulted,
    Canceled,
    Completed
  };

  class Task
  {
  public:

    int Id();
    void Start();
    bool IsCanceled();
    bool IsCompleted();
    bool IsFaulted();
    TaskState State;
    //Add overloads
    static Task Run();
    void Wait();
    //Add overloads
    void ContinueWith();

    std::exception_ptr Exception;

  };

  template<typename TResult>
  class TaskT : Task
  {

  };

}
