#include "LineParserActor.h"

#include "MapWordsActor.h"



#include <fstream>
#include <iosfwd>
#include <vector>

using namespace RStein::AsyncCpp::Actors;
using namespace std;

namespace Samples::MapReduceActors
{
  LineParserActor::LineParserActor(std::vector<MapWordsActor*> wordParsers,
                                   size_t batchSize): _wordParsers(std::move(wordParsers)),
                                                        _batchSize(batchSize)
  {
  }

  RStein::AsyncCpp::Tasks::Task<void> LineParserActor::ProcessBook(std::string bookPath)
  {
    return ScheduleFunction([bookPath=std::move(bookPath), this]
    {
      const size_t parserCount = _wordParsers.size();
      size_t parserIndex  = 0;
      wifstream stream{bookPath};
      wstring line;
      vector<wstring> linesBatch{};
      linesBatch.reserve(_batchSize);
      while(getline(stream, line))
      {
        if (line.empty())
        {
          continue;
        }

        linesBatch.push_back(line);

        if (linesBatch.size() < _batchSize)
        {
          continue;
        }
       _wordParsers[parserIndex % parserCount]->ProcessLines(std::move(linesBatch));
        linesBatch = vector<wstring>{};
        linesBatch.reserve(_batchSize);
        parserIndex++;
        if (parserIndex == std::numeric_limits<size_t>::max())
        {
          parserIndex = 0;
        }
      }
    });
  }

  void LineParserActor::OnCompleted()
  {
    for(auto processor : _wordParsers)
    {
      processor->Complete();
    }
  }
}
