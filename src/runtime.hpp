#ifndef SRC_COMMON_EXCEPTION_HPP
#define SRC_COMMON_EXCEPTION_HPP

#include <csetjmp>
#include <csignal>
#include <cstdint>
#include <string>
#include <type_traits>

#include "src/compiler.hpp"

extern sigjmp_buf globalTrapEnv;

class Runtime {
public:
  explicit Runtime(Compiler &compiler) : compiler_(compiler) {
  }

  void registerSignalHandler() const;

  std::string getTrapCode() const;
  std::string getTrapMessage() const;

  struct CallReturn {
    bool hasTrapped;
    uint64_t returnValue; // max support 64 bits number
  };

  template <typename T, typename... Args> CallReturn callByName(std::string const &funcName, std::string const &signature, Args &&...args) {
    CallReturn ret{};

    int status = sigsetjmp(globalTrapEnv, 0);
    if (status == 0) {
      ///< Normal call

      ///< if-constexpr only supported after C++17
      if constexpr (!std::is_void<T>::value) {
        // if (signature[0] != '(') {
        T const v = compiler_.singleCallByName<T>(funcName, signature, std::forward<Args>(args)...);
        ret.returnValue = static_cast<uint64_t>(v);
      } else {
        compiler_.singleCallByName<void>(funcName, signature, std::forward<Args>(args)...);
        ret.returnValue = 0; // No return value for void functions
      }
    }

    ret.hasTrapped = (status != 0);
    return ret;
  }

private:
  Compiler &compiler_;
};

#endif // SRC_COMMON_EXCEPTION_HPP