#include "WriteTopWordsActor.h"

#include <iostream>
using namespace std;
namespace Samples::MapReduceActors
{
  void WriteTopWordsActor::ProcessAggregatedWords(std::vector<std::pair<std::wstring, int>> aggregatedWords)
  {
    ScheduleFunction([this, aggregatedWords=std::move(aggregatedWords)]()
      {
        const int MAX_WORDS = 50;
        auto index = 0;
        for (const auto& [word, frequency] : aggregatedWords)
        {
          ++index;
          wcout << index << ".\t" << word << " : " << frequency << endl;
          if (index == MAX_WORDS)
          {
            return;
          }
        }
      });
  }

  void WriteTopWordsActor::OnCompleted()
  {
  }
}
