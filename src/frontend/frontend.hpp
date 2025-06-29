#ifndef SRC_FRONTEND_FRONTEND_H
#define SRC_FRONTEND_FRONTEND_H

#include <cstdint>
#include <string>
#include <vector>

#include "src/backend/aarch64Assembler.hpp"
#include "src/common/ModuleInfo.hpp"
#include "src/common/logger.hpp"
#include "src/common/operand_stack.hpp"
#include "src/common/stack.hpp"
#include "src/frontend/byteCodeReader.hpp"

class Frontend {
public:
  explicit Frontend(ModuleInfo &module, Stack &stack, OperandStack &operandStack)
      : sTable_(as_), module_(module), stack_(stack), operandStack_(operandStack) {
  }

  ExecutableMemory startCompilation(std::string const &wasmPath);
  void logParsedInfo();
  inline uintptr_t getFunctionStartAddress(uint32_t const functionIndex) const {
    return sTable_.get(functionIndex);
  }

  /// @note This is used to relpatch the branch instructions after all labels are registered
  /// @note Scope: within the same function
  class LabelManager {
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

  class SymbolTable {
  public:
    explicit SymbolTable(Assembler &as) : as_(as) {
    }
    inline void addSymbol(uint32_t const functionIndex, uintptr_t const funcStartAddr) {
      symbols_[functionIndex] = funcStartAddr;
    }
    inline void addBl(uint32_t const funcIndex, uintptr_t const blInstructionStartAddr) {
      blStartAddrs_.emplace_back(funcIndex, blInstructionStartAddr);
    }
    inline uintptr_t get(uint32_t const functionIndex) const {
      auto const &it = symbols_.find(functionIndex);
      confirm(it != symbols_.end(), "function index not found in symbol table");
      return it->second;
    }

    void relpatchAllSymbols(); // Relpatch all bl instructions to the correct function start address

  private:
    std::unordered_map<uint32_t, uintptr_t> symbols_;          ///< functionIndex to funcStart address mapping
    std::vector<std::pair<uint32_t, uintptr_t>> blStartAddrs_; ///< functionIndex and bl instruction address

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

  void compile();

private:
  void emitWasmCall(uint32_t const callFuncIndex);

private:
  BytecodeReader br_;
  Assembler as_;
  SymbolTable sTable_;

private:
  ModuleInfo &module_;
  Stack &stack_;
  OperandStack &operandStack_;
};

#endif