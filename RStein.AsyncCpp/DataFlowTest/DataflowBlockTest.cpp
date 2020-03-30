#include "../AsyncPrimitives/FutureEx.h"
#include "../DataFlow/ActionBlock.h"
#include "../DataFlow/DataflowAsyncFactory.h"
#include "../DataFlow/DataFlowSyncFactory.h"
#include "../DataFlow/TransformBlock.h"

#include <gtest/gtest.h>
#include <vector>
using namespace std;

using namespace RStein::AsyncCpp::DataFlow;
using namespace RStein::AsyncCpp::AsyncPrimitives;
using namespace std;
namespace RStein::AsyncCpp::DataFlowTest
{
  TEST(DataFlowTest, WhenFlatDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 1000;
    auto transform1 = DataFlowSyncFactory::CreateTransformBlock<int, std::string, Detail::NoState>([](const int& item, Detail::NoState*& _)
    {
      auto message = "int: "  + to_string(item) + "\n";
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
    auto finalAction = DataFlowSyncFactory::CreateActionBlock<string, Detail::NoState>([&_processedItems](const string& item, Detail::NoState*&)
    {
      auto message = "Final action: " + item + "\n";
      cout << message;     
      _processedItems.push_back(item);

    });

    transform1->Then(transform2)
              ->Then(finalAction);
                         
    transform1->Start();
    for (int i = 0; i < EXPECTED_PROCESSED_ITEMS; ++i)
    {
      transform1->AcceptInputAsync(i).get();
    }

    transform1->Complete();
    finalAction->Completion().get();
    const auto processedItemsCount = _processedItems.size();

    ASSERT_EQ(EXPECTED_PROCESSED_ITEMS, processedItemsCount);
  }

   TEST(DataFlowTest, WhenDisjointCompleteDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 5;
    auto transform1 = std::make_shared<TransformBlock<int, int>>([](const int& item, Detail::NoState*& _)
    {
      auto message = "int: "  + to_string(item) + "\n";
      cout << message;
      return item;
    });

    auto transform2 = std::make_shared<TransformBlock<int, string>>([](const int& item, Detail::NoState*& _)
    {
      auto message = "Even number: " + to_string(item) + "\n";
      cout << message;
      return to_string(item);
      },
      [](const int& item){return item % 2 == 0;});

    auto transform3 = std::make_shared<TransformBlock<int, string>>([](const int& item, Detail::NoState*& _)
    {
      auto message = "Odd number: " + to_string(item) + "\n";
      cout << message;
      return to_string(item);
      },
      [](const int& item){return item % 2 != 0;});

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
      transform1->AcceptInputAsync(i).get();
    }

    transform1->Complete();
    finalAction->Completion().get();
    const auto processedItemsCount = _processedItems.size();

    ASSERT_EQ(EXPECTED_PROCESSED_ITEMS, processedItemsCount);
  }

  TEST(DataFlowTest, WhenAsyncFlatDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 1000;
    auto transform1 = DataFlowAsyncFactory::CreateTransformBlock<int, string, Detail::NoState>([](const int& item, Detail::NoState*& _)->shared_future<string>
    {
      auto message = "int: "  + to_string(item) + "\n";
      cout << message;
      co_await GetCompletedSharedFuture();
      co_return to_string(item);
    });

    auto transform2 = DataFlowAsyncFactory::CreateTransformBlock<string, string>([](const string& item)->shared_future<string>
    {
      auto message = "String transform: " + item + "\n";
      cout << message;
      co_await GetCompletedSharedFuture();
      co_return item + ": {string}";
    });

    vector<string> _processedItems{};
    auto finalAction=DataFlowAsyncFactory::CreateActionBlock<string>([&_processedItems](const string& item)->shared_future<void>
    {
      auto message = "Final action: " + item + "\n";
      cout << message;
      co_await GetCompletedSharedFuture();
      _processedItems.push_back(item);
    });

    transform1->Then(transform2)
              ->Then(finalAction);
                         
    transform1->Start();
    for (int i = 0; i < EXPECTED_PROCESSED_ITEMS; ++i)
    {
      transform1->AcceptInputAsync(i).get();
    }

    transform1->Complete();
    finalAction->Completion().get();
    const auto processedItemsCount = _processedItems.size();

    ASSERT_EQ(EXPECTED_PROCESSED_ITEMS, processedItemsCount);
  }

}