#pragma once
#include <future>

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
      std::promise<void> _promise;
       _promise.set_value();
      auto _completedFuture = _promise.get_future().share();
       return _completedFuture;
    }
}
