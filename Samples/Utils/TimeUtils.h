#pragma once
#include <chrono>
#include <iostream>

template <typename TFunc>
void MeasureTime(TFunc func)
{
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  func();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout << "Elapsed time(ms):" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;
  std::cout << "Elapsed time(s):" << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << std::endl;

}
