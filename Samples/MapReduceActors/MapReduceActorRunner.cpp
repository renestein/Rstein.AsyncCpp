#include "MapReduceActorRunner.h"



#include "LineParserActor.h"
#include "MapWordsActor.h"
#include "ReduceWordProcessorActor.h"
#include "WriteTopWordsActor.h"
#include "../../RStein.AsyncCpp/Actors/SimpleActor.h"
#include "../Utils/TimeUtils.h"


#include <vector>
#include <memory>
#include <iostream>
using namespace RStein::AsyncCpp::Actors;
using namespace std;
namespace Samples::MapReduceActors
{
  void MapReduceActorRunner::Run()
  {
    const size_t LINE_BATCH_SIZE = 1000;

    MeasureTime([LINE_BATCH_SIZE] {
      cout << "Creating actors..." << endl;
      auto numberOfWordsMapActors = thread::hardware_concurrency();
      WriteTopWordsActor topWordsActor{};
      ReduceWordProcessorActor reduceWordProcessorActor{&topWordsActor, numberOfWordsMapActors};
      vector<unique_ptr<MapWordsActor>> mapWordsActors{};
      for (unsigned i = 0; i < numberOfWordsMapActors; i++)
      {
        mapWordsActors.push_back(make_unique<MapWordsActor>(&reduceWordProcessorActor));
      }
      vector<MapWordsActor*> rawMapWordsActors;

      transform(mapWordsActors.begin(), mapWordsActors.end(), back_inserter(rawMapWordsActors), [](const std::unique_ptr<MapWordsActor>& wordParserActor)
      {
          return wordParserActor.get();
      });

      LineParserActor lineParserActor{rawMapWordsActors, LINE_BATCH_SIZE};
      cout << "Done." << endl;
      cout << "Processing book Shakespeare.txt..." << endl;
      //TODO: Change path to the file.
      lineParserActor.ProcessBook("c:\\Repositories_Git\\RStein.AsyncCpp\\Samples\\MapReduceActors\\Shakespeare.txt").Wait();
      lineParserActor.Complete();
      topWordsActor.Completion().Wait();
      cout << "Done." << endl;
      });

    cin.get();

  }
}
