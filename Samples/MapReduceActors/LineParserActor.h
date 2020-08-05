#pragma once
#include "../../RStein.AsyncCpp/Actors/ActorPolicy.h"

#include <vector>
namespace Samples::MapReduceActors
{
  class MapWordsActor;
  class LineParserActor : public RStein::AsyncCpp::Actors::ActorPolicy
  {
  public:

    explicit LineParserActor(std::vector<MapWordsActor*> wordParsers, size_t batchSize);

    LineParserActor(const LineParserActor& other) = delete;
    LineParserActor(LineParserActor&& other) noexcept = delete;
    LineParserActor& operator=(const LineParserActor& other) = delete;
    LineParserActor& operator=(LineParserActor&& other) noexcept = delete;
    ~LineParserActor() = default;

    RStein::AsyncCpp::Tasks::Task<void> ProcessBook(std::string bookPath);

  protected:
    void OnCompleted() override;
  private:
    std::vector<MapWordsActor*> _wordParsers;
    size_t _batchSize;
  };
}
