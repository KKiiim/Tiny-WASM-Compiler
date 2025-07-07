#include <csetjmp>
#include <cstdint>
#include <cstring>

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
  if (sig == SIGTRAP && info->si_code == TRAP_BRKPT) {
    LOG_DEBUG << "Signal handler triggered with brkAddress: " << static_cast<uint64_t>(bit_cast<uintptr_t>(info->si_addr)) << LOG_END;
    ucontext_t const *const ctx = static_cast<ucontext_t *>(ucontext);
    // Default use R0 to store trap code
    globalTrapcode = ctx->uc_mcontext.regs[0];
    siglongjmp(globalTrapEnv, 0);
  } else {
    globalTrapcode = static_cast<uint32_t>(Trapcode::NONE);
    LOG_ERROR << "Unexpected signal: " << sig << " with code: " << info->si_code << LOG_END;
    std::terminate(); // Cleanly terminate the program for unexpected signals
  }
}

void Runtime::registerSignalHandler() const {
  struct sigaction sa {};
  sa.sa_sigaction = signal_handler;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGTRAP, &sa, nullptr) == -1) {
    LOG_ERROR << "Failed to register SIGTRAP: " << strerror(errno) << LOG_END;
  }
}
void Runtime::unregisterSignalHandler() const {
  struct sigaction sa {};
  sa.sa_sigaction = nullptr;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGTRAP, &sa, nullptr) == -1) {
    LOG_ERROR << "Failed to unregister SIGTRAP: " << strerror(errno) << LOG_END;
  }
}

Trapcode Runtime::getTrapCode() const {
  return static_cast<Trapcode>(globalTrapcode);
}