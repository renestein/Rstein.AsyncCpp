#include <gtest/gtest.h>
#include "../DataFlow/TransformBlock.h"

using namespace RStein::AsyncCpp::DataFlow;
using namespace std;
namespace RStein::AsyncCpp::DataFlowTest
{
  TEST(DataFlowTest, WhenFlatDataflowThenAllInputsProcessed)
  {
    auto transform1 = std::make_shared<TransformBlock<int, std::string>>([](const int& item, Detail::NoState*& _){return string{};});
  }
}