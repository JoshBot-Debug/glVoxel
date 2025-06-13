#pragma once

#ifdef DEBUG

#include <iostream>

#define LOG(...) Log(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define LOG_BREAK_BEFORE                                                       \
  std::cout << std::endl                                                       \
            << "-------------------------------------------------------------" \
               "---------------------------"                                   \
            << std::endl
#define LOG_BREAK_AFTER                                                        \
  std::cout << "-------------------------------------------------------------" \
               "---------------------------"                                   \
            << std::endl

template <typename... Args>
void Log(const char *file, int line, const char *functionName,
         const Args &...args) {
  std::cout << "LOG " << file << ":" << line << " (" << functionName << "):";
  ((std::cout << " " << args), ...);
  std::cout << std::endl;
}

#else
#define LOG(...)
#define LOG_BREAK_BEFORE
#define LOG_BREAK_AFTER
#endif