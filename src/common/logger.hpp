#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

///< Log Wrapper
constexpr const char *ConsoleRed = "\033[31m";
constexpr const char *ConsoleGreen = "\033[32m";
constexpr const char *ConsoleYellow = "\033[33m";
constexpr const char *ConsoleWhite = "\033[37m";
constexpr const char *ConsolePurple = "\033[35m";
constexpr const char *ConsoleNone = "\033[0m";
constexpr const char *ConsoleGray = "\033[90m";

#define DISABLE_DEBUG_LOGS 1

#if DISABLE_DEBUG_LOGS
// clang-format off
#define LOG_DEBUG if(false) std::cout << ConsoleGray
// clang-format on
#else
#define LOG_DEBUG std::cout << ConsoleGray
#endif

#define LOG_INFO std::cout << ConsoleNone
#define LOG_GREEN std::cout << ConsoleGreen
#define LOG_YELLOW std::cout << ConsoleYellow
#define LOG_ERROR std::cerr << ConsoleRed

#define LOG_END std::endl << ConsoleNone

///< Assert Wrapper
#if defined(__linux__)
#define confirm(check, msg)                                                                                                                          \
  if (!(check)) {                                                                                                                                    \
    LOG_ERROR << "assert failed in " << __FILE__ << ":" << __LINE__ << ": " << ConsoleNone << (msg) << LOG_END;                                      \
    abort();                                                                                                                                         \
  }
#else
#define cdc_assert(check, msg)
#endif

#endif