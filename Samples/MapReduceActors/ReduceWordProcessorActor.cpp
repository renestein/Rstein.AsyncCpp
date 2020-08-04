#include "ReduceWordProcessorActor.h"

#include "WriteTopWordsActor.h"

#include <vector>
#include <algorithm>
using namespace std;
namespace Samples::MapReduceActors
{
  ReduceWordProcessorActor::ReduceWordProcessorActor(WriteTopWordsActor* topWordsActor, unsigned int numberOfWordMappers) : _aggregatedWords{},
    _writeTopWordsActor{ topWordsActor },
    _completedCallsRequired{ numberOfWordMappers }

  {
  }

  void ReduceWordProcessorActor::AggregateWords(std::unordered_map<std::wstring, int> words)
  {
    ScheduleFunction([this, words = std::move(words)]()
    {
      for (const auto& [word, frequency] : words)
      {
        _aggregatedWords[word] += frequency;
      }
    });
  }

  void ReduceWordProcessorActor::OnCompleted()
  {
    vector<std::pair<std::wstring, int>> wordsFrequency{ _aggregatedWords.size() };
    copy(_aggregatedWords.begin(), _aggregatedWords.end(), wordsFrequency.begin());
    sort(wordsFrequency.begin(), wordsFrequency.end(), [](const auto& firstPair, const auto& secondPair) {return firstPair.second > secondPair.second; });
    _writeTopWordsActor->ProcessAggregatedWords(wordsFrequency);
    _writeTopWordsActor->Complete();
  }

  bool ReduceWordProcessorActor::CanCompleteNow()
  {
    auto completedCalls = --_completedCallsRequired;
    return completedCalls == 0;
  }
}
