#include "../DataFlow/ActionBlock.h"
#include "../DataFlow/TransformBlock.h"

#include <gtest/gtest.h>
#include <vector>
using namespace std;

using namespace RStein::AsyncCpp::DataFlow;
using namespace std;
namespace RStein::AsyncCpp::DataFlowTest
{
  TEST(DataFlowTest, WhenFlatDataflowThenAllInputsProcessed)
  {
    const int EXPECTED_PROCESSED_ITEMS = 1000;
    auto transform1 = std::make_shared<TransformBlock<int, std::string>>([](const int& item, Detail::NoState*& _)
    {
      auto message = "int: "  + to_string(item) + "\n";
      cout << message;
      return to_string(item);
    });

    auto transform2 = std::make_shared<TransformBlock<std::string, std::string>>([](const string& item, Detail::NoState*& _)
    {
      auto message = "String transform: " + item + "\n";
      cout << message;
      return item + ": {string}";
    });

    vector<string> _processedItems{};
    auto finalAction = std::make_shared<ActionBlock<string>>([&_processedItems](const string& item, Detail::NoState*& )
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
}