#include "MapWordsActor.h"

#include "ReduceWordProcessorActor.h"


#include <sstream>
#include <algorithm>
#include <cwctype>
using namespace std;
namespace Samples::MapReduceActors
{
  MapWordsActor::MapWordsActor(ReduceWordProcessorActor* reduceWordProcessor) : ActorPolicy(),
    _reduceProcessor{ reduceWordProcessor },
    _wordsDictionary{}
  {
  }

  void MapWordsActor::ProcessLines(std::vector<std::wstring> lines)
  {
    ScheduleFunction([this, lines = std::move(lines)]
      {
            for (auto& line : lines)
            {
              std::wistringstream wss{line};
              for (std::wstring word; wss >> word;)
              {
                word.erase(remove_if(word.begin(), word.end(), [](auto c) { return iswpunct(c) || iswspace(c); }), word.end());
                transform(word.begin(), word.end(), word.begin(), towlower);
                _wordsDictionary[word]++;
              }
            }
      });
  }

  void MapWordsActor::OnCompleted()
  {
    _reduceProcessor->AggregateWords(_wordsDictionary);
    _reduceProcessor->Complete();
  }
}
