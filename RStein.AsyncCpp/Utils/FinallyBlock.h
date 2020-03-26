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
      explicit FinallyBlock(std::function<void (void)> function) : m_function(std::move(function))
      {
      }

      explicit FinallyBlock(const DisposablePtr& disposable) : m_function([disposable]
      {
        if (disposable)
          disposable->Dispose();
      })
      {
      }
      
      ~FinallyBlock(void)
      {
        m_function();
      }

    private:
      std::function<void (void)> m_function;
    };
}
