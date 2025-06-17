#pragma once

#ifdef DEBUG

#include <chrono>
#include <fstream>
#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <thread>

template <typename... Args>
inline void Log(const char *file, int line, const char *functionName,
                const Args &...args) {
  std::cout << "LOG " << file << ":" << line << " (" << functionName << "):";
  ((std::cout << " " << args), ...);
  std::cout << std::endl;
}

inline void LogIVec3(const char *file, int line, const char *functionName,
                     const std::string &name, const glm::ivec3 &position) {

  Log(file, line, functionName, name, "(", position.x, ",", position.y, ",",
      position.z, ")");
}

template <typename... Args>
inline void LogToFile(const char *file, int line, const char *functionName,
                      const std::string &outputFile, const Args &...args) {
  std::ofstream ofs(outputFile, std::ios::app); // append mode
  if (!ofs.is_open())
    return;

  ofs << "LOG " << file << ":" << line << " (" << functionName << "):";
  ((ofs << " " << args), ...);
  ofs << std::endl;
}

inline void Benchmark(const char *file, int line, const char *functionName,
                      const std::function<void()> &func, int iterations) {

  for (int i = 0; i < 50; ++i)
    func();

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; ++i)
    func();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;

  Log(file, line, functionName, "Took:", elapsed.count() / iterations,
      "ms (average) over", iterations, "iterations");
}

inline void EndTimer(const char *file, int line, const char *functionName,
                     std::chrono::_V2::system_clock::time_point startTime) {
  auto end = std::chrono::high_resolution_clock::now(); // End timing
  std::chrono::duration<double, std::milli> duration = end - startTime;
  Log(file, line, functionName, duration.count(), "ms");
}

#define LOG(...) Log(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define BENCHMARK(...) Benchmark(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define START_TIMER std::chrono::high_resolution_clock::now()
#define END_TIMER(...) EndTimer(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_IVEC3(...) LogIVec3(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_TO_FILE(outputFile, ...)                                           \
  LogToFile(__FILE__, __LINE__, __func__, outputFile, __VA_ARGS__)

#else
#define LOG(...)
#define BENCHMARK(...)
#define START_TIMER 0
#define END_TIMER(...)
#define LOG_IVEC3(...)
#define LOG_TO_FILE(...)
#endif