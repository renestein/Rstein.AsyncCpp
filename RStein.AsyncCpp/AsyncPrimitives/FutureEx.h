#pragma once
#include <future>
#include <iostream>
#include <thread>

namespace RStein::AsyncCpp::AsyncPrimitives
{
    template<typename TR>
    std::shared_future<TR> GetCompletedSharedFuture(TR val)
    {
       std::promise<TR> _promise;
       _promise.set_value(val);
       auto _completedFuture = _promise.get_future().share();

      return _completedFuture;
    }

    inline std::shared_future<void> GetCompletedSharedFuture()
    {
      std::promise<void> promise;
       promise.set_value();
      auto _completedFuture = promise.get_future().share();
       return _completedFuture;
    }

    inline std::future<void> GetCompletedFuture()
    {
      std::promise<void> promise;
       promise.set_value();
      auto _completedFuture = promise.get_future();
       return _completedFuture;
    }

    inline auto operator co_await(const std::shared_future<void>& future)
    {
      struct SharedFutureAwaiter
      {
        std::shared_future<void> Future;

        [[nodiscard]] bool await_ready() const 
        {
          return Future._Is_ready();
        }

        [[nodiscard]] bool await_suspend(std::experimental::coroutine_handle<> coroutine) const
        {
          if (Future._Is_ready())
          {
            return false;
          }

          //This is really terrible awaiter, we are wasting thread (but MS do the same?)
          std::thread waitThread{[coroutine=std::move(coroutine), waitToFuture = Future]()
          {
            try
            {
              waitToFuture.get();
            }
            catch(...)
            {
              std::cerr << "Future awaiter: exception while waiting";
            }

            coroutine.resume();
          }};

          waitThread.detach();
          return true;
        }

        void await_resume() const
        {
          Future.get();
        } 

      };

      return SharedFutureAwaiter{future};
    }

    template<typename TR>
    auto operator co_await(const std::shared_future<TR>& future)
    {
      struct SharedFutureAwaiter
      {
        std::shared_future<TR> Future;

        [[nodiscard]] bool await_ready() const
        {
          return Future._Is_ready();
        }

        [[nodiscard]] bool await_suspend(std::experimental::coroutine_handle<> coroutine) const
        {
          std::cerr << "In await_suspend\n";
          if (Future._Is_ready())
          {
            std::cerr << "await_suspend - future is ready.\n";
            return false;
          }

          //This is really terrible awaiter, we are wasting thread (but MS do the same?)
          std::thread waitThread{[coroutine=std::move(coroutine), waitToFuture = Future]()
          {
            try
            {
              waitToFuture.get();
            }
            catch(...)
            {
              std::cerr << "Future awaiter: exception while waiting\n";
            }

            coroutine.resume();
          }};
          std::cerr << "await_suspend - detaching thread.\n";
          waitThread.detach();
          return true;
        }

        decltype(auto) await_resume() const
        {
          return Future.get();
        }
      };

      return SharedFutureAwaiter{future};
    }
}
