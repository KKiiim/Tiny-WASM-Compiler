#ifndef SRC_FRONTEND_FRONTEND_H
#define SRC_FRONTEND_FRONTEND_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "src/backend/aarch64Assembler.hpp"
#include "src/common/ModuleInfo.hpp"
#include "src/common/logger.hpp"
#include "src/common/stack.hpp"
#include "src/common/storage.hpp"
#include "src/frontend/byteCodeReader.hpp"
#include "src/frontend/operandManager.hpp"
#include "src/frontend/runtimeBlock.hpp"

class Frontend {
public:
  explicit Frontend(ModuleInfo &module, Stack &stack) : module_(module), stack_(stack) {
  }

  ExecutableMemory &startCompilation(std::string const &wasmPath);
  void logParsedInfo();
  inline uintptr_t getFunctionStartAddress(uint32_t const functionIndex) const {
    confirm(codeSectionParsed, "must");
    return funcAddrTable.get(functionIndex);
  }

  /// @note This is used to relpatch the branch instructions after all labels are registered
  /// @note Scope: within the same function
  class LabelManager final {
  public:
    explicit LabelManager(Assembler &as) : as_(as) {
    }
    struct BrInfo {
      bool isBrIf;
      uint32_t labelIndex;
      uint32_t BrInsStartAddress;
    };
    inline void registerBr(bool const isBrIf, uint32_t const labelIndex, uint32_t const BrInsStartAddress) {
      brIfInfos_.push_back({isBrIf, labelIndex, BrInsStartAddress});
    }
    inline uint32_t registerLabel() {
      labels_.push_back(0U); // placeholder for label address, need END to fill it
      return labels_.size(); // label start from @1. Align with wasm spec
    }
    inline void fillTargetJumpAddress(uint32_t const labelIndex, uint32_t const targetAddressLabelJump) {
      // labelIndex start from 1, align with wasm spec
      confirm(labelIndex <= labels_.size(), "must has registered");
      labels_[labelIndex - 1] = targetAddressLabelJump; // directly used for set_b_cond_off or set_b_off
    }

    void relpatchAllLabels();

  private:
    std::vector<uint32_t> labels_;  // labelIndex to labelAddress. As index start from 0
    std::vector<BrInfo> brIfInfos_; // need relpatch

    uint32_t labelIndexCounter_ = 0U; ///< Used to generate label index

    Assembler &as_;
  };

public:
  uintptr_t nativeToJitWrapper{};

private:
  void validateMagicNumber();
  void validateVersion();
  void parseTypeSection();
  void parseFunctionSection();
  void parseExportSection();
  void parseCodeSection();
  void parseElementSection();
  void parseTableSection();
  void parseGlobalSection();
  void parseImportSection();

  void parseMemorySection();
  void parseDataSection();

  void compile();
  void genWrapperFunction();

private:
  void emitWasmCall(Storage const callFuncIndex);
  void prepareCallParams(ModuleInfo::TypeInfo const &callType, OP &op);
  void recoveryCurrentFrameReg(ModuleInfo::FunctionInfo const &funcBody, ModuleInfo::TypeInfo const &funcType);
  void handleReturnValue(ModuleInfo::TypeInfo const &funcType, OP &op);

  void makeElementIndexToPureSignatureIndex();

  // Before and after call import function
  void spillRuntime();
  void recoverRuntime();

public:
  bool codeSectionParsed = false;

  static_assert(8U == sizeof(uintptr_t), "");
  RuntimeBlock<uintptr_t> funcAddrTable;                   ///< 8bytes array by function index to function abs address
  RuntimeBlock<uint32_t> elementIndexToFunctionIndex;      ///< 4bytes array by element index to function index
  RuntimeBlock<uint32_t> elementIndexToPureSignatureIndex; ///< 4bytes array by element index to pure function signature index

  RuntimeBlock<uint8_t> operandStack_; ///< JIT runtime stack for simulate WASM operand stack
  RuntimeBlock<uint64_t> globalMemory; ///< JIT runtime global memory. u32 and u64 are aligned to 8 bytes to store
  RuntimeBlock<uint8_t> linearMemory{config::MaxLinearMemoryPages};
  // FIXME(#88): don't need too large memory just for store linearMemoryByteSize(only 32bits)
  // Should serialize these blocks to an uniformly outputBinary.
  RuntimeBlock<uint32_t> linearMemoryByteSize;

private:
  BytecodeReader br_;
  Assembler as_;

private:
  ModuleInfo &module_;
  Stack &stack_;
};

#endif