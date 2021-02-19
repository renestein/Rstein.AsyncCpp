#include "SimpleThreadPool.h"
#include <stdexcept>

using namespace std;

namespace RStein::AsyncCpp::Schedulers
{
  SimpleThreadPool::SimpleThreadPool() : SimpleThreadPool(thread::hardware_concurrency())
  {
  }

  SimpleThreadPool::SimpleThreadPool(unsigned int numberOfThreads) : _innerQueue(),
                                                                  _lockRoot(),
                                                                  _queueConditionVariable(),
                                                                  _threadPoolState(ThreadPoolState::Created),
                                                                  _numberOfThreads(numberOfThreads),
                                                                  _quitRequest(false)

  {
    if (numberOfThreads == 0)
    {
      invalid_argument numberOfThreadsInvalidEx("numberOfThreads");
      throw numberOfThreadsInvalidEx;
    }
  }


  SimpleThreadPool::~SimpleThreadPool()
  {
    if (_threadPoolState == ThreadPoolState::Started)
    {
      /*throwInvalidThreadPoolState();*/
      //Log invalid life cycle
      try
      {
      Stop();
      }
      catch(...)
      {
        
      }    
    }
  }

  void SimpleThreadPool::Start()
  {
    if (_threadPoolState != ThreadPoolState::Created)
    {
      throwInvalidThreadPoolState();
    }

    for (auto i = 0u; i < _numberOfThreads; i++)
  {
      _threads.emplace_back([this]
      {
        try
        {       
          do
          {
            WorkItem currentWorkItem;

            {
              unique_lock<mutex> lock(_lockRoot);
              while (!_quitRequest.load() && _innerQueue.empty())
              {
                _queueConditionVariable.wait(lock);
              }

              if (_quitRequest.load() && _innerQueue.empty())
              {
                break;
              }

              currentWorkItem = move(_innerQueue.front());
              _innerQueue.pop();
            }

            try
            {
              currentWorkItem();
            }
            catch (...)
            {
              __debugbreak();
            }
          } while (!_quitRequest.load());
      }      
      catch(...)
      {
          
      }
     
    });

    _threadPoolState = ThreadPoolState::Started;

    }
  }

  void SimpleThreadPool::Stop()
  {
    if (_threadPoolState != ThreadPoolState::Started)
    {
      throwInvalidThreadPoolState();
    }

    _quitRequest.store(true);
    _queueConditionVariable.notify_all();
    for (auto& thread : _threads)
    {
      thread.join();
    }

    _threadPoolState = ThreadPoolState::Stopped;
  }

  void SimpleThreadPool::EnqueueItem(WorkItem originalFunction)
  {
    unique_lock<mutex> lock(_lockRoot);
    _innerQueue.push(move(originalFunction));
    _queueConditionVariable.notify_one();
  }

  unsigned SimpleThreadPool::GetNumberOfThreads() const
  {
    return _numberOfThreads;
  }

  SimpleThreadPool::ThreadPoolState SimpleThreadPool::GetThreadPoolState() const
  {
    return _threadPoolState;
  }

  void SimpleThreadPool::throwInvalidThreadPoolState()
  {
    throw std::logic_error("ThreadPool is in invalid state.");
  }
}
