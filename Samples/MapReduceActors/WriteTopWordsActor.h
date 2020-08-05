#pragma once
#include "../../RStein.AsyncCpp/Actors/ActorPolicy.h"

#include <map>

namespace Samples::MapReduceActors
{
  class WriteTopWordsActor : public RStein::AsyncCpp::Actors::ActorPolicy
  {
  
  public:

    void ProcessAggregatedWords(std::vector<std::pair<std::wstring, int>> aggregatedWords);

    protected:
      void OnCompleted() override;

  };
}
