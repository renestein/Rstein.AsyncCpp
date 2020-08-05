#pragma once
#include "../../RStein.AsyncCpp/Actors/ActorPolicy.h"

#include <map>

namespace Samples::MapReduceActors
{
  class WriteTopWordsActor;

  class ReduceWordProcessorActor : public RStein::AsyncCpp::Actors::ActorPolicy
  {
  public:

    explicit ReduceWordProcessorActor(WriteTopWordsActor* topWordsActor, unsigned int numberOfWordMappers);
    ReduceWordProcessorActor(const ReduceWordProcessorActor& other) = delete;
    ReduceWordProcessorActor(ReduceWordProcessorActor&& other) noexcept = delete;
    ReduceWordProcessorActor& operator=(const ReduceWordProcessorActor& other) = delete;
    ReduceWordProcessorActor& operator=(ReduceWordProcessorActor&& other) noexcept = delete;
    ~ReduceWordProcessorActor() = default;
    void AggregateWords(std::unordered_map<std::wstring, int> words);

  protected:
    void OnCompleted() override;
    bool CanCompleteNow() override;
  private:
    std::unordered_map<std::wstring, int> _aggregatedWords;
    WriteTopWordsActor* _writeTopWordsActor;
    std::atomic<unsigned int> _completedCallsRequired;
  };
}
