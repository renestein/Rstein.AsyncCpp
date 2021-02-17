#pragma once
#include <future>
#include <iostream>
#include <thread>

#if defined(__clang__)
#include "../ClangWinSpecific/Coroutine.h"
#elif defined(__cpp_impl_coroutine)
#include <coroutine>
#else
#include <experimental/coroutine>
#endif

namespace RStein::AsyncCpp::AsyncPrimitives
{
  template <typename TR>
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

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] bool await_suspend(std::coroutine_handle<> coroutine) const
#else
      [[nodiscard]] bool await_suspend(std::experimental::coroutine_handle<> coroutine) const
#endif
      {
        if (Future._Is_ready())
        {
          return false;
        }

        //This is really terrible awaiter, we are wasting thread (but MS do the same?)
        std::thread waitThread{
            [coroutine= coroutine, waitToFuture = Future]()
            {
              try
              {
                waitToFuture.get();
              }
              catch (...)
              {
                std::cerr << "Future awaiter: exception while waiting";
              }

              coroutine.resume();
            }
        };

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

  template <typename TR>
  auto operator co_await(const std::shared_future<TR>& future)
  {
    struct SharedFutureAwaiter
    {
      std::shared_future<TR> Future;

      [[nodiscard]] bool await_ready() const
      {
        return Future._Is_ready();
      }

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
      [[nodiscard]] bool await_suspend(std::coroutine_handle<> coroutine) const
#else
      [[nodiscard]] bool await_suspend(std::experimental::coroutine_handle<> coroutine) const
#endif

      {
        std::cerr << "In await_suspend\n";
        if (Future._Is_ready())
        {
          std::cerr << "await_suspend - future is ready.\n";
          return false;
        }

        //This is really terrible awaiter, we are wasting thread (but MS do the same?)
        std::thread waitThread{
            [coroutine = coroutine, waitToFuture = Future]()
            {
              try
              {
                waitToFuture.get();
              }
              catch (...)
              {
                std::cerr << "Future awaiter: exception while waiting\n";
              }

              coroutine.resume();
            }
        };
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

  struct SharedVoidFutureCoroutinePromise
  {
    std::promise<void> _promise;
    std::shared_future<void> _future;

    SharedVoidFutureCoroutinePromise() : _promise(),
                                         _future(_promise.get_future().share())
    {
    }

    [[nodiscard]] std::shared_future<void> get_return_object() const
    {
      return _future;
    }

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never initial_suspend() const noexcept
#else
    [[nodiscard]] std::experimental::suspend_never initial_suspend() const noexcept
#endif
    {
      return {};
    }

    void return_void()
    {
      _promise.set_value();
    }

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never final_suspend() const noexcept
#else
    [[nodiscard]] std::experimental::suspend_never final_suspend() const noexcept
#endif
    {
      return {};
    }

    void unhandled_exception()
    {
      _promise.set_exception(std::current_exception());
    }
  };

  template <typename TR>
  struct SharedFutureTCoroutinePromise
  {
    std::promise<TR> _promise;
    std::shared_future<TR> _future;

    SharedFutureTCoroutinePromise() : _promise(),
                                      _future(_promise.get_future().share())
    {
    }

    [[nodiscard]] std::shared_future<TR> get_return_object() const
    {
      return _future;
    }

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never initial_suspend() const noexcept
#else
    [[nodiscard]] std::experimental::suspend_never initial_suspend() const noexcept
#endif
    {
      return {};
    }

    template <typename TU>
    void return_value(TU&& retValue)
    {
      _promise.set_value(std::forward<TU>(retValue));
    }

#if defined(__cpp_impl_coroutine) && !defined(__clang__)
    [[nodiscard]] std::suspend_never final_suspend() const noexcept
#else
    [[nodiscard]] std::experimental::suspend_never final_suspend() const noexcept
#endif
    {
      return {};
    }

    void unhandled_exception()
    {
      _promise.set_exception(std::current_exception());
    }
  };
}


#if defined(__cpp_impl_coroutine) && !defined(__clang__)
namespace std
#else
namespace std::experimental
#endif

{
  template <typename... ARGS>
  struct coroutine_traits<std::shared_future<void>, ARGS...>
  {
    using promise_type = RStein::AsyncCpp::AsyncPrimitives::SharedVoidFutureCoroutinePromise;
  };

  template <typename TR, typename... ARGS>
  struct coroutine_traits<std::shared_future<TR>, ARGS...>
  {
    using promise_type = RStein::AsyncCpp::AsyncPrimitives::SharedFutureTCoroutinePromise<TR>;
  };
}
