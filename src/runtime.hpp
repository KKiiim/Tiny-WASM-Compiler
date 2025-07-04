#ifndef SRC_COMMON_EXCEPTION_HPP
#define SRC_COMMON_EXCEPTION_HPP

#include <csetjmp>
#include <csignal>
#include <cstdint>
#include <string>
#include <type_traits>

#include "src/common/ModuleInfo.hpp"
#include "src/common/logger.hpp"
#include "src/compiler.hpp"

extern sigjmp_buf globalTrapEnv;

class Runtime {
public:
  explicit Runtime(Compiler &compiler);
  ~Runtime();

  Runtime(const Runtime &) = delete;
  Runtime &operator=(const Runtime &) = delete;
  Runtime(Runtime &&) = delete;
  Runtime &operator=(Runtime &&) = delete;

  std::string getTrapCode() const;
  std::string getTrapMessage() const;

  struct CallReturn {
    bool hasTrapped{false};
    uint64_t returnValue{};
  };

  template <typename TRet, typename... Args> CallReturn callByName(std::string const &funcName, std::string const &signature, Args &&...args) {
    ModuleInfo const &moduleInfo = compiler_.getModuleInfo();
    ///< Get function index
    auto const &funcIndex = moduleInfo.exportFuncNameToIndex_.find(funcName);
    if (funcIndex == moduleInfo.exportFuncNameToIndex_.end()) {
      throw std::runtime_error("Function not found: " + funcName);
    }
    uint32_t const functionIndex = funcIndex->second;
    if (!moduleInfo.validateSignature(functionIndex, signature)) {
      throw std::runtime_error("Signature validation failed for function: " + funcName);
    }

    CallReturn ret{};

    // set nonzero savemask for saving the signal mask(set in Runtime)
    int status = sigsetjmp(globalTrapEnv, 1); // NOLINT(misc-const-correctness)
    LOG_DEBUG << "status=" << status << LOG_END;
    if (status == 0) {
      ///< Normal call
      initialize();
      ///< if-constexpr only supported after C++17
      if constexpr (!std::is_void<TRet>::value) {
        // if (signature[0] != '(') {
        TRet const v = compiler_.singleCallByIndex<TRet>(functionIndex, std::forward<Args>(args)...);
        ret.returnValue = static_cast<uint64_t>(v);
      } else {
        compiler_.singleCallByIndex<void>(functionIndex, std::forward<Args>(args)...);
        ret.returnValue = 0; // No return value for void functions
      }
    }

    ret.hasTrapped = (status != 0);
    LOG_DEBUG << "callbyname return" << LOG_END;
    return ret;
  }

private:
  void initialize();
  void registerSignalHandler() const;
  void unregisterSignalHandler() const;

private:
  Compiler &compiler_;
  RuntimeBlock<uint8_t> operandStack_; ///< JIT runtime stack for simulate WASM operand stack
};

#endif // SRC_COMMON_EXCEPTION_HPP