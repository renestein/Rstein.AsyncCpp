#include "StrandSchedulerDecorator.h"
#include <stdexcept>

using namespace std;

namespace RStein::AsyncCpp::Schedulers
{
  StrandSchedulerDecorator::StrandSchedulerDecorator(const std::shared_ptr<Scheduler>& scheduler) :
    _scheduler(scheduler),
    _strandQueue(),
    _queueMutex(),
    _operationInProgress(false)

  {
    if (!scheduler)
    {
      invalid_argument invalidSchedulerEx("scheduler");
      throw invalidSchedulerEx;
    }
  }


  StrandSchedulerDecorator::~StrandSchedulerDecorator() = default;

  void StrandSchedulerDecorator::Start()
  {
    _scheduler->Start();
  }

  void StrandSchedulerDecorator::Stop()
  {
    _scheduler->Stop();
  }

  void StrandSchedulerDecorator::OnEnqueueItem(std::function<void()>&& originalFunction)
  {
    if (_scheduler->IsMethodInvocationSerialized())
    {
      _scheduler->EnqueueItem(move(originalFunction));
      return;
    }

    auto wrappedFunction = wrapFunctionInStrand(move(originalFunction));
    lock_guard<mutex> lock(_queueMutex);

    tryRunItem(move(wrappedFunction));
  }

  bool StrandSchedulerDecorator::IsMethodInvocationSerialized () const
  {
    return true;
  }

  std::function<void()> StrandSchedulerDecorator::wrapFunctionInStrand(std::function<void()>&& originalFunction)
  {
    return [originalFunction, this]
    {
      originalFunction();
      markStrandOperationAsDone();
    };
  }

  void StrandSchedulerDecorator::markStrandOperationAsDone()
  {
    lock_guard<mutex> lock(_queueMutex);
    _operationInProgress.store(false);
    tryDequeItem();
  }

  void StrandSchedulerDecorator::tryDequeItem()
  {
    if (_strandQueue.empty())
    {
      return;
    }

    auto function = _strandQueue.front();
    _strandQueue.pop();
    tryRunItem(move(function));
  }

  void StrandSchedulerDecorator::tryRunItem(std::function<void()>&& originalFunction)
  {
    if (!_operationInProgress.load())
    {
      _operationInProgress.store(true);
      runOnOriginalScheduler(move(originalFunction));
      return;
    }

    _strandQueue.push(move(originalFunction));
  }

  void StrandSchedulerDecorator::runOnOriginalScheduler(std::function<void()>&& originalFunction) const
  {
    _scheduler->EnqueueItem(move(originalFunction));
  }
}
