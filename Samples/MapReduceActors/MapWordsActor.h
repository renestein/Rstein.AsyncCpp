#pragma once
#include "../../RStein.AsyncCpp/Actors/ActorPolicy.h"

#include <xstring>


namespace Samples::MapReduceActors
{
  class ReduceWordProcessorActor;

  class MapWordsActor : public RStein::AsyncCpp::Actors::ActorPolicy
  {

  public:
    explicit MapWordsActor(ReduceWordProcessorActor* reduceWordProcessor);
    MapWordsActor(const MapWordsActor& other) = delete;
    MapWordsActor(MapWordsActor&& other) noexcept = delete;
    MapWordsActor& operator=(const MapWordsActor& other) = delete;
    MapWordsActor& operator=(MapWordsActor&& other) noexcept = delete;
    ~MapWordsActor() = default;
    void ProcessLines(std::vector<std::wstring> lines);

    protected:
    void OnCompleted() override;

    private:
    ReduceWordProcessorActor* _reduceProcessor;
    std::unordered_map<std::wstring, int> _wordsDictionary;
  };
}
