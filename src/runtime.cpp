#include <csetjmp>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "runtime.hpp"

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/common/util.hpp"

sigjmp_buf globalTrapEnv{};
static volatile uint32_t globalTrapcode{};

Runtime::Runtime(Compiler &compiler) : compiler_(compiler) {
  registerSignalHandler();
  globalTrapcode = 0U;
}
Runtime::~Runtime() {
  unregisterSignalHandler();
}

extern "C" void signal_handler(int sig, siginfo_t *info, void *ucontext) {
  if (sig != SIGTRAP || info->si_code != TRAP_BRKPT) {
    LOG_ERROR << "Unexpected signal: " << sig << " with code: " << info->si_code << LOG_END;
    return;
  }
  LOG_DEBUG << "Signal handler triggered with brkAddress: " << static_cast<uint64_t>(bit_cast<uintptr_t>(info->si_addr)) << LOG_END;
  ucontext_t const *const ctx = static_cast<ucontext_t *>(ucontext);
  // Default use R0 to store trap code
  globalTrapcode = ctx->uc_mcontext.regs[0];
  LOG_DEBUG << "Trap code: " << globalTrapcode << LOG_END;

  siglongjmp(globalTrapEnv, 0);
}

void Runtime::registerSignalHandler() const {
  struct sigaction sa {};
  sa.sa_sigaction = signal_handler;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGTRAP, &sa, nullptr) == -1) {
    LOG_ERROR << "Failed to register signal handler: " << strerror(errno) << LOG_END;
  }
}
void Runtime::unregisterSignalHandler() const {
  struct sigaction sa {};
  sa.sa_sigaction = nullptr;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGTRAP, &sa, nullptr) == -1) {
    LOG_ERROR << "Failed to register signal handler: " << strerror(errno) << LOG_END;
  }
}

std::string Runtime::getTrapCode() const {
  Trapcode const trapcode = static_cast<Trapcode>(globalTrapcode);
  if (trapcode == Trapcode::NONE) {
    throw std::runtime_error("trap code NONE is invalid");
  }
  auto const &v = trapcodeString.find(trapcode);
  if (v == trapcodeString.end()) {
    throw std::runtime_error("unregistered trap code " + std::to_string(static_cast<uint32_t>(trapcode)));
  }
  return v->second;
}
std::string Runtime::getTrapMessage() const {
  static const std::vector<std::string> trapMessages = {
      "No error",         // Trapcode 0, invalid trap code
      "Division by zero", // Trapcode 1
      "Integer overflow", // Trapcode 2
  };
  return trapMessages[globalTrapcode];
}