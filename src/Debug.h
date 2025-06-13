#pragma once

#ifdef DEBUG

#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#define LOG(...) Log(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define BENCHMARK(...) Benchmark(__VA_ARGS__)
#define LOG_BREAK_BEFORE                                                       \
  std::cout << std::endl                                                       \
            << "-------------------------------------------------------------" \
               "---------------------------"                                   \
            << std::endl
#define LOG_BREAK_AFTER                                                        \
  std::cout << "-------------------------------------------------------------" \
               "---------------------------"                                   \
            << std::endl

#define START_TIMER std::chrono::high_resolution_clock::now()
#define END_TIMER(...) EndTimer(__VA_ARGS__)

template <typename... Args>
inline void Log(const char *file, int line, const char *functionName,
                const Args &...args) {
  std::cout << "LOG " << file << ":" << line << " (" << functionName << "):";
  ((std::cout << " " << args), ...);
  std::cout << std::endl;
}

inline void Benchmark(const std::string &functionName,
                      const std::function<void()> &func, int iterations) {
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; ++i)
    func();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;

  std::cout << functionName << " took " << elapsed.count() / iterations
            << " ms (average) over " << iterations << " iterations.\n";
}

inline void EndTimer(std::chrono::_V2::system_clock::time_point startTime,
                     const std::string &name) {
  auto end = std::chrono::high_resolution_clock::now(); // End timing
  std::chrono::duration<double, std::milli> duration = end - startTime;
  std::cout << name << ": " << duration.count() << " ms\n";
}

#else
#define LOG(...)
#define BENCHMARK(...)
#define LOG_BREAK_BEFORE
#define LOG_BREAK_AFTER
#define START_TIMER 0
#define END_TIMER(...)
#endif