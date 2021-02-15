#include "../../RStein.AsyncCpp/AsyncPrimitives/FutureEx.h"
#include "../../RStein.AsyncCpp/DataFlow/ActionBlock.h"
#include "../../RStein.AsyncCpp/DataFlow/DataflowAsyncFactory.h"
#include "../../RStein.AsyncCpp/DataFlow/DataFlowSyncFactory.h"
#include "../../RStein.AsyncCpp/DataFlow/TransformBlock.h"
#include "../../RStein.AsyncCpp/Tasks/Task.h"
#include "../../RStein.AsyncCpp/Tasks/TaskCombinators.h"
#include "../../RStein.AsyncCpp/Tasks/TaskCompletionSource.h"
#include <gtest/gtest.h>
#include <vector>
using namespace std;

using namespace RStein::AsyncCpp::DataFlow;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace std;

namespace RStein::AsyncCpp::DataFlowTest
{
  class DataFlowTest : public testing::Test
  {
  public:
    Tasks::Task<size_t> WhenAsyncFlatDataflowThenAllInputsProcessedImpl(int processItemsCount) const
    {
      //Create TransformBlock. As the name of the block suggests, TransformBlock transforms input to output.
      //Following block transforms int to string.
      auto transform1 = DataFlowAsyncFactory::CreateTransformBlock<int, string>([](const int& item)-> Tasks::Task<string>
                                                          {
                                                            auto message = "int: " + to_string(item) + "\n";
                                                            cout << message;
                                                            //await async operation returning standard shared_future.
                                                            co_await GetCompletedSharedFuture();
                                                            co_return to_string(item);
                                                          });

      //TransformBlock transforms a string to another string.
      auto transform2 = DataFlowAsyncFactory::CreateTransformBlock<string, string>([](const string& item)-> Tasks::Task<string>
                                                          {
                                                            auto message = "String transform: " + item + "\n";
                                                            cout << message;
                                                            //await async operation returning Task.
                                                            co_await Tasks::GetCompletedTask();
                                                            co_return item + ": {string}";
                                                          });

      //Create final (last) dataflow block. ActionBlock does not propagate output and usually performs some important "side effect".
      //For example: Save data to collection, send data to socket, write to log...
      //Following ActionBlock stores all strings which it has received from the previous block in the _processedItems collection.
      vector<string> _processedItems{};
      auto finalAction = DataFlowAsyncFactory::CreateActionBlock<string>([&_processedItems](const string& item)-> Tasks::Task<void>
                                                            {
                                                              auto message = "Final action: " + item + "\n";
                                                              cout << message;
                                                              //await async operation returning Task.
                                                              co_await Tasks::GetCompletedTask();
                                                              _processedItems.push_back(item);
                                                            });
      //Connect all dataflow nodes.
      transform1->Then(transform2)
                 ->Then(finalAction);

      //Start dataflow.
      transform1->Start();

      //Add input data to the first transform node.
      for (auto i = 0; i < processItemsCount; ++i)
      {
        co_await transform1->AcceptInputAsync(i);
      }

      //All input data are in the dataflow. Send notification that no more data will be added.
      //This does not mean that all data in the dataflow are processed!
      transform1->Complete();

      //Wait for completion. When finalAction (last block) completes, all data were processed.
      co_await finalAction->Completion();

      //_processedItems contains all transformed items.
      const auto processedItemsCount = _processedItems.size();

      co_return processedItemsCount;
    }

  Tasks::Task<size_t> WhenAsyncForkJoinDataflowThenAllInputsProcessedImpl(int inputItemsCount)
  {
      //Create TransformBlock. As the name of the block suggests, TransformBlock transforms input to output.
      //Following block transforms int to string.
      auto transform1 = DataFlowAsyncFactory::CreateTransformBlock<int, int>([](const int& item)-> Tasks::Task<int>
                                                            {
                                                              //Simulate work
                                                              co_await Tasks::GetCompletedTask();
                                                              auto message = "int: " + to_string(item) + "\n";
                                                              cout << message;
                                                              co_return item;
                                                            });

      //Fork dataflow (even numbers are processed in one TransformBlock, for odd numbers create another transformBlock)    
      auto transform2 =  DataFlowAsyncFactory::CreateTransformBlock<int, string>([](const int& item)->Tasks::Task<string>
                                                        {
                                                          //Simulate work
                                                          co_await Tasks::GetCompletedTask();
                                                          auto message = "Even number: " + to_string(item) + "\n";
                                                          cout << message;
                                                          co_return to_string(item);
                                                        },
                                                        //Accept only even numbers.
                                                        //Condition is evaluated for every input.
                                                        //If the condition evaluates to true, input is accepted; otherwise input is ignored.
                                                        [](const int& item)
                                                        {
                                                          return item % 2 == 0;
                                                        });

      auto transform3 = DataFlowAsyncFactory::CreateTransformBlock<int, string>([](const int& item)->Tasks::Task<string>
                                                      {
                                                         //Simulate work
                                                         co_await Tasks::GetCompletedTask();
                                                        auto message = "Odd number: " + to_string(item) + "\n";
                                                        cout << message;
                                                        co_return to_string(item);
                                                      },
                                                       //Accept only odd numbers.
                                                       //Condition is evaluated for every input.
                                                       //If the condition evaluates to true, input is accepted; otherwise input is ignored.
                                                      [](const int& item)
                                                      {
                                                        return item % 2 != 0;
                                                      });
      //End fork.

      vector<string> _processedItems{};
      auto finalAction = DataFlowSyncFactory::CreateActionBlock<string>([&_processedItems](const string& item)
                                                            {
                                                              auto message = "Final action: " + item + "\n";
                                                              cout << message;
                                                              _processedItems.push_back(item);
                                                            });
      //Fork
      transform1->ConnectTo(transform2);
      transform1->ConnectTo(transform3);
      //end fork

       //Join
      transform3->ConnectTo(finalAction);
      transform2->ConnectTo(finalAction);
      //End join

      //Start dataflow
      transform1->Start();

      //Add input data to the first block.
      for (int i = 0; i < inputItemsCount; ++i)
      {
        co_await transform1->AcceptInputAsync(i);
      }

      
      //All input data are in the dataflow. Send notification that no more data will be added.
      //This does not mean that all data in the dataflow are processed!
      transform1->Complete();
      //Wait for last block.
      co_await finalAction->Completion();
      const auto processedItemsCount = _processedItems.size();

      co_return processedItemsCount;    
    }
  };

  TEST_F(DataFlowTest, WhenFlatDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 1000;
    auto transform1 = DataFlowSyncFactory::CreateTransformBlock<
      int, std::string, Detail::NoState>([](const int& item, Detail::NoState*& _)
        {
          auto message = "int: " + to_string(item) + "\n";
          cout << message;
          return to_string(item);
        });

    auto transform2 = DataFlowSyncFactory::CreateTransformBlock<std::string, std::string>([](const string& item)
      {
        auto message = "String transform: " + item + "\n";
        cout << message;
        return item + ": {string}";
      });

                                                                                                               vector<string> _processedItems{};
    auto finalAction = DataFlowSyncFactory::CreateActionBlock<string, Detail::NoState>([&_processedItems
    ](const string& item,
      Detail::NoState*&)
      {
        auto message =
          "Final action: " + item +
          "\n";
        cout << message;
        _processedItems.
          push_back(item);
      });

    transform1->Then(transform2)
      ->Then(finalAction);

    transform1->Start();
    for (int i = 0; i < EXPECTED_PROCESSED_ITEMS; ++i)
    {
      transform1->AcceptInputAsync(i).Wait();
    }

    transform1->Complete();
    finalAction->Completion().Wait();
    const auto processedItemsCount = _processedItems.size();

    ASSERT_EQ(EXPECTED_PROCESSED_ITEMS, processedItemsCount);
  }

  TEST_F(DataFlowTest, WhenForkJoinDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 5;
    auto transform1 = std::make_shared<TransformBlock<int, int>>([](const int& item, Detail::NoState*& _)
                                                          {
                                                            auto message = "int: " + to_string(item) + "\n";
                                                            cout << message;
                                                            return item;
                                                          });

    auto transform2 = std::make_shared<TransformBlock<int, string>>([](const int& item, Detail::NoState*& _)
                                                      {
                                                        auto message = "Even number: " + to_string(item) + "\n";
                                                        cout << message;
                                                        return to_string(item);
                                                      },
                                                      [](const int& item)
                                                      {
                                                        return item % 2 == 0;
                                                      });

    auto transform3 = std::make_shared<TransformBlock<int, string>>([](const int& item, Detail::NoState*& _)
                                                    {
                                                      auto message = "Odd number: " + to_string(item) + "\n";
                                                      cout << message;
                                                      return to_string(item);
                                                    },[](const int& item)
                                                    {
                                                      return item % 2 != 0;
                                                    });

    vector<string> _processedItems{};
    auto finalAction = DataFlowSyncFactory::CreateActionBlock<string>([&_processedItems](const string& item)
                                                          {
                                                            auto message = "Final action: " + item + "\n";
                                                            cout << message;
                                                            _processedItems.push_back(item);
                                                          });

    transform1->ConnectTo(transform2);
    transform1->ConnectTo(transform3);
    transform3->ConnectTo(finalAction);
    transform2->ConnectTo(finalAction);

    transform1->Start();
    for (int i = 0; i < EXPECTED_PROCESSED_ITEMS; ++i)
    {
      transform1->AcceptInputAsync(i).Wait();
    }

    transform1->Complete();
    finalAction->Completion().Wait();
    const auto processedItemsCount = _processedItems.size();

    ASSERT_EQ(EXPECTED_PROCESSED_ITEMS, processedItemsCount);
  }

  TEST_F(DataFlowTest, WhenAsyncFlatDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 100;
    auto processedItemsCount = WhenAsyncFlatDataflowThenAllInputsProcessedImpl(EXPECTED_PROCESSED_ITEMS).Result();

    ASSERT_EQ(EXPECTED_PROCESSED_ITEMS, processedItemsCount);

  }
  
  TEST_F(DataFlowTest, WhenAsyncForkJoinDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 5;

    auto processedItems = WhenAsyncForkJoinDataflowThenAllInputsProcessedImpl(EXPECTED_PROCESSED_ITEMS).Result();

    ASSERT_EQ(EXPECTED_PROCESSED_ITEMS, processedItems);

  }

  TEST_F(DataFlowTest, WhenInputClassDoesNotHaveDefaultCtorThenCanBeProcessedInDataflow)
  {
    const int EXPECTED_VALUE = 101;
    class WithoutDefaultCtor
    {
      public:
        WithoutDefaultCtor(int value): _value(value)
        {
            
        }

        int Value() const
        {
          return _value;
        }

      private:
        int _value;
    };

    int processedValue  = 0;

    auto ac2  = DataFlowAsyncFactory::CreateActionBlock<WithoutDefaultCtor*>([&processedValue](WithoutDefaultCtor* const& source)-> Tasks::Task<void>
    {
      co_await Tasks::GetCompletedTask();
      processedValue = source->Value();
    
    });
    WithoutDefaultCtor wdc1 {EXPECTED_VALUE};
    ac2->Start();
    ac2->AcceptInputAsync(&wdc1);
    ac2->Complete();
    ac2->Completion().Wait();

    ASSERT_EQ(EXPECTED_VALUE, processedValue);
  }
}
