#pragma once

// #ifdef DEBUG

#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <thread>

#define LOG(...) Log(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define BENCHMARK(...) Benchmark(__VA_ARGS__)
#define LOG_BREAK_BEFORE std::cout << std::endl \
                                   << "----------------------------------------------------------------------------------------" << std::endl
#define LOG_BREAK_AFTER std::cout << "----------------------------------------------------------------------------------------" << std::endl

template <typename... Args>
void Log(const char *file, int line, const char *functionName, const Args &...args)
{
  std::cout
      << "LOG " << file << ":" << line << " (" << functionName << "):";
  ((std::cout << " " << args), ...);
  std::cout << std::endl;
}

inline void Benchmark(const std::string &functionName, const std::function<void()> &func, int iterations)
{
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; ++i)
    func();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;

  std::cout << functionName << " took " << elapsed.count() / iterations << " ms (average) over " << iterations << " iterations.\n";
}

// #else
// #define LOG(...)
// #define BENCHMARK(...)
// #define LOG_BREAK_BEFORE
// #define LOG_BREAK_AFTER
// #endif