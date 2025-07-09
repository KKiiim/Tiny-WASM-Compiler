#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

#include "frontend.hpp"
#include "operandManager.hpp"

#include "src/backend/aarch64_encoding.hpp"
#include "src/backend/relpatch.hpp"
#include "src/common/ModuleInfo.hpp"
#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/common/stack.hpp"
#include "src/common/storage.hpp"
#include "src/common/util.hpp"
#include "src/common/wasm_type.hpp"

ExecutableMemory &Frontend::startCompilation(std::string const &wasmPath) {
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
      parseImportSection();
      break;
    case SectionType::FUNCTION:
      parseFunctionSection();
      break;
    case SectionType::TABLE:
      parseTableSection();
      break;
    case SectionType::MEMORY:
      parseMemorySection();
      break;
    case SectionType::GLOBAL:
      parseGlobalSection();
      break;
    case SectionType::EXPORT:
      parseExportSection();
      break;
    case SectionType::START:
      throw std::runtime_error("SectionType::START unsupported");
      break;
    case SectionType::ELEMENT:
      parseElementSection();
      break;
    case SectionType::CODE:
      genWrapperFunction();
      parseCodeSection();
      break;
    case SectionType::DATA:
      parseDataSection();
      break;
    case SectionType::DATA_COUNT:
      module_.dataCount = br_.readLEB128<uint32_t>();
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

  if (module_.hasTable) {
    makeElementIndexToPureSignatureIndex();
  }

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

void Frontend::genWrapperFunction() {
  nativeToJitWrapper = as_.getCurrentAbsAddress();

  ///< Init X28(ROP) for operandStack
  as_.emit_mov_x_imm64(ROP, operandStack_.getStartAddr());
  ///< Init X27(GLOBAL) for global memory
  as_.emit_mov_x_imm64(GLOBAL, globalMemory.getStartAddr());
  if (module_.memoryNumber > 0U) {
    ///< Init X26(LinMem) for linear memory
    as_.emit_mov_x_imm64(LinMem, linearMemory.getStartAddr());
    ///< Init/Restore X25(SizeLinMem) for linear memory pages size
    as_.emit_mov_x_imm64(REG::R9, linearMemoryByteSize.getStartAddr());
    as_.ldr_base_byteOff(SizeLinMem, REG::R9, 0, false);
  }

  ///< Init X24 for stack guard
  as_.emit_mov_x_imm64(StackGuard, stackGuardSize);
  as_.sub_r_r_immReg(StackGuard, REG::SP, StackGuard, true);

  as_.dec_sp(16U); // reserve 8 bytes(align to 16) for return address(LR)
  Storage const functionIndexReg{REG::R7};
  emitWasmCall(functionIndexReg);
  as_.inc_sp(16U);

  // Update SizeLinMem to memory after current nativeToWasm call
  as_.emit_mov_x_imm64(REG::R9, linearMemoryByteSize.getStartAddr());
  as_.str_base_byteOff(REG::R9, SizeLinMem, 0, false);

  as_.ret();
}

void Frontend::parseMemorySection() {
  module_.memoryNumber = br_.readLEB128<uint32_t>();
  confirm(module_.memoryNumber == 1U, "only support 1 memory yet");
  for (uint32_t i = 0U; i < module_.memoryNumber; ++i) {
    ModuleInfo::MemoryInfo memoryInfo{};
    memoryInfo.hasLimit = br_.readByte<uint8_t>() == 1U;
    memoryInfo.initialSize = br_.readLEB128<uint32_t>();
    if (memoryInfo.hasLimit) {
      memoryInfo.maximumSize = br_.readLEB128<uint32_t>();
      confirm(memoryInfo.maximumSize <= MaxLinearMemoryPages, "must");
    }
    module_.memoryInfos.push_back(memoryInfo);
    uint64_t const memoryByteSize = static_cast<uint64_t>(DefaultPageSize) * module_.memoryInfos[0].initialSize;
    confirm(memoryByteSize <= UINT32_MAX, "memoryByteSize assumed to within 32 bits");
    linearMemoryByteSize.set(0, static_cast<uint32_t>(memoryByteSize & 0xFFFFFFFFU));
  }
}
void Frontend::parseDataSection() {
  module_.numberDataSegments = br_.readLEB128<uint32_t>();
  for (uint32_t i = 0U; i < module_.numberDataSegments; ++i) {
    // data segment header
    enum class DataMode : uint32_t { Active, Passive, ActiveInNonDefaultMemory };
    DataMode const mode{static_cast<DataMode>(br_.readLEB128<uint32_t>())};
    confirm(mode == DataMode::Active, "Passive_mode_data_segments_not_implemented");

    OPCode const type = br_.readByte<OPCode>();
    confirm(type == OPCode::I32_CONST, "others not supported yet");
    uint32_t const dataInitOffset = br_.readLEB128<uint32_t>();

    confirm(br_.readByte<OPCode>() == OPCode::END, "must");

    uint32_t const currentDataSegmentSize = br_.readLEB128<uint32_t>();
    // data segment data
    for (uint32_t j = 0U; j < currentDataSegmentSize; ++j) {
      linearMemory.set(j + dataInitOffset, br_.readByte<uint8_t>());
    }
  }
}
void Frontend::parseGlobalSection() {
  uint32_t const globalNumbers{br_.readLEB128<uint32_t>()};

  uint32_t globalOffset = 0U; // used for mutable globals
  for (uint32_t i = 0U; i < globalNumbers; ++i) {
    WasmType const type = br_.readByte<WasmType>();
    confirm((type == WasmType::I64 || type == WasmType::I32), "only i64 and i32 global supported");
    bool const is64bit = (type == WasmType::I64);

    ModuleInfo::GlobalInfo globalInfo{};
    globalInfo.isMutable = br_.readByte<uint8_t>() == 1U;
    globalInfo.is64bit = is64bit;
    OPCode const dataType = br_.readByte<OPCode>();
    confirm((((dataType == OPCode::I32_CONST) && (!is64bit)) || ((dataType == OPCode::I64_CONST) && is64bit)), "global type and init value mismatch");

    if (is64bit) {
      globalInfo.value = bit_cast<uint64_t>(br_.readLEB128<int64_t>());
    } else {
      // Aligned to 8 bytes, u32 use low 4 bytes
      uint64_t const u64Value = bit_cast<uint64_t>(static_cast<int64_t>(br_.readLEB128<int32_t>()));
      globalInfo.value = u64Value;
    }

    if (globalInfo.isMutable) {
      globalInfo.offset = globalOffset;
      globalMemory.set(globalInfo.offset / sizeof(uint64_t), globalInfo.value);
      globalOffset += 8U; // aligned to 8 bytes
    }
    confirm(br_.readByte<OPCode>() == OPCode::END, "global declaration must end with END");
    module_.globalManager.push_back(globalInfo);
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

    std::string params{"("};
    uint32_t const paramsNum = br_.readLEB128<uint32_t>();
    confirm(paramsNum <= MaxParamsForWasmFunction, "more params not supported yet");
    for (uint32_t paramIndex = 0U; paramIndex < paramsNum; paramIndex++) {
      WasmType const pType = br_.readByte<WasmType>();
      paramInfos.push_back(pType);
      params.push_back(static_cast<char>(ModuleInfo::wasmType2SignatureType(pType)));
    }
    params.push_back(')'); // end of params

    std::string signature{};

    uint32_t const resultNum = br_.readLEB128<uint32_t>();
    for (uint32_t resultIndex = 0U; resultIndex < resultNum; resultIndex++) {
      WasmType const rType = br_.readByte<WasmType>();
      resultInfos.push_back(rType);
      signature.push_back(static_cast<char>(ModuleInfo::wasmType2SignatureType(rType)));
    }
    confirm(resultInfos.size() <= 1U, "only one result supported");

    signature += params;

    module_.typeInfo_.push_back({paramInfos, resultInfos, signature});

    // Check whether a previous signature matched. Needed for indirect calls to matching signatures with different indices
    module_.signatureStringToPureSigIndex.set(signature);
  }
}
void Frontend::parseFunctionSection() {
  uint32_t const funcNumbers{br_.readLEB128<uint32_t>()};
  uint32_t counter = 0U;
  while (counter < funcNumbers) {
    counter++;

    uint32_t const index = br_.readLEB128<uint32_t>();
    module_.funcIndex2TypeIndex_.push_back(index);
  }
}
void Frontend::parseTableSection() {
  uint32_t const tableNumbers{br_.readLEB128<uint32_t>()};
  confirm(tableNumbers == 1U, "so far, even spec only support one table");
  WasmType const elementType{br_.readByte<WasmType>()};
  confirm(elementType == WasmType::FUNC_REF, "only tableType funcref allowed");
  module_.hasTable = true;

  uint8_t const hasSizeLimit{br_.readByte<uint8_t>()};
  // hasSizeLimit flag is only allowed to be 0 = false or 1 = true
  confirm((hasSizeLimit == 0U || hasSizeLimit == 1U), "Unknown size limit flag");
  // Convert to bool
  module_.tableHasSizeLimit = hasSizeLimit != 0U;
  module_.tableInitialSize = br_.readLEB128<uint32_t>();

  if (module_.tableHasSizeLimit) {
    module_.tableMaximumSize = br_.readLEB128<uint32_t>();
    confirm(module_.tableInitialSize <= module_.tableMaximumSize, "Maximum_table_size_smaller_than_initial_table_size");

    confirm(module_.tableInitialSize == module_.tableMaximumSize, "current implementation only supports all initial");
  }
}
void Frontend::parseElementSection() {
  uint32_t const numElementSegments{br_.readLEB128<uint32_t>()};
  confirm(numElementSegments == 1U, "Number of table segments only support 1");
  enum class ElementMode : uint8_t { LegacyIndex, PassiveIndex, ActiveIndex, DeclaredIndex, LegacyExpr, PassiveExpr, ActiveExpr, DeclaredExpr };
  ElementMode const mode{static_cast<ElementMode>(br_.readLEB128<uint32_t>())};
  confirm(mode == ElementMode::LegacyIndex, "Bulk_memory_operations_feature_not_implemented");
  confirm(module_.hasTable, "must has table if has element section");

  confirm(br_.readByte<OPCode>() == OPCode::I32_CONST, "only support i32 offset yet");
  confirm(br_.readLEB128<uint32_t>() == 0U, "only support 0 offset yet");
  confirm(br_.readByte<OPCode>() == OPCode::END, "must be END");

  // Number of actual table elements (function pointers/references)
  module_.numberElements = br_.readLEB128<uint32_t>();
  confirm(module_.numberElements <= module_.tableInitialSize, "Table_element_index_out_of_range__initial_table_size_");
  confirm(module_.numberElements == module_.tableInitialSize, "current implementation only supports all initial");
  for (uint32_t elementIndex = 0U; elementIndex < module_.numberElements; elementIndex++) {
    uint32_t const functionIndex{br_.readLEB128<uint32_t>()};
    confirm(functionIndex < module_.funcIndex2TypeIndex_.size(), "Function_index_out_of_range");
    confirm(functionIndex >= module_.importsFunctionNumbers, "indirect call import function not supported yet");
    // offset is in number of Data
    elementIndexToFunctionIndex.set(elementIndex, functionIndex);
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

    if (type != WasmImportExportType::FUNC) {
      continue; ///< only function export supported yet
    }

    module_.export_.push_back({exportName, type, index});
    module_.exportFuncNameToIndex_[exportName] = index;
  }
}
RuntimeBlock<uint8_t> *linearMemoryHelper = nullptr;
int myPrintf(uint32_t const formatOffset, uint32_t const argsOffset) {
  const char *const msg = bit_cast<const char *>(linearMemoryHelper->addr(formatOffset));
  int const firstArg = *bit_cast<int *>(linearMemoryHelper->addr(argsOffset));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  printf(msg, firstArg);
  return 0;
}
void Frontend::parseImportSection() {
  uint32_t const importsNumbers{br_.readLEB128<uint32_t>()};
  for (uint32_t counter = 0U; counter < importsNumbers; counter++) {
    uint32_t const moduleNameLength{br_.readLEB128<uint32_t>()};
    std::string moduleName{};
    for (uint32_t i = 0; i < moduleNameLength; i++) {
      moduleName += br_.readByte<char>();
    }
    uint32_t const fieldNameLength{br_.readLEB128<uint32_t>()};
    std::string fieldName{};
    for (uint32_t i = 0; i < fieldNameLength; i++) {
      fieldName += br_.readByte<char>();
    }
    uint8_t const importKind{br_.readLEB128<uint8_t>()};
    confirm(importKind == 0x00, "only support function import yet");

    uint32_t const signatureIndex{br_.readLEB128<uint32_t>()};
    module_.funcIndex2TypeIndex_.push_back(signatureIndex);
    module_.functionInfos_.push_back({/*placeholder*/});

    confirm(moduleName == "env" && fieldName == "myPrintf", "Partially supported yet");
    linearMemoryHelper = &linearMemory;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    uintptr_t const staticLinkPrintfAddr = reinterpret_cast<uintptr_t>(myPrintf);
    // counter is the import's functionIndex, since imports must occur before all non-import definitions
    funcAddrTable.set(counter, staticLinkPrintfAddr);
  }

  module_.importsFunctionNumbers = importsNumbers;
}
void Frontend::parseCodeSection() {
  uint32_t const internalFuncNumbers{br_.readLEB128<uint32_t>()};

  // parse each function body
  for (uint32_t internalFuncCouter = 0U; internalFuncCouter < internalFuncNumbers; ++internalFuncCouter) {
    uint32_t const currentFuncIndex = internalFuncCouter + module_.importsFunctionNumbers;
    ////// Temporary variables of CURRENT function
    ///< record local's memory addr offset from SP in this function
    ///< to handle operand variables
    // TODO(): should split to different class
    OP op{as_};
    LabelManager labelManager{as_};

    auto const &currentFuncTypeInfo = module_.getTypeInfo(currentFuncIndex);

    ModuleInfo::FunctionInfo funcBody{};
    uint32_t const stackElementIndex = stack_.push(StackElement{ElementType::FUNC_START});
    funcBody.belongingBlockIndex = stackElementIndex;
    uint32_t const funcBodySize = br_.readLEB128<uint32_t>();
    uint32_t const preParseFuncBROffset = br_.getOffset();

    // Parse local type decl(same type local array)
    // (local i64 i32 i64) -> localDeclCount == 3. i64*1 i32*1 i64*1
    // (local i64 i32 i32) -> localDeclCount == 2. i64*1 i32*2
    // params not included
    uint32_t const localDeclCount = br_.readLEB128<uint32_t>();

    ///< Prepare locals and params. Calculate each local's offset from SP
    // Params:
    // params normally used by register, but store in memory before call
    for (auto const &param : currentFuncTypeInfo.params) {
      funcBody.locals.push_back({true, op.add(param), param});
    }
    // Other Locals:
    uint32_t localDeclIndex = 0U;
    while (localDeclIndex++ < localDeclCount) {
      uint32_t const currentTypeLocalCount = br_.readLEB128<uint32_t>();
      WasmType const localType = br_.readByte<WasmType>();
      for (uint32_t i = 0U; i < currentTypeLocalCount; ++i) {
        funcBody.locals.push_back({false, op.add(localType), localType});
      }
    }

    ///////////////////////////////
    ///< Position Of Function Start
    ///////////////////////////////

    funcAddrTable.set(currentFuncIndex, as_.getCurrentAbsAddress());

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
        if (currentFuncTypeInfo.results.size() == 1U) {
          // function with return value
          confirm(stack_.top().isValue(), "should at least have one value element for return value");
          StackElement const retValue = stack_.top();
          confirm(toWasmType(retValue.elementType_) == currentFuncTypeInfo.results[0], "validation stack top should be the return value type");

          bool const is64bit = currentFuncTypeInfo.results[0] == WasmType::I64;
          // prepare return value
          op.subROP(is64bit);
          as_.ldr_base_byteOff(REG::R0, REG::R28, 0U, is64bit);
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
          confirm(localIdx < currentFuncTypeInfo.params.size(), "");
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
          confirm(localIdx < currentFuncTypeInfo.params.size(), "");
          confirm(l.type == currentFuncTypeInfo.params[localIdx], "");
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
        as_.str_base_byteOff(ROP, REG::R9, 0U, false);
        as_.add_r_r_imm(ROP, ROP, 4U, true);
        break;
      }
      case OPCode::I64_CONST: {
        uint64_t const v = bit_cast<uint64_t>(br_.readLEB128<int64_t>());
        stack_.push(StackElement{ElementType::I64});
        // Use W9 as scratch register
        as_.emit_mov_x_imm64(REG::R9, v);
        as_.str_base_byteOff(ROP, REG::R9, 0U, true);
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
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        // Use R10 as left value scratch register
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);
        as_.add_r_r_shiftR(REG::R9, REG::R10, REG::R9, is64bit);

        // Store result to ROP
        as_.str_base_byteOff(ROP, REG::R9, 0U, is64bit);
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
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        // Use R10 as left value scratch register
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);
        as_.sub_r_r_shiftR(REG::R9, REG::R10, REG::R9, is64bit);

        // Store result to ROP
        as_.str_base_byteOff(ROP, REG::R9, 0U, is64bit);
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
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        // Use R10 as left value scratch register
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);
        as_.mul_r_r(REG::R9, REG::R10, REG::R9, is64bit);

        // Store result to ROP
        as_.str_base_byteOff(ROP, REG::R9, 0U, is64bit);
        op.addROP(is64bit);
        break;
      }
      case OPCode::END: {
        StackElement const &lastControlFlowElement = stack_.lastControlFlowElement();

        switch (lastControlFlowElement.elementType_) {
        case ElementType::FUNC_START: {
          confirm(br_.getOffset() == (preParseFuncBROffset + funcBodySize), "");
          if (currentFuncTypeInfo.results.size() == 1U) {
            // function with return value
            confirm(stack_.top().isValue(), "should at least have one value element for return value");
            StackElement const retValue = stack_.pop();
            confirm(toWasmType(retValue.elementType_) == currentFuncTypeInfo.results[0], "validation stack top should be the return value type");

            ///< Function may have other values in stack, it's valid. Use the last value as return value
            stack_.popToLastControlFlowElement(); // including pop FUNC_START
            // FIXME(#59): should else pop redundant value in runtime operand stack
            stack_.push(retValue);

            bool const is64bit = currentFuncTypeInfo.results[0] == WasmType::I64;
            // prepare return value
            op.subROP(is64bit);
            as_.ldr_base_byteOff(REG::R0, REG::R28, 0U, is64bit);
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
          if (!lastControlFlowElement.isLoopBlock) {
            labelManager.fillTargetJumpAddress(lastControlFlowElement.labelIndex, as_.getCurrentOffset());
          }

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
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, false);

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
        as_.ldr_base_byteOff(REG::R9, ROP, 0, is64bit);
        as_.cmp_r_imm(REG::R9, 0U, is64bit);
        Relpatch const notDiv0 = as_.prepareJmp(CC::NE);
        as_.setTrap(Trapcode::DIV_0);
        notDiv0.linkToHere();
        ///< Divisor is not zero, continue to do division
        // get dividend in R10
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);

        bool const isSigned = (opcode == OPCode::I32_DIV_S || opcode == OPCode::I64_DIV_S);
        ///< If divisor is -1 and dividend is INT_MIN, sign div will trap with integer overflow
        if (isSigned) {
          // check if dividend is INT_MIN
          if (is64bit) {
            as_.emit_mov_x_imm64(REG::R11, 0x8000000000000000);
            as_.cmp_r_r(REG::R10, REG::R11, true);
          } else {
            as_.emit_mov_w_imm32(REG::R11, 0x80000000);
            as_.cmp_r_r(REG::R10, REG::R11, false);
          }
          Relpatch const notIntMin = as_.prepareJmp(CC::NE);

          // Then check if divisor is -1
          // If divisor is -1, it will trap with integer overflow
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
        }

        ///< Safe division
        if (opcode == OPCode::I32_DIV_S || opcode == OPCode::I64_DIV_S) {
          as_.sdiv_r_r(REG::R9, REG::R10, REG::R9, is64bit);
        } else {
          as_.udiv_r_r(REG::R9, REG::R10, REG::R9, is64bit);
        }
        as_.str_base_byteOff(ROP, REG::R9, 0U, is64bit);
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
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, false);
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
      case OPCode::I32_CLZ:
      case OPCode::I64_CTZ:
      case OPCode::I64_CLZ: {
        bool const is64bit = (opcode == OPCode::I64_CTZ || opcode == OPCode::I64_CLZ);
        confirm((stack_.top().isValue() && (stack_.top().isI64() == is64bit)), "must be value type");
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        if (opcode == OPCode::I32_CTZ || opcode == OPCode::I64_CTZ) {
          as_.rBits_r_r(REG::R9, REG::R9, is64bit);
        }
        as_.clz_r_r(REG::R9, REG::R9, is64bit);
        as_.str_base_byteOff(ROP, REG::R9, 0U, is64bit);
        op.addROP(is64bit);
        // don't pop stack, since the ctz result is the same type element
        break;
      }
      case OPCode::I64_LE_U:
      case OPCode::I32_LE_U:
      case OPCode::I64_LE_S:
      case OPCode::I32_LE_S: {
        bool const is64bit = (opcode == OPCode::I64_LE_U || opcode == OPCode::I64_LE_S);
        bool const isSigned = (opcode == OPCode::I64_LE_S || opcode == OPCode::I32_LE_S);
        StackElement const right = stack_.pop();
        StackElement const left = stack_.pop();
        confirm(right.isI64() == is64bit, "must be value type");
        confirm(left.isI64() == is64bit, "must be value type");
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);

        // prepare result as default true, unsigned left <= right
        REG const resultReg = REG::R11;
        as_.emit_mov_w_imm32(resultReg, 1U);

        as_.cmp_r_r(REG::R10, REG::R9, is64bit);
        if (isSigned) {
          Relpatch const signedLessOrEqual = as_.prepareJmp(CC::LE);
          // not LE(signed)
          as_.emit_mov_w_imm32(resultReg, 0);
          signedLessOrEqual.linkToHere();
        } else {
          Relpatch const unsignedLessOrSame = as_.prepareJmp(CC::LS);
          // not LS(unsigned)
          as_.emit_mov_w_imm32(resultReg, 0);
          unsignedLessOrSame.linkToHere();
        }
        as_.str_base_byteOff(ROP, resultReg, 0U, false);
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
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);

        REG const resultReg = REG::R10;
        // prepare default not zero, set 0 (if-else downgraded to if)
        as_.emit_mov_w_imm32(resultReg, 0U);

        as_.cmp_r_imm(REG::R9, 0U, is64bit);
        Relpatch const notZero = as_.prepareJmp(CC::NE);
        // value is zero. Set true
        as_.emit_mov_w_imm32(resultReg, 1U);
        notZero.linkToHere();
        as_.str_base_byteOff(ROP, resultReg, 0U, false);
        op.addROP(false);

        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::I64_NE:
      case OPCode::I32_NE: {
        bool const is64bit = (opcode == OPCode::I64_NE);
        confirm((stack_.top().isValue() && (stack_.top().isI64() == is64bit)), "must match value type");
        stack_.pop();
        confirm((stack_.top().isValue() && (stack_.top().isI64() == is64bit)), "must match value type");
        stack_.pop();

        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);

        REG const resultReg = REG::R11;
        // prepare default not equal(true)
        as_.emit_mov_w_imm32(resultReg, 1U);

        as_.cmp_r_r(REG::R10, REG::R9, is64bit);
        Relpatch const notEqual = as_.prepareJmp(CC::NE);
        as_.emit_mov_w_imm32(resultReg, 0U);
        notEqual.linkToHere();

        as_.str_base_byteOff(ROP, resultReg, 0U, false);
        op.addROP(false);

        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::LOOP: {
        WasmType const loopReturnType = br_.readByte<WasmType>();
        confirm(((loopReturnType == WasmType::TVOID) || (loopReturnType == WasmType::I32) || (loopReturnType == WasmType::I64)),
                "only i32, i64 or void supported for LOOP return type");
        StackElement loopElement{ElementType::BLOCK};
        loopElement.returnType_ = loopReturnType;
        loopElement.labelIndex = labelManager.registerLabel();
        loopElement.isLoopBlock = true;
        stack_.push(loopElement);

        labelManager.fillTargetJumpAddress(loopElement.labelIndex, as_.getCurrentOffset());
        break;
      }
      case OPCode::I32_EQ:
      case OPCode::I64_EQ: {
        bool const is64bit = (opcode == OPCode::I64_EQ);
        StackElement const right = stack_.pop();
        StackElement const left = stack_.pop();
        confirm((right.isI64() == is64bit && left.isI64() == is64bit), "must match value type");

        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);

        REG const resultReg = REG::R11;
        // prepare default equal, set 1 (if-else downgraded to if)
        as_.emit_mov_w_imm32(resultReg, 1U);
        as_.cmp_r_r(REG::R10, REG::R9, is64bit);
        Relpatch const equal = as_.prepareJmp(CC::EQ);
        // not equal
        as_.emit_mov_w_imm32(resultReg, 0U);
        equal.linkToHere();
        // Store result to ROP
        as_.str_base_byteOff(ROP, resultReg, 0U, false);
        op.addROP(false);
        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::I32_GT_U:
      case OPCode::I64_GT_U: {
        bool const is64bit = (opcode == OPCode::I64_GT_U);
        StackElement const right = stack_.pop();
        StackElement const left = stack_.pop();
        confirm((right.isI64() == is64bit && left.isI64() == is64bit), "must match value type");

        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);

        REG const resultReg = REG::R11;
        // prepare default true (CC::HI for unsigned)
        as_.emit_mov_w_imm32(resultReg, 1U);
        as_.cmp_r_r(REG::R10, REG::R9, is64bit);
        Relpatch const higher = as_.prepareJmp(CC::HI);
        // not higher
        as_.emit_mov_w_imm32(resultReg, 0U);
        higher.linkToHere();
        // Store result to ROP
        as_.str_base_byteOff(ROP, resultReg, 0U, false);
        op.addROP(false);
        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::CALL: {
        // save registers(only current function's params need to be saved yet, none-param locals always in memory)
        for (uint32_t i = 0U; i < currentFuncTypeInfo.params.size(); i++) {
          auto const &param = funcBody.locals[i];
          confirm(param.isParam, "must be param");
          confirm(param.type == currentFuncTypeInfo.params[i], "param type should match the validation");
          as_.str_base_byteOff(REG::SP, static_cast<REG>(i), param.offset, param.type == WasmType::I64);
        }

        uint32_t const callIndex{br_.readLEB128<uint32_t>()};
        confirm(callIndex < module_.funcIndex2TypeIndex_.size(), "function index out of range");
        auto const &callType = module_.getTypeInfo(callIndex);

        prepareCallParams(callType, op);

        Storage const callIndexStorage{ConstUnion{callIndex}};
        bool const isImportFunction = callIndex < module_.importsFunctionNumbers;
        if (isImportFunction) {
          spillRuntime();
          emitWasmCall(callIndexStorage);
          recoverRuntime();
        } else {
          emitWasmCall(callIndexStorage);
        }
        handleReturnValue(callType, op);
        recoveryCurrentFrameReg(funcBody, currentFuncTypeInfo);

        break;
      }
      case OPCode::CALL_INDIRECT: {
        confirm(module_.hasTable, "must has table for CALL_INDIRECT");
        // The index of the function type/signature is given as an immediate to this instruction
        uint32_t const expectedSignatureIndex = br_.readLEB128<uint32_t>();
        uint32_t const expectedPureSigIndex = module_.signatureStringToPureSigIndex.get(module_.typeInfo_[expectedSignatureIndex].signature);
        // Only table index 0 is supported in the MVP of Wasm
        confirm(br_.readLEB128<uint32_t>() == 0U, "must");

        ///< Get and check table element index
        REG const elementIndex = REG::R9;
        confirm(!stack_.pop().isI64(), "CALL_INDIRECT need one i32 element index");
        op.subROP(false);
        as_.ldr_base_byteOff(elementIndex, ROP, 0U, false);
        // TRAP if elementIndex is not smaller than the length of tab.elem
        as_.cmp_r_imm(elementIndex, module_.numberElements, false);
        Relpatch const notOutOfRange = as_.prepareJmp(CC::LO);
        as_.setTrap(Trapcode::TableElement_out_of_range);
        notOutOfRange.linkToHere();
        // TRAP if tab.elem[𝑖] is uninitialized
        confirm(module_.numberElements == module_.tableInitialSize, "must be all initialized");

        ///< Get its pure signature index
        REG const callPureSigIndexReg = REG::R10; // 4B
        as_.emit_mov_x_imm64(REG::R11, elementIndexToPureSignatureIndex.getStartAddr());
        as_.ldr_offReg(callPureSigIndexReg, REG::R11, elementIndex, false);
        ///< Check if the signature index matches the expected one
        as_.cmp_r_imm(callPureSigIndexReg, expectedPureSigIndex, false);
        Relpatch const signatureMatch = as_.prepareJmp(CC::EQ);
        as_.setTrap(Trapcode::IndirectCall_signature_mismatch);
        signatureMatch.linkToHere();

        ///< Prepare call params, since matched signature is compile-time known
        auto const &callType = module_.typeInfo_[expectedSignatureIndex];
        // pure signature index is checked above, so we can use it to get the function type
        prepareCallParams(callType, op);

        // reuse R10 as base address
        as_.emit_mov_x_imm64(REG::R10, elementIndexToFunctionIndex.getStartAddr());
        ///< Get function abs address
        REG const functionIndex = REG::R10; // 4B
        as_.ldr_offReg(functionIndex, REG::R10, elementIndex, false);

        ///< Emit call
        Storage const functionIndexRegStorage{REG{functionIndex}};
        // emitWasmCall will use R9 as scratch registers, functionIndexRegStorage as the param should avoid using R9
        emitWasmCall(functionIndexRegStorage);
        handleReturnValue(callType, op);
        recoveryCurrentFrameReg(funcBody, currentFuncTypeInfo);

        break;
      }
      case OPCode::GLOBAL_GET: {
        uint32_t const globalIndex{br_.readLEB128<uint32_t>()};
        confirm(globalIndex < module_.globalManager.size(), "global index out of range");
        auto const &globalInfo = module_.globalManager[globalIndex];
        if (globalInfo.isMutable) {
          // global stored as 8 bytes aligned
          as_.ldr_base_byteOff(REG::R9, GLOBAL, globalInfo.offset, true);
        } else {
          as_.emit_mov_x_imm64(REG::R9, globalInfo.value);
        }
        as_.str_base_byteOff(ROP, REG::R9, 0U, globalInfo.is64bit);
        op.addROP(globalInfo.is64bit);
        stack_.push(StackElement{globalInfo.is64bit ? ElementType::I64 : ElementType::I32});
        break;
      }
      case OPCode::GLOBAL_SET: {
        uint32_t const globalIndex{br_.readLEB128<uint32_t>()};
        confirm(globalIndex < module_.globalManager.size(), "global index out of range");
        auto const &globalInfo = module_.globalManager[globalIndex];
        confirm(globalInfo.isMutable, "global set must be mutable");
        confirm(stack_.pop().isI64() == globalInfo.is64bit, "global set value type should match the global type");
        op.subROP(globalInfo.is64bit);
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, globalInfo.is64bit);
        // global stored as 8 bytes aligned
        as_.str_base_byteOff(GLOBAL, REG::R9, globalInfo.offset, true);
        break;
      }
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
      case OPCode::I64_LOAD32_U: {
        ///< Feature not fully supported
        confirm((opcode != OPCode::F32_LOAD && opcode != OPCode::F64_LOAD), "float number not supported");
        confirm((opcode == OPCode::I32_LOAD8_U || opcode == OPCode::I32_LOAD || opcode == OPCode::I32_LOAD8_S), "only part supported for now");

        constexpr auto maxAlignmentPow2 = make_array(2_U8, 3_U8, 2_U8, 3_U8, 0_U8, 0_U8, 1_U8, 1_U8, 0_U8, 0_U8, 1_U8, 1_U8, 2_U8, 2_U8);
        uint32_t const alignment{br_.readLEB128<uint32_t>()};
        confirm(alignment <= maxAlignmentPow2[static_cast<uint32_t>(opcode) - static_cast<uint32_t>(OPCode::I32_LOAD)], "Alignment_out_of_range");

        uint32_t const loadOffset{br_.readLEB128<uint32_t>()};
        confirm(loadOffset == 0U, "offset not supported for now");

        bool const is64bit =
            (opcode == OPCode::I64_LOAD || opcode == OPCode::F64_LOAD || (opcode >= OPCode::I64_LOAD8_S && opcode <= OPCode::I64_LOAD32_U));
        constexpr auto loadSizeArray = make_array(4U, 8U, 4U, 8U, 1U, 1U, 2U, 2U, 1U, 1U, 2U, 2U, 4U, 4U);
        uint32_t const loadSize = loadSizeArray[static_cast<uint32_t>(opcode) - static_cast<uint32_t>(OPCode::I32_LOAD)];

        confirm(!stack_.pop().isI64(), "offset must be i32 value type");
        op.subROP(false);
        REG const offsetReg = REG::R9;
        as_.ldr_base_byteOff(offsetReg, ROP, 0U, false);

        // TRAP if range is out of linear memory bounds
        as_.add_r_r_imm(REG::R11, offsetReg, loadSize, true);
        as_.cmp_r_r(REG::R11, SizeLinMem, true);
        Relpatch const notOOM = as_.prepareJmp(CC::LS);
        as_.setTrap(Trapcode::Out_of_bounds_memory_access);
        notOOM.linkToHere();

        REG const getDataReg = REG::R10;
        if (loadSize == 1U) {
          as_.ldrb_uReg(getDataReg, LinMem, offsetReg);
        } else if (loadSize == 2U) {
          confirm(false, "load size 2 not supported");
        } else if (loadSize == 4U) {
          as_.add_r_r_shiftR(offsetReg, LinMem, offsetReg, true);
          // offsetReg is now the address
          as_.ldr_base_byteOff(getDataReg, offsetReg, 0, false);
        } else if (loadSize == 8U) {
          confirm(false, "load size 8 not supported");
        } else {
          confirm(false, "load size not supported");
        }
        as_.str_base_byteOff(ROP, getDataReg, 0U, is64bit);
        op.addROP(is64bit);
        stack_.push(StackElement{is64bit ? ElementType::I64 : ElementType::I32});
        break;
      }
      case OPCode::I32_STORE:
      case OPCode::I64_STORE:
      case OPCode::F32_STORE:
      case OPCode::F64_STORE:
      case OPCode::I32_STORE8:
      case OPCode::I32_STORE16:
      case OPCode::I64_STORE8:
      case OPCode::I64_STORE16:
      case OPCode::I64_STORE32: {
        bool const is64bit =
            (opcode == OPCode::I64_STORE || opcode == OPCode::F64_STORE || (opcode >= OPCode::I64_STORE8 && opcode <= OPCode::I64_STORE32));

        ///< Feature not fully supported
        confirm((opcode != OPCode::F32_STORE && opcode != OPCode::F64_STORE), "float number not supported");
        confirm(!is64bit, "only i32 store supported for now");
        confirm((opcode == OPCode::I32_STORE || opcode == OPCode::I32_STORE8 || opcode == OPCode::I32_STORE16), "only part supported for now");

        constexpr auto storeSizeArray = make_array(4U, 8U, 4U, 8U, 1U, 2U, 1U, 2U, 4U);
        constexpr auto maxAlignmentPow2 = make_array(2_U8, 3_U8, 2_U8, 3_U8, 0_U8, 1_U8, 0_U8, 1_U8, 2_U8);
        uint32_t const alignment{br_.readLEB128<uint32_t>()};
        confirm(alignment <= maxAlignmentPow2[static_cast<uint32_t>(opcode) - static_cast<uint32_t>(OPCode::I32_STORE)], "Alignment_out_of_range");

        uint32_t const storeOffset{br_.readLEB128<uint32_t>()};
        confirm(storeOffset == 0U, "offset not supported for now");
        // validation
        confirm(stack_.pop().isI64() == is64bit, "store data type should match");
        confirm(!stack_.pop().isI64(), "offset must be i32 value type");
        // get data and offset
        REG const dataReg = REG::R9;
        REG const offsetReg = REG::R10;
        op.subROP(is64bit);
        as_.ldr_base_byteOff(dataReg, ROP, 0U, is64bit);
        op.subROP(false);
        as_.ldr_base_byteOff(offsetReg, ROP, 0U, false);

        // TRAP if range is out of linear memory bounds
        as_.add_r_r_imm(REG::R11, offsetReg, storeSizeArray[static_cast<uint32_t>(opcode) - static_cast<uint32_t>(OPCode::I32_STORE)], true);
        as_.cmp_r_r(REG::R11, SizeLinMem, true);
        Relpatch const notOOM = as_.prepareJmp(CC::LS);
        as_.setTrap(Trapcode::Out_of_bounds_memory_access);
        notOOM.linkToHere();

        as_.add_r_r_shiftR(offsetReg, LinMem, offsetReg, true);
        REG const storeAddr = offsetReg;
        // store data
        uint32_t const storeSize = storeSizeArray[static_cast<uint32_t>(opcode) - static_cast<uint32_t>(OPCode::I32_STORE)];
        if (storeSize == 1U) {
          as_.str_base_byteOff(storeAddr, dataReg, 0U, is64bit);
        } else if (storeSize == 2U) {
          as_.str_h_w_r(storeAddr, dataReg);
        } else if (storeSize == 4U) {
          as_.str_w_r(storeAddr, dataReg);
        } else if (storeSize == 8U) {
          confirm(false, "store size 8 not supported");
        } else {
          confirm(false, "store size not supported");
        }
        break;
      }
      case OPCode::MEMORY_SIZE: {
        uint32_t const memoryIndex{br_.readLEB128<uint32_t>()};
        confirm(memoryIndex == 0U, "only memory index 0 supported for now");
        // memory size is in pages, 1 page = 64K
        REG const memorySizeReg = REG::R9;
        REG const memoryPageSizeReg = REG::R10;
        as_.emit_mov_x_imm64(memoryPageSizeReg, DefaultPageSize);
        as_.udiv_r_r(memorySizeReg, SizeLinMem, memoryPageSizeReg, true);
        as_.str_base_byteOff(ROP, memorySizeReg, 0, false);
        op.addROP(false); // memory size is i32
        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::MEMORY_GROW: {
        auto const &memoryInfo = module_.memoryInfos[0];
        uint32_t const memoryIndex{br_.readLEB128<uint32_t>()};
        confirm(memoryIndex == 0U, "only memory index 0 supported for now");
        confirm(!stack_.pop().isI64(), "grow size should be i32 value");
        op.subROP(false);
        REG const growSizeReg = REG::R9;
        as_.ldr_base_byteOff(growSizeReg, ROP, 0U, false);
        // now growSizeReg is number of pages
        REG const currentPageSize = REG::R10;
        REG const memorySizePerPage = REG::R11;
        as_.emit_mov_x_imm64(memorySizePerPage, DefaultPageSize);
        as_.udiv_r_r(currentPageSize, SizeLinMem, memorySizePerPage, true);
        REG const toPageSize = REG::R12;
        as_.add_r_r_shiftR(toPageSize, currentPageSize, growSizeReg, false);

        ///< Get min(embedderLimit, moduleLimit)
        REG const limitSize = REG::R13;
        as_.emit_mov_w_imm32(limitSize, MaxLinearMemoryPages); // init with embedderLimit
        if (memoryInfo.hasLimit) {
          as_.cmp_r_imm(limitSize, memoryInfo.maximumSize, false);
          Relpatch const noNeedToUpdateMin = as_.prepareJmp(CC::LO);
          as_.emit_mov_w_imm32(limitSize, memoryInfo.maximumSize);
          noNeedToUpdateMin.linkToHere();
        }

        // If size larger than limit, or larger than resources available of the embedder
        //    return err(-1)
        // else: return old size
        REG const growRet = REG::R14;
        // init with err
        as_.emit_mov_w_imm32(growRet, static_cast<uint32_t>(-1));
        as_.cmp_r_r(toPageSize, limitSize, false);
        Relpatch const exceedLimit = as_.prepareJmp(CC::HI);
        ///< Not exceed, should return old size
        as_.mov_r_r(growRet, currentPageSize, false);
        ///< Update SizeLinMem
        as_.mul_r_r(growSizeReg, growSizeReg, memorySizePerPage, true);
        // now growSizeReg is bytes size
        as_.add_r_r_shiftR(SizeLinMem, SizeLinMem, growSizeReg, true);
        exceedLimit.linkToHere();

        as_.str_base_byteOff(ROP, growRet, 0, false);
        op.addROP(false);
        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::I32_GE_U:
      case OPCode::I64_GE_U: {
        bool const is64bit = (opcode == OPCode::I64_GE_U);
        confirm((stack_.top().isValue() && (stack_.top().isI64() == is64bit)), "must match value type");
        stack_.pop();
        confirm((stack_.top().isValue() && (stack_.top().isI64() == is64bit)), "must match value type");
        stack_.pop();

        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R9, ROP, 0U, is64bit);
        op.subROP(is64bit);
        as_.ldr_base_byteOff(REG::R10, ROP, 0U, is64bit);

        REG const resultReg = REG::R11;
        // prepare default greater(true)
        as_.emit_mov_w_imm32(resultReg, 1U);

        as_.cmp_r_r(REG::R10, REG::R9, is64bit);
        Relpatch const higher = as_.prepareJmp(CC::HI);
        as_.emit_mov_w_imm32(resultReg, 0U);
        higher.linkToHere();

        as_.str_base_byteOff(ROP, resultReg, 0U, false);
        op.addROP(false);

        stack_.push(StackElement{ElementType::I32});
        break;
      }
      case OPCode::UNREACHABLE:
      case OPCode::BR_TABLE:
      case OPCode::REF_NULL:
      case OPCode::REF_IS_NULL:
      case OPCode::REF_FUNC:
      case OPCode::SELECT:
      case OPCode::SELECT_T:
      case OPCode::TABLE_GET:
      case OPCode::TABLE_SET:
      case OPCode::F32_CONST:
      case OPCode::F64_CONST:
      case OPCode::I32_LT_S:
      case OPCode::I32_LT_U:
      case OPCode::I32_GT_S:
      case OPCode::I32_GE_S:
      case OPCode::I64_LT_S:
      case OPCode::I64_LT_U:
      case OPCode::I64_GT_S:
      case OPCode::I64_GE_S:
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

  confirm(internalFuncNumbers + module_.importsFunctionNumbers == module_.functionInfos_.size(), "parse code functionBodys exception");
  confirm(codeSectionParsed == false, "");
  codeSectionParsed = true;
}
void Frontend::parseNameSection() {
  // uint32_t const stringLength = br_.readLEB128<uint32_t>();
  // std::string name{};
  // for (uint32_t i = 0; i < stringLength; i++) {
  //   name += static_cast<char>(br_.readByte());
  // }
}
void Frontend::prepareCallParams(ModuleInfo::TypeInfo const &callType, OP &op) {
  auto const &callParams = callType.params;
  confirm(callParams.size() <= 8, "call params size should not exceed 8");
  // validate
  for (uint32_t i = 0; i < callParams.size(); ++i) {
    const uint32_t regIndex = callParams.size() - 1 - i; // params are pushed in reverse order
    auto const &paramElement = stack_.pop();
    confirm(paramElement.isI64() == (callParams[regIndex] == WasmType::I64), "Parameter type mismatch");
    op.subROP(paramElement.isI64());
    as_.ldr_base_byteOff(static_cast<REG>(regIndex), ROP, 0, paramElement.isI64());
  }
}
void Frontend::emitWasmCall(Storage const callFuncIndex) {
  // save current LR
  as_.str_base_byteOff(REG::SP, REG::LR, 0, true);

  // emit call
  // R9 as scratch register is used for function table base address and then for function pointer
  as_.emit_mov_x_imm64(REG::R9, funcAddrTable.getStartAddr());
  if (callFuncIndex.type_ == StorageType::REGISTER) {
    // offset reg will times 8
    as_.ldr_offReg(REG::R9, REG::R9, callFuncIndex.location_.reg, true);
  } else if (callFuncIndex.type_ == StorageType::CONSTANT) {
    // offset imm is byte
    as_.ldr_base_byteOff(REG::R9, REG::R9, callFuncIndex.location_.constUnion.u32 * sizeof(uintptr_t), true);
  } else {
    confirm(false, "not supported yet");
  }
  as_.blr(REG::R9);

  // restore LR
  as_.ldr_base_byteOff(REG::LR, REG::SP, 0, true);
}
void Frontend::recoveryCurrentFrameReg(ModuleInfo::FunctionInfo const &funcBody, ModuleInfo::TypeInfo const &funcType) {
  // restore current function's params back to registers
  for (uint32_t i = 0U; i < funcType.params.size(); i++) {
    auto const &param = funcBody.locals[i];
    confirm(param.isParam, "must be param");
    confirm(param.type == funcType.params[i], "param type should match the validation");
    as_.ldr_base_byteOff(static_cast<REG>(i), REG::SP, param.offset, param.type == WasmType::I64);
  }
}
void Frontend::handleReturnValue(ModuleInfo::TypeInfo const &funcType, OP &op) {
  // restore result in R0 if has return value
  if (funcType.results.size() == 1U) {
    bool const is64bit = (funcType.results[0] == WasmType::I64);
    as_.str_base_byteOff(ROP, REG::R0, 0U, is64bit);
    op.addROP(is64bit);
    // push the return value type to stack
    stack_.push(StackElement{is64bit ? ElementType::I64 : ElementType::I32});
  }
}

void Frontend::LabelManager::relpatchAllLabels() {
  for (LabelManager::BrInfo const &brInfo : brIfInfos_) {
    uint32_t const branchInsPosOff = brInfo.BrInsStartAddress;
    uint32_t const whereToJump = labels_[brInfo.labelIndex - 1U]; // should -1 as index in vector
    int32_t const insOffset = static_cast<int32_t>(whereToJump - branchInsPosOff) / 4;
    if (brInfo.isBrIf) {
      // condition jump instruction
      as_.set_b_cond_off(branchInsPosOff, insOffset);
    } else {
      as_.set_b_off(branchInsPosOff, insOffset);
    }
  }
}

void Frontend::spillRuntime() {
  // need store X24~X28 to memory
}
void Frontend::recoverRuntime() {
  // need load back X24~X28 from memory
}

void Frontend::makeElementIndexToPureSignatureIndex() {
  for (uint32_t elementIndex = 0U; elementIndex < module_.numberElements; elementIndex++) {
    uint32_t const functionIndex = elementIndexToFunctionIndex.get(elementIndex);
    std::string const &currentSignatureString = module_.getTypeInfo(functionIndex).signature;
    LOG_DEBUG << "elementIndex[" << elementIndex << "]funcIndex[" << functionIndex << "] sig.size=" << currentSignatureString.size() << ":"
              << currentSignatureString << LOG_END;
    uint32_t const pureSignatureIndex = module_.signatureStringToPureSigIndex.get(currentSignatureString);
    elementIndexToPureSignatureIndex.set(elementIndex, pureSignatureIndex);
  }
}

void Frontend::logParsedInfo() {
  confirm(codeSectionParsed, "must");
  LOG_DEBUG << "========================= type section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.typeInfo_.size(); i++) {
    LOG_DEBUG << "type[" << i << "] params num = " << module_.typeInfo_[i].params.size() << " result num = " << module_.typeInfo_[i].results.size()
              << LOG_END;
  }
  LOG_DEBUG << "========================= func section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.funcIndex2TypeIndex_.size(); i++) {
    LOG_DEBUG << "func[" << i << "] signatureIndex = " << module_.funcIndex2TypeIndex_[i] << LOG_END;
  }
  LOG_DEBUG << "========================= export section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.export_.size(); i++) {
    LOG_DEBUG << "export[" << i << "] name:\"" << module_.export_[i].exportName << "\" type = " << static_cast<uint32_t>(module_.export_[i].type)
              << " index = " << module_.export_[i].funcIndex << LOG_END;
  }
  LOG_DEBUG << "========================= code section =========================" << LOG_END;
  LOG_DEBUG << "function number = " << module_.functionInfos_.size() << LOG_END;
  for (auto &functionInfo : module_.functionInfos_) {
    LOG_DEBUG << " numLocalDecl = " << functionInfo.locals.size() << LOG_END;
  }
  LOG_DEBUG << "========================= name section =========================" << LOG_END;
  for (uint32_t i = 0; i < module_.names_.size(); i++) {
    LOG_DEBUG << "name[" << i << "] name " << module_.names_[i].name << "nameSubsectionType " << module_.names_[i].nameSubsectionType << LOG_END;
  }
}
