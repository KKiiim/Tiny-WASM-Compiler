#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "frontend.hpp"
#include "operandManager.hpp"

#include "src/backend/aarch64_encoding.hpp"
#include "src/backend/relpatch.hpp"
#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/common/stack.hpp"
#include "src/common/wasm_type.hpp"

ExecutableMemory Frontend::startCompilation(std::string const &wasmPath) {
  br_.readWasmBinary(wasmPath);

  validateMagicNumber();
  validateVersion();
  LOG_DEBUG << "validate success" << LOG_END;

  while (br_.hasNextByte()) {
    SectionType const sectionType{br_.readByte<SectionType>()};
    uint32_t const sectionSize{br_.readLEB128<uint32_t>()}; // FIXME(): ignore invalid size check
    switch (sectionType) {
    case SectionType::CUSTOM:
      br_.jump(sectionSize);
      // TODO(): Not supported yet
      // parseNameSection();
      break;
    case SectionType::TYPE:
      parseTypeSection();
      break;
    case SectionType::IMPORT:
      throw std::runtime_error("SectionType::IMPORT unsupported");
      break;
    case SectionType::FUNCTION:
      parseFunctionSection();
      break;
    case SectionType::TABLE:
      throw std::runtime_error("SectionType::TABLE unsupported");
      break;
    case SectionType::MEMORY:
      throw std::runtime_error("SectionType::MEMORY unsupported");
      break;
    case SectionType::GLOBAL:
      throw std::runtime_error("SectionType::GLOBAL unsupported");
      break;
    case SectionType::EXPORT:
      parseExportSection();
      break;
    case SectionType::START:
      throw std::runtime_error("SectionType::START unsupported");
      break;
    case SectionType::ELEMENT:
      throw std::runtime_error("SectionType::ELEMENT unsupported");
      break;
    case SectionType::CODE:
      parseCodeSection();
      break;
    case SectionType::DATA:
      throw std::runtime_error("SectionType::DATA unsupported");
      break;
    case SectionType::DATA_COUNT:
      throw std::runtime_error("SectionType::DATA_COUNT unsupported");
      break;
    case SectionType::PLACEHOLDER:
      throw std::runtime_error("SectionType::PLACEHOLDER unsupported");
      break;
    }
  }

  if (br_.getBytesLeft() != 0U) {
    throw std::runtime_error("bytecode left length should be zero after parsing");
  }
  LOG_DEBUG << "parse wasm success" << LOG_END;

  // logParsedInfo();

  return as_.getExecutableMemory();
}

void Frontend::validateMagicNumber() {
  constexpr std::array<uint8_t, 5U> wasmBinaryMagic{{0U, 0x61U, 0x73U, 0x6DU, 0U}};
  static_assert((wasmBinaryMagic.size() - 1U) == 4U, "Binary magic length needs to be four");
  while (br_.getOffset() < (wasmBinaryMagic.size() - 1U)) {
    if (wasmBinaryMagic[br_.getOffset()] != br_.readByte<uint8_t>()) {
      throw std::runtime_error("Wrong_Wasm_magic_number");
    }
  }
}

void Frontend::validateVersion() {
  uint32_t const moduleWasmVersion{br_.readLEU32()};
  constexpr uint32_t supportedWasmVersion{1U};

  if (moduleWasmVersion != supportedWasmVersion) {
    throw std::runtime_error("Wasm_Version_not_supported");
  }
}
void Frontend::parseTypeSection() {
  uint32_t const typeNumbers{br_.readLEB128<uint32_t>()};
  uint32_t counter = 0U;
  while (counter < typeNumbers) {
    counter++;

    uint8_t const typeType = br_.readByte<uint8_t>();
    // Only function types supported
    if (typeType != 0x60U) {
      throw std::runtime_error("type_section_wrong_type");
    }

    std::vector<WasmType> paramInfos{};
    std::vector<WasmType> resultInfos{};

    uint32_t const paramsNum = br_.readLEB128<uint32_t>();
    uint32_t paramIndex = 0U;
    while (paramIndex++ < paramsNum) {
      paramInfos.push_back(br_.readByte<WasmType>());
    }
    confirm(paramsNum == paramInfos.size(), "must");

    uint32_t const resultNum = br_.readLEB128<uint32_t>();
    uint32_t resultIndex = 0U;
    while (resultIndex++ < resultNum) {
      resultInfos.push_back(br_.readByte<WasmType>());
    }
    confirm(resultNum == resultInfos.size(), "must");
    confirm(resultInfos.size() <= 1U, "only one result supported");

    module_.type_.push_back({paramInfos, resultInfos});
  }
}
void Frontend::parseFunctionSection() {
  uint32_t const funcNumbers{br_.readLEB128<uint32_t>()};
  uint32_t counter = 0U;
  while (counter < funcNumbers) {
    counter++;

    uint32_t const index = br_.readLEB128<uint32_t>();
    module_.func_.push_back({index});
  }
}
void Frontend::parseExportSection() {
  uint32_t const exportNumbers{br_.readLEB128<uint32_t>()};
  uint32_t counter = 0U;
  while (counter < exportNumbers) {
    counter++;

    uint32_t const stringLength = br_.readLEB128<uint32_t>();
    std::string exportName{};
    for (uint32_t i = 0; i < stringLength; i++) {
      exportName += br_.readByte<char>();
    }

    WasmImportExportType const type{br_.readByte<WasmImportExportType>()};
    uint32_t const index = br_.readLEB128<uint32_t>();

    confirm(type == WasmImportExportType::FUNC, "only function export supported yet");

    module_.export_.push_back({exportName, type, index});
    module_.exportFuncNameToIndex_[exportName] = index;
  }
}
void Frontend::parseCodeSection() {
  uint32_t const funcNumbers{br_.readLEB128<uint32_t>()};

  // parse each function body
  for (uint32_t currentFuncIndex = 0U; currentFuncIndex < funcNumbers; ++currentFuncIndex) {
    uint32_t const funcSignatureIndex{module_.func_[currentFuncIndex].signatureIndex};
    auto const &funcTypeInfo = module_.type_[funcSignatureIndex];

    ModuleInfo::FunctionInfo funcBody{};
    uint32_t const stackElementIndex = stack_.push(StackElement{ElementType::FUNC_START});
    funcBody.belongingBlockIndex = stackElementIndex;
    funcBody.startAddressOffset = as_.getCurrentOffset();
    funcBody.paramsNumber = funcTypeInfo.params.size();
    uint32_t const funcBodySize = br_.readLEB128<uint32_t>();
    funcBody.bodySize = funcBodySize;
    for (auto const &p : funcTypeInfo.params) {
      // init params as local
      funcBody.locals.push_back({true, UINT32_MAX, p});
    }

    uint32_t const preParseFuncBROffset = br_.getOffset();

    // Parse local type decl(same type local array)
    // (local i64 i32 i64) -> localDeclCount == 3. i64*1 i32*1 i64*1
    // (local i64 i32 i32) -> localDeclCount == 2. i64*1 i32*2
    // params not included
    uint32_t const localDeclCount = br_.readLEB128<uint32_t>();
    uint32_t localDeclIndex = 0U;

    ////// Temporary variables of CURRENT function
    ///< record local's memory addr offset from SP in this function
    ///< to handle operand variables
    // TODO(): should split to different class
    OP op{as_};
    LabelManager labelManager{as_};

    while (localDeclIndex++ < localDeclCount) {
      uint32_t const currentTypeLocalCount = br_.readLEB128<uint32_t>();
      WasmType const localType = br_.readByte<WasmType>();
      for (uint32_t i = 0U; i < currentTypeLocalCount; ++i) {
        funcBody.locals.push_back({false, op.add(localType), localType});
      }
    }
    // TODO(): other stack use excluding local
    uint32_t const stackUsage = op.getAlignedSize();
    LOG_DEBUG << "stackUsage = " << stackUsage << LOG_END;
    if (stackUsage != 0U) {
      as_.decreaseSPWithClean(stackUsage);
    }

    while (br_.getOffset() < (preParseFuncBROffset + funcBodySize)) {
      OPCode const opcode = br_.readByte<OPCode>();

      switch (opcode) {
      case OPCode::NOP: {
        break; ///< NOP, do nothing
      }
      case OPCode::RETURN: {
        if (funcTypeInfo.results.size() == 1U) {
          // function with return value
          confirm(stack_.top().isValue(), "should at least have one value element for return value");
          StackElement const retValue = stack_.top();
          confirm(toWasmType(retValue.elementType_) == funcTypeInfo.results[0], "validation stack top should be the return value type");

          bool const is64bit = funcTypeInfo.results[0] == WasmType::I64;
          // prepare return value
          op.subROP(is64bit);
          as_.ldr_base_off(REG::R0, REG::R28, 0U, is64bit);
        }
        if (stackUsage != 0U) {
          as_.inc_sp(stackUsage);
        }
        as_.ret();

        stack_.setCurrentFrameFrontBlocksUnreachable();
        break;
      }
      case OPCode::LOCAL_GET: {
        uint32_t const localIdx{br_.readLEB128<uint32_t>()};
        auto const &l = funcBody.locals[localIdx];
        bool const is64bit = l.type == WasmType::I64;
        stack_.push(StackElement{is64bit ? ElementType::I64 : ElementType::I32});
        if (l.isParam) {
          // param in register. Assumed params <= 8
          confirm(localIdx < funcTypeInfo.params.size(), "");
          op.get_r_param(localIdx, is64bit);
        } else {
          // local
          uint32_t const offset2SP = l.offset;
          op.get_ofsp_local(offset2SP, is64bit);
        }
        break;
      }
      case OPCode::LOCAL_SET:
      case OPCode::LOCAL_TEE: {
        uint32_t const localIdx{br_.readLEB128<uint32_t>()};
        auto const &l = funcBody.locals[localIdx];
        confirm(stack_.top().isI64() == (l.type == WasmType::I64), "must");
        bool const is64bit = (l.type == WasmType::I64);

        if (l.isParam) {
          confirm(localIdx < funcTypeInfo.params.size(), "");
          confirm(l.type == funcTypeInfo.params[localIdx], "");
          op.set_r_param(localIdx, is64bit, opcode == OPCode::LOCAL_TEE);
        } else {
          uint32_t const offset2SP = l.offset;
          op.set_ofsp_local(offset2SP, is64bit, opcode == OPCode::LOCAL_TEE);
        }

        // local.tee don't pop operand stack element
        if (opcode == OPCode::LOCAL_SET) {
          stack_.pop();
        }
        break;
      }
      case OPCode::I32_CONST: {
        uint32_t const v = bit_cast<uint32_t>(br_.readLEB128<int32_t>());
        stack_.push(StackElement{ElementType::I32});
        // Use W9 as scratch register
        as_.emit_mov_w_imm32(REG::R9, v);
        as_.str_base_off(ROP, REG::R9, 0U, false);
        as_.add_r_r_imm(ROP, ROP, 4U, true);
        break;
      }
      case OPCode::I64_CONST: {
        uint64_t const v = bit_cast<uint64_t>(br_.readLEB128<int64_t>());
        stack_.push(StackElement{ElementType::I64});
        // Use W9 as scratch register
        as_.emit_mov_x_imm64(REG::R9, v);
        as_.str_base_off(ROP, REG::R9, 0U, true);
        as_.add_r_r_imm(ROP, ROP, 8U, true);
        break;
      }
      case OPCode::I32_ADD:
      case OPCode::I64_ADD: {
        bool const is64bit = (opcode == OPCode::I64_ADD);

        StackElement const right = stack_.pop();
        StackElement const left = stack_.top();
        // don't pop again since the same value type result will be pushed
        confirm(right.isI64() == is64bit, "must");
        confirm(left.isI64() == is64bit, "must");

        // Use R9 as right value scratch register
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        // Use R10 as left value scratch register
        as_.ldr_base_off(REG::R10, ROP, 0U, is64bit);
        as_.add_r_r_shiftR(REG::R9, REG::R10, REG::R9, is64bit);

        // Store result to ROP
        as_.str_base_off(ROP, REG::R9, 0U, is64bit);
        op.addROP(is64bit);
        break;
      }
      case OPCode::I64_SUB:
      case OPCode::I32_SUB: {
        bool const is64bit = (opcode == OPCode::I64_SUB);

        StackElement const right = stack_.pop();
        StackElement const left = stack_.top();
        // don't pop again since the same value type result will be pushed
        confirm(right.isI64() == is64bit, "must");
        confirm(left.isI64() == is64bit, "must");

        // Use R9 as right value scratch register
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        // Use R10 as left value scratch register
        as_.ldr_base_off(REG::R10, ROP, 0U, is64bit);
        as_.sub_r_r_shiftR(REG::R9, REG::R10, REG::R9, is64bit);

        // Store result to ROP
        as_.str_base_off(ROP, REG::R9, 0U, is64bit);
        op.addROP(is64bit);
        break;
      }
      case OPCode::I64_MUL:
      case OPCode::I32_MUL: {
        bool const is64bit = (opcode == OPCode::I64_MUL);

        StackElement const right = stack_.pop();
        StackElement const left = stack_.top();
        // don't pop again since the same value type result will be pushed
        confirm(right.isI64() == is64bit, "must");
        confirm(left.isI64() == is64bit, "must");

        // Use R9 as right value scratch register
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        // Use R10 as left value scratch register
        as_.ldr_base_off(REG::R10, ROP, 0U, is64bit);
        as_.mul_r_r(REG::R9, REG::R10, REG::R9, is64bit);

        // Store result to ROP
        as_.str_base_off(ROP, REG::R9, 0U, is64bit);
        op.addROP(is64bit);
        break;
      }
      case OPCode::END: {
        StackElement const &lastControlFlowElement = stack_.lastControlFlowElement();

        switch (lastControlFlowElement.elementType_) {
        case ElementType::FUNC_START: {
          confirm(br_.getOffset() == (preParseFuncBROffset + funcBodySize), "");
          if (funcTypeInfo.results.size() == 1U) {
            // function with return value
            confirm(stack_.top().isValue(), "should at least have one value element for return value");
            StackElement const retValue = stack_.pop();
            confirm(toWasmType(retValue.elementType_) == funcTypeInfo.results[0], "validation stack top should be the return value type");

            ///< Function may have other values in stack, it's valid. Use the last value as return value
            stack_.popToLastControlFlowElement(); // including pop FUNC_START
            // FIXME(#59): should else pop redundant value in runtime operand stack
            stack_.push(retValue);

            bool const is64bit = funcTypeInfo.results[0] == WasmType::I64;
            // prepare return value
            op.subROP(is64bit);
            as_.ldr_base_off(REG::R0, REG::R28, 0U, is64bit);
          }
          if (stackUsage != 0U) {
            as_.inc_sp(stackUsage);
          }
          as_.ret();

          break;
        }
        case ElementType::IF: {
          // IF->B.c->TRUE_BLOCK->END->OTHER. Which means no ELSE branch

          ///< Link the B.c to OTHER(if false, jump to OTHER)
          uint32_t const branchInsPosOff = lastControlFlowElement.relpatchInsPos;
          uint32_t const afterIfOffset = as_.getCurrentOffset();
          // This offset pos pointed is behind the end of IF block(cond == false, no ELSE)
          confirm(afterIfOffset > branchInsPosOff, "for IF->END case, always high address at end");
          confirm((afterIfOffset - branchInsPosOff) % 4 == 0, "must 4 times, arm ins always 4 bytes");
          ///< B.cond. Its offset from the address of this instruction, in the range +/-1MB, is encoded as "imm19" times 4.
          int32_t const condOffset = static_cast<int32_t>((afterIfOffset - branchInsPosOff) / 4);
          as_.set_b_cond_off(branchInsPosOff, condOffset);

          ///< Check the return type of IF block
          WasmType const ifReturnType = lastControlFlowElement.returnType_;
          confirm(ifReturnType != WasmType::INVALID, "IF ELSE return type should be valid");
          if (ifReturnType != WasmType::TVOID) {
            confirm(ifReturnType == toWasmType(stack_.top().elementType_), "IF block return type should match the validation");
            StackElement const retValue = stack_.pop();
            confirm(&stack_.top() == &lastControlFlowElement, "IF block should not have other values exclude the return value");
            stack_.pop();          // pop IF
            stack_.push(retValue); // push the return value back to stack
          } else {
            // IF->END case, no return value
            confirm(&stack_.top() == &lastControlFlowElement, "IF block should be the last control flow element");
            stack_.pop(); // pop IF
          }
          break;
        }
        case ElementType::ELSE: {
          // IF->TRUE_BLOCK->B->ELSE->FALSE_BLOCK->END->OTHER

          ///< Link the B to OTHER(after exec TRUE_BLOCK, step the false part)
          // b ins position at the end of IF (cond == true) block
          uint32_t const branchInsPosOff = lastControlFlowElement.relpatchInsPos;
          uint32_t const afterELSEOffset = as_.getCurrentOffset();
          confirm(afterELSEOffset > branchInsPosOff, "for IF->ELSE->END case, always high address at end");
          confirm((afterELSEOffset - branchInsPosOff) % 4 == 0, "must 4 times, arm ins always 4 bytes");
          int32_t const condOffset = static_cast<int32_t>((afterELSEOffset - branchInsPosOff) / 4);
          as_.set_b_off(branchInsPosOff, condOffset);

          ///< Prepare to return
          WasmType const ifElseReturnType = lastControlFlowElement.returnType_; // IF and ELSE element both store the return type
          confirm(ifElseReturnType != WasmType::INVALID, "IF ELSE return type should be valid");
          if (ifElseReturnType != WasmType::TVOID) {
            // Has return value
            confirm(ifElseReturnType == toWasmType(stack_.top().elementType_), "IF ELSE return type should match the validation");
            StackElement const retValue = stack_.pop();
            confirm(&stack_.top() == &lastControlFlowElement, "ELSE block should not have other values exclude the return value");
            stack_.pop(); // pop ELSE
            StackElement const &shouldBeIf = stack_.lastControlFlowElement();
            confirm(shouldBeIf.elementType_ == ElementType::IF, "IF block should be the last control flow element");
            confirm(stack_.top().isI64() == retValue.isI64(), "IF TRUE branch else have the same type return value");
            stack_.pop(); // pop IF-TRUE branch return value
            confirm(&stack_.top() == &shouldBeIf, "must be IF");
            stack_.pop();          // pop IF
            stack_.push(retValue); // push the return value back to stack
          } else {
            // IF->ELSE->END case, no return value
            confirm(&stack_.top() == &lastControlFlowElement, "ELSE block should be the last control flow element");
            stack_.pop(); // pop ELSE
            confirm(stack_.top().elementType_ == ElementType::IF, "IF block should be the last control flow element");
            stack_.pop(); // pop IF
          }

          break;
        }
        case ElementType::BLOCK: {
          // BLOCK->END->OTHER
          WasmType const blockReturnType = lastControlFlowElement.returnType_;
          labelManager.fillTargetJumpAddress(lastControlFlowElement.labelIndex, as_.getCurrentOffset());

          if (lastControlFlowElement.isUnreachableBlock()) {
            stack_.popToLastControlFlowElement(); // including pop the Block
            if (blockReturnType != WasmType::TVOID) {
              ElementType const result = blockReturnType == WasmType::I64 ? ElementType::I64 : ElementType::I32;
              // push a dummy value to keep stack valid
              stack_.push(StackElement{result});
            }
          } else {
            // check block return
            if (blockReturnType != WasmType::TVOID) {
              // BLOCK has return value
              confirm(blockReturnType == toWasmType(stack_.top().elementType_), "BLOCK return type should match the validation");
              StackElement const retValue = stack_.pop();
              confirm(&stack_.top() == &lastControlFlowElement, "BLOCK should not have other values exclude the return value");
              stack_.pop();          // pop BLOCK
              stack_.push(retValue); // push the return value back to stack
            } else {
              // BLOCK->END case, no return value
              confirm(&stack_.top() == &lastControlFlowElement, "BLOCK should be the last control flow element");
              stack_.pop(); // pop BLOCK
            }
          }

          break;
        }
        default: {
          throw std::runtime_error("unexpected END opcode in function body");
        }
        }

        break; ///< END
      }
      case OPCode::IF: {
        ///< Two cases
        // IF->B.c->TRUE_BLOCK->B->ELSE->FALSE_BLOCK->END->OTHER
        // IF->B.c->TRUE_BLOCK->OTHER

        WasmType const returnType = br_.readByte<WasmType>();
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        confirm((returnType == WasmType::TVOID) || (returnType == WasmType::I32) || (returnType == WasmType::I64),
                "only i32, i64 or void supported for IF return type");
        StackElement ifElement{ElementType::IF};
        ifElement.returnType_ = returnType;

        // pop and get condition value in R9
        StackElement const condition = stack_.pop();
        confirm(condition.isValue() && (!condition.isI64()), "condition must be i32 value type");
        op.subROP(false);
        as_.ldr_base_off(REG::R9, ROP, 0U, false);

        as_.cmp_r_imm(REG::R9, 0U, false);

        ///< Need relocation patching for branch offset.
        uint32_t const positionOffsetOfConditionInstruction = as_.getCurrentOffset();
        ifElement.relpatchInsPos = positionOffsetOfConditionInstruction;
        ///< Prepare B.c
        ///< Will be link(set off) when trigger ELSE or END
        as_.prepare_b_cond(CC::EQ);

        stack_.push(ifElement);
        break;
      }
      case OPCode::ELSE: {
        // IF->B.c->TRUE_BLOCK->B->ELSE->FALSE_BLOCK->END->OTHER

        StackElement const &preIfElement = stack_.lastControlFlowElement();
        confirm(preIfElement.elementType_ == ElementType::IF, "");

        StackElement elseElement{ElementType::ELSE};
        elseElement.returnType_ = preIfElement.returnType_; // ELSE also store the return type for better validation

        ///< Prepare B
        // True branch of IF should jump to the END of ELSE branch code
        // This jump should emitted before ELSE branch code
        uint32_t const positionOffsetOfJumpInsStart = as_.getCurrentOffset();
        as_.prepare_b();
        elseElement.relpatchInsPos = positionOffsetOfJumpInsStart;

        ///< Link B.c to ELSE
        uint32_t const branchInsPosOff = preIfElement.relpatchInsPos;
        uint32_t const elseCodeStartOffset = as_.getCurrentOffset();
        // This offset pos pointed is behind the end of IF block(cond == false)
        // TODO(): if IF-TRUE branch is none, don't need to emit jump
        confirm(elseCodeStartOffset > branchInsPosOff, "for IF->ELSE case, always high address at end");
        confirm((elseCodeStartOffset - branchInsPosOff) % 4 == 0, "must 4 times, arm ins always 4 bytes");
        ///< B.cond. Its offset from the address of this instruction, in the range +/-1MB, is encoded as "imm19" times 4.
        int32_t const condOffset = static_cast<int32_t>((elseCodeStartOffset - branchInsPosOff) / 4);
        as_.set_b_cond_off(branchInsPosOff, condOffset);

        stack_.push(elseElement);
        break;
      }
      case OPCode::I32_DIV_S:
      case OPCode::I32_DIV_U:
      case OPCode::I64_DIV_S:
      case OPCode::I64_DIV_U: {
        bool const is64bit = (opcode == OPCode::I64_DIV_S || opcode == OPCode::I64_DIV_U);
        ///< Validation
        // pop the right value
        confirm(stack_.pop().isI64() == is64bit, "should be I64 value type");
        // check the left value, don't pop it since DIV will push the same type result
        confirm(stack_.top().isI64() == is64bit, "should be I64 value type");

        // get divisor in R9
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R9, ROP, 0, is64bit);
        as_.cmp_r_imm(REG::R9, 0U, is64bit);
        Relpatch const notDiv0 = as_.prepareJmp(CC::NE);
        as_.setTrap(Trapcode::DIV_0);
        notDiv0.linkToHere();
        ///< Divisor is not zero, continue to do division
        ///< If divisor is -1 and dividend is INT_MIN, it will trap with integer overflow
        // get dividend in R10
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R10, ROP, 0U, is64bit);

        // Is INT_MIN
        if (is64bit) {
          as_.emit_mov_x_imm64(REG::R11, 0x8000000000000000);
          as_.cmp_r_r(REG::R10, REG::R11, true);
        } else {
          as_.emit_mov_w_imm32(REG::R11, 0x80000000);
          as_.cmp_r_r(REG::R10, REG::R11, false);
        }
        Relpatch const notIntMin = as_.prepareJmp(CC::NE);

        // Is -1
        if (is64bit) {
          as_.emit_mov_x_imm64(REG::R11, static_cast<uint64_t>(-1));
          as_.cmp_r_r(REG::R9, REG::R11, true);
        } else {
          as_.emit_mov_w_imm32(REG::R11, static_cast<uint32_t>(-1));
          as_.cmp_r_r(REG::R9, REG::R11, false);
        }
        Relpatch const notMinusOne = as_.prepareJmp(CC::NE);
        as_.setTrap(Trapcode::Integer_overflow);

        notIntMin.linkToHere();
        notMinusOne.linkToHere();
        // safe division
        if (opcode == OPCode::I32_DIV_S || opcode == OPCode::I64_DIV_S) {
          as_.sdiv_r_r(REG::R9, REG::R10, REG::R9, is64bit);
        } else {
          as_.udiv_r_r(REG::R9, REG::R10, REG::R9, is64bit);
        }
        as_.str_base_off(ROP, REG::R9, 0U, is64bit);
        op.addROP(is64bit);

        break;
      }
      case OPCode::BLOCK: {
        WasmType const blockReturnType = br_.readByte<WasmType>();
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        confirm((blockReturnType == WasmType::TVOID) || (blockReturnType == WasmType::I32) || (blockReturnType == WasmType::I64),
                "only i32, i64 or void supported for BLOCK return type");
        StackElement blockElement{ElementType::BLOCK};
        blockElement.returnType_ = blockReturnType;
        blockElement.labelIndex = labelManager.registerLabel();
        stack_.push(blockElement);
        break;
      }
      case OPCode::BR: {
        uint32_t const breakDepth{br_.readLEB128<uint32_t>()};
        StackElement const targetElement = stack_.findTargetBlock(breakDepth);

        // validate the target block return type
        if (targetElement.returnType_ != WasmType::TVOID) {
          confirm(stack_.top().isValue(), "must value type");
          confirm(targetElement.returnType_ == toWasmType(stack_.top().elementType_), "target block return type should match the validation");
        }

        labelManager.registerBr(false, targetElement.labelIndex, as_.getCurrentOffset());
        as_.prepare_b();

        // Should then skip all until the next END
        // Current block has no more emit, so current block label can filled now
        // StackElement const currentBlock = stack_.findTargetBlock(0U);
        // labelManager.fillTargetJumpAddress(currentBlock.labelIndex, as_.getCurrentOffset());
        // TODO(): How to skip
        break;
      }
      case OPCode::BR_IF: {
        uint32_t const breakDepth{br_.readLEB128<uint32_t>()};
        StackElement const targetElement = stack_.findTargetBlock(breakDepth);
        // validate the condition value
        StackElement const condition = stack_.pop();
        confirm(condition.isValue() && (!condition.isI64()), "condition must be i32 value type");
        // pop the condition value at runtime
        op.subROP(false);
        as_.ldr_base_off(REG::R9, ROP, 0U, false);
        as_.cmp_r_imm(REG::R9, 0U, false);
        labelManager.registerBr(true, targetElement.labelIndex, as_.getCurrentOffset());
        as_.prepare_b_cond(CC::NE);

        // validate the target block return type
        if (targetElement.returnType_ != WasmType::TVOID) {
          confirm(stack_.top().isValue(), "must value type");
          confirm(targetElement.returnType_ == toWasmType(stack_.top().elementType_), "target block return type should match the validation");
        }

        break;
      }
      case OPCode::DROP: {
        confirm(stack_.top().isValue(), "must be value type");
        op.subROP(stack_.top().isI64()); // drop
        stack_.pop();
        break;
      }
      case OPCode::I32_CTZ:
      case OPCode::I64_CTZ: {
        bool const is64bit = (opcode == OPCode::I64_CTZ);
        confirm((stack_.top().isValue() && (stack_.top().isI64() == is64bit)), "must be value type");
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R9, ROP, 0U, is64bit);
        as_.rBits_r_r(REG::R9, REG::R9, is64bit);
        as_.clz_r_r(REG::R9, REG::R9, is64bit);
        as_.str_base_off(ROP, REG::R9, 0U, is64bit);
        op.addROP(is64bit);
        // don't pop stack, since the ctz result is the same type element
        break;
      }
      case OPCode::I64_LE_U:
      case OPCode::I32_LE_U: {
        bool const is64bit = (opcode == OPCode::I64_LE_U);
        StackElement const right = stack_.pop();
        StackElement const left = stack_.pop();
        confirm(right.isI64() == is64bit, "must be value type");
        confirm(left.isI64() == is64bit, "must be value type");
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        as_.ldr_base_off(REG::R10, ROP, 0U, is64bit);

        // prepare result as default true, unsigned left <= right
        REG const resultReg = REG::R11;
        as_.emit_mov_w_imm32(resultReg, 1U);

        as_.cmp_r_r(REG::R10, REG::R9, is64bit);
        Relpatch const unsignedLessOrSame = as_.prepareJmp(CC::LS);
        // not LS(unsigned)
        as_.emit_mov_w_imm32(resultReg, 0);
        unsignedLessOrSame.linkToHere();
        as_.str_base_off(ROP, resultReg, 0U, false);
        op.addROP(false);

        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::I64_EQZ:
      case OPCode::I32_EQZ: {
        bool const is64bit = (opcode == OPCode::I64_EQZ);
        confirm((stack_.top().isValue() && (stack_.top().isI64() == is64bit)), "must match value type");
        stack_.pop();

        op.subROP(is64bit);
        as_.ldr_base_off(REG::R9, ROP, 0U, is64bit);

        REG const resultReg = REG::R10;
        // prepare default not zero, set 0 (if-else downgraded to if)
        as_.emit_mov_w_imm32(resultReg, 0U);

        as_.cmp_r_imm(REG::R9, 0U, is64bit);
        Relpatch const notZero = as_.prepareJmp(CC::NE);
        // value is zero. Set true
        as_.emit_mov_w_imm32(resultReg, 1U);
        notZero.linkToHere();
        as_.str_base_off(ROP, resultReg, 0U, false);
        op.addROP(false);

        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::UNREACHABLE:
      case OPCode::LOOP:
      case OPCode::BR_TABLE:
      case OPCode::CALL:
      case OPCode::CALL_INDIRECT:
      case OPCode::REF_NULL:
      case OPCode::REF_IS_NULL:
      case OPCode::REF_FUNC:
      case OPCode::SELECT:
      case OPCode::SELECT_T:
      case OPCode::GLOBAL_GET:
      case OPCode::GLOBAL_SET:
      case OPCode::TABLE_GET:
      case OPCode::TABLE_SET:
      case OPCode::I32_LOAD:
      case OPCode::I64_LOAD:
      case OPCode::F32_LOAD:
      case OPCode::F64_LOAD:
      case OPCode::I32_LOAD8_S:
      case OPCode::I32_LOAD8_U:
      case OPCode::I32_LOAD16_S:
      case OPCode::I32_LOAD16_U:
      case OPCode::I64_LOAD8_S:
      case OPCode::I64_LOAD8_U:
      case OPCode::I64_LOAD16_S:
      case OPCode::I64_LOAD16_U:
      case OPCode::I64_LOAD32_S:
      case OPCode::I64_LOAD32_U:
      case OPCode::I32_STORE:
      case OPCode::I64_STORE:
      case OPCode::F32_STORE:
      case OPCode::F64_STORE:
      case OPCode::I32_STORE8:
      case OPCode::I32_STORE16:
      case OPCode::I64_STORE8:
      case OPCode::I64_STORE16:
      case OPCode::I64_STORE32:
      case OPCode::MEMORY_SIZE:
      case OPCode::MEMORY_GROW:
      case OPCode::F32_CONST:
      case OPCode::F64_CONST:
      case OPCode::I32_EQ:
      case OPCode::I32_NE:
      case OPCode::I32_LT_S:
      case OPCode::I32_LT_U:
      case OPCode::I32_GT_S:
      case OPCode::I32_GT_U:
      case OPCode::I32_LE_S:
      case OPCode::I32_GE_S:
      case OPCode::I32_GE_U:
      case OPCode::I64_EQ:
      case OPCode::I64_NE:
      case OPCode::I64_LT_S:
      case OPCode::I64_LT_U:
      case OPCode::I64_GT_S:
      case OPCode::I64_GT_U:
      case OPCode::I64_LE_S:
      case OPCode::I64_GE_S:
      case OPCode::I64_GE_U:
      case OPCode::I32_CLZ:
      case OPCode::I32_POPCNT:
      case OPCode::I32_REM_S:
      case OPCode::I32_REM_U:
      case OPCode::I32_AND:
      case OPCode::I32_OR:
      case OPCode::I32_XOR:
      case OPCode::I32_SHL:
      case OPCode::I32_SHR_S:
      case OPCode::I32_SHR_U:
      case OPCode::I32_ROTL:
      case OPCode::I32_ROTR:
      case OPCode::I64_CLZ:
      case OPCode::I64_POPCNT:
      case OPCode::I64_REM_S:
      case OPCode::I64_REM_U:
      case OPCode::I64_AND:
      case OPCode::I64_OR:
      case OPCode::I64_XOR:
      case OPCode::I64_SHL:
      case OPCode::I64_SHR_S:
      case OPCode::I64_SHR_U:
      case OPCode::I64_ROTL:
      case OPCode::I64_ROTR:
        throw std::runtime_error("unsupported OPCode " + std::to_string(static_cast<uint32_t>(opcode)));
        break;
      }
    }
    confirm(br_.getOffset() == (preParseFuncBROffset + funcBodySize), "must end with all code parsed");

    module_.functionInfos_.push_back(std::move(funcBody));
    labelManager.relpatchAllLabels(); // relpatch all labels in this function body
    // labelManager will destruct
  }

  confirm(funcNumbers == module_.functionInfos_.size(), "parse code functionBodys exception");
}
void Frontend::parseNameSection() {
  // uint32_t const stringLength = br_.readLEB128<uint32_t>();
  // std::string name{};
  // for (uint32_t i = 0; i < stringLength; i++) {
  //   name += static_cast<char>(br_.readByte());
  // }
}

void Frontend::LabelManager::relpatchAllLabels() {
  for (LabelManager::BrInfo const &brInfo : brIfInfos_) {
    uint32_t const branchInsPosOff = brInfo.BrInsStartAddress;
    uint32_t const whereToJump = labels_[brInfo.labelIndex - 1U]; // should -1 as index in vector
    if (brInfo.isBrIf) {
      // condition jump instruction
      int32_t const condOffset = static_cast<int32_t>((static_cast<int32_t>(whereToJump) - branchInsPosOff) / 4);
      as_.set_b_cond_off(branchInsPosOff, condOffset);
    } else {
      int32_t const jumpOffset = static_cast<int32_t>((static_cast<int32_t>(whereToJump) - branchInsPosOff) / 4);
      as_.set_b_off(branchInsPosOff, jumpOffset);
    }
  }
}

void Frontend::logParsedInfo() {
  LOG_DEBUG << "========================= type section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.type_.size(); i++) {
    LOG_DEBUG << "type[" << i << "] params num = " << module_.type_[i].params.size() << " result num = " << module_.type_[i].results.size()
              << LOG_END;
  }
  LOG_DEBUG << "========================= func section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.func_.size(); i++) {
    LOG_DEBUG << "func[" << i << "] signatureIndex = " << module_.func_[i].signatureIndex << LOG_END;
  }
  LOG_DEBUG << "========================= export section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.export_.size(); i++) {
    LOG_DEBUG << "export[" << i << "] name:\"" << module_.export_[i].exportName << "\" type = " << static_cast<uint32_t>(module_.export_[i].type)
              << " index = " << module_.export_[i].funcIndex << LOG_END;
  }
  LOG_DEBUG << "========================= code section =========================" << LOG_END;
  LOG_DEBUG << "function number = " << module_.functionInfos_.size() << LOG_END;
  for (uint32_t i = 0; i < module_.functionInfos_.size(); i++) {
    LOG_DEBUG << "body[" << i << "] size = " << module_.functionInfos_[i].bodySize << " numLocalDecl = " << module_.functionInfos_[i].locals.size()
              << LOG_END;
  }
  LOG_DEBUG << "========================= name section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.names_.size(); i++) {
    LOG_DEBUG << "name[" << i << "] name " << module_.names_[i].name << "nameSubsectionType " << module_.names_[i].nameSubsectionType << LOG_END;
  }
}
