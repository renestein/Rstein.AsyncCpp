#pragma once
namespace RStein::AsyncCpp::Tasks
{
  enum class TaskState
  {
    Created,
    Scheduled,
    Running,
    Faulted,
    Canceled,
    RunToCompletion
  };
}
