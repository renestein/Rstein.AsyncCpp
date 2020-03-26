#include <gtest/gtest.h>
#include "../DataFlow/TransformBlock.h"

using namespace RStein::AsyncCpp::DataFlow;
using namespace std;
namespace RStein::AsyncCpp::DataFlowTest
{
  TEST(DataFlowTest, WhenFlatDataflowThenAllInputsProcessed)
  {
    auto transform1 = std::make_shared<TransformBlock<int, std::string>>([](const int& item, Detail::NoState*& _)
    {
      //auto message = "int: "  + to_string(item) + "\n";
      //cout << message;
      return to_string(item);
    });

    auto transform2 = std::make_shared<TransformBlock<std::string, std::string>>([](const string& item, Detail::NoState*& _)
    {
     // auto message = "String transform: " + item + "\n";
     // cout << message;
      return item;
    });

    transform1->ConnectTo(transform2);
    transform1->Start();
    for (int i = 0; i < 1000000; ++i)
    {
      transform1->AcceptInputAsync(i).get();
    }

    transform1->Complete();
    transform2->Completion().get();
    SUCCEED();
  }
}