#pragma once
#include <functional>
#include <utility>
#include "Disposable.h"

namespace RStein::Utils
{
    class FinallyBlock
    {
    public:

      FinallyBlock(const FinallyBlock& other) = delete;
      FinallyBlock(FinallyBlock&& other) = delete;
      FinallyBlock& operator=(const FinallyBlock& other) = delete;
      FinallyBlock& operator=(FinallyBlock&& other) = delete;
      //TODO: Resolve ambiguous call - VS 2010?
      explicit FinallyBlock(std::function<void (void)> function) : _function(std::move(function))
      {
      }

      explicit FinallyBlock(const DisposablePtr& disposable) : _function([disposable]
                                                              {
                                                                if (disposable)
                                                                {
                                                                  disposable->Dispose();
                                                                }
                                                              })
      {

      }
      
      ~FinallyBlock(void)
      {
        _function();
      }

    private:
      std::function<void (void)> _function;
    };
}
