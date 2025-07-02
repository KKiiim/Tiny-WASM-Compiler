#ifndef SRC_FRONTEND_FRONTEND_H
#define SRC_FRONTEND_FRONTEND_H

#include <cstdint>
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
  explicit Frontend(ModuleInfo &module, Stack &stack) : sTable_(as_), module_(module), stack_(stack) {
  }

  ExecutableMemory &startCompilation(std::string const &wasmPath);
  void logParsedInfo();
  inline uintptr_t getFunctionStartAddress(uint32_t const functionIndex) const {
    confirm(codeSectionParsed, "must");
    return sTable_.get(functionIndex);
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

  class SymbolTable final {
  public:
    explicit SymbolTable(Assembler &as) : as_(as) {
    }
    inline void addSymbol(uint32_t const functionIndex, uintptr_t const funcAbsAddress) {
      static_assert(sizeof(uintptr_t) == 8U, "must");
      LOG_DEBUG << "addSymbol: functionIndex=" << functionIndex << ", funcAbsAddress=" << std::hex << static_cast<uint64_t>(funcAbsAddress)
                << LOG_END;
      funcStartAbsAddressArray.set(functionIndex, funcAbsAddress);
    }

    // must used after code section parsed
    inline uintptr_t get(uint32_t const functionIndex) const {
      LOG_DEBUG << "get: functionIndex=" << functionIndex << ", funcAbsAddress=" << std::hex
                << static_cast<uint64_t>(funcStartAbsAddressArray.get<uintptr_t>(functionIndex)) << LOG_END;
      return funcStartAbsAddressArray.get<uintptr_t>(functionIndex);
    }
    inline uint64_t getTableStartAddress() const {
      return funcStartAbsAddressArray.getStartAddr();
    }

  private:
    RuntimeBlock funcStartAbsAddressArray; ///< 8bytes array by function index to function abs address

    Assembler &as_;
  };

private:
  void validateMagicNumber();
  void validateVersion();
  void parseTypeSection();
  void parseFunctionSection();
  void parseExportSection();
  void parseCodeSection();
  void parseNameSection();
  void parseElementSection();
  void parseTableSection();

  void compile();

private:
  void emitWasmCall(Storage const callFuncIndex);
  void prepareCallParams(ModuleInfo::TypeInfo const &callType, OP &op);

  void makeElementIndexToPureSignatureIndex();

public:
  SymbolTable sTable_;
  bool codeSectionParsed = false;

  RuntimeBlock elementIndexToFunctionIndex;      ///< 4bytes array by element index to function index
  RuntimeBlock elementIndexToPureSignatureIndex; ///< 4bytes array by element index to pure function signature index

private:
  BytecodeReader br_;
  Assembler as_;

private:
  ModuleInfo &module_;
  Stack &stack_;
};

#endif