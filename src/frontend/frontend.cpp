#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "frontend.hpp"
#include "operandManager.hpp"

#include "src/common/wasm_type.hpp"

ExecutableMemory Frontend::startCompilation(std::string const &wasmPath) {
  br_.readWasmBinary(wasmPath);

  validateMagicNumber();
  validateVersion();
  std::cout << "validate success" << std::endl;

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
  std::cout << "parse wasm success" << std::endl;

  // logParsedInfo();

  return backend_.emit.getExecutableMemory();
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
    assert(paramsNum == paramInfos.size() && "must");

    uint32_t const resultNum = br_.readLEB128<uint32_t>();
    uint32_t resultIndex = 0U;
    while (resultIndex++ < resultNum) {
      resultInfos.push_back(br_.readByte<WasmType>());
    }
    assert(resultNum == resultInfos.size() && "must");
    assert(resultInfos.size() <= 1U && "only one result supported");

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

    assert(type == WasmImportExportType::FUNC && "only function export supported yet");

    module_.export_.push_back({exportName, type, index});
    module_.exportFuncNameToIndex_[exportName] = index;
  }
}
void Frontend::parseCodeSection() {
  uint32_t const funcNumbers{br_.readLEB128<uint32_t>()};
  uint32_t counter = 0U;

  // parse each function body
  while (counter < funcNumbers) {
    uint32_t const funcSignatureIndex{module_.func_[counter].signatureIndex};
    auto const &funcTypeInfo = module_.type_[funcSignatureIndex];
    counter++;

    ModuleInfo::FunctionInfo funcBody{};
    funcBody.startAddressOffset = backend_.emit.getCurrentOffset();
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
    ///< handle operand variables
    // TODO(): should split to different class
    OP op{backend_};
    ///< checker of local operations
    std::stack<OperandStack::OperandType> validationStack{};

    while (localDeclIndex++ < localDeclCount) {
      uint32_t const currentTypeLocalCount = br_.readLEB128<uint32_t>();
      WasmType const localType = br_.readByte<WasmType>();
      for (uint32_t i = 0U; i < currentTypeLocalCount; ++i) {
        funcBody.locals.push_back({false, op.add(localType), localType});
      }
    }
    // TODO(): other stack use excluding local
    uint32_t const stackUsage = op.getAlignedSize();
    std::cout << "stackUsage = " << stackUsage << std::endl;
    if (stackUsage != 0U) {
      backend_.emit.decreaseSPWithClean(stackUsage);
    }
    std::vector<ModuleInfo::WasmInstruction> instructions{};
    while (true) {
      ModuleInfo::WasmInstruction ins{};
      OPCode const opcode = br_.readByte<OPCode>();
      if (opcode == OPCode::END) {
        break;
      }
      ins.opCode = opcode;
      instructions.push_back(ins);

      switch (opcode) {
      case OPCode::RETURN: {
        // do check after END
        break;
      }
      case OPCode::LOCAL_GET: {
        uint32_t const localIdx{br_.readLEB128<uint32_t>()};
        auto const &l = funcBody.locals[localIdx];
        bool const is64bit = l.type == WasmType::I64;
        validationStack.push(is64bit ? OperandStack::OperandType::I64 : OperandStack::OperandType::I32);
        if (l.isParam) {
          // param in register. Assumed params <= 8
          assert(localIdx < funcTypeInfo.params.size());
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
        assert(validationStack.size() != 0U);
        uint32_t const localIdx{br_.readLEB128<uint32_t>()};
        auto const &l = funcBody.locals[localIdx];
        auto const topType = validationStack.top();
        assert(operandStack_.toWasmType(topType) == l.type && "must");
        validationStack.pop();

        if (l.isParam) {
          assert(localIdx < funcTypeInfo.params.size());
          assert(l.type == funcTypeInfo.params[localIdx]);
          op.set_r_param(localIdx, topType == OperandStack::OperandType::I64, opcode == OPCode::LOCAL_TEE);
        } else {
          uint32_t const offset2SP = l.offset;
          op.set_ofsp_local(offset2SP, topType == OperandStack::OperandType::I64, opcode == OPCode::LOCAL_TEE);
        }
        break;
      }
      case OPCode::UNREACHABLE:
      case OPCode::NOP:
      case OPCode::BLOCK:
      case OPCode::LOOP:
      case OPCode::IF:
      case OPCode::ELSE:
      case OPCode::END:
      case OPCode::BR:
      case OPCode::BR_IF:
      case OPCode::BR_TABLE:
      case OPCode::CALL:
      case OPCode::CALL_INDIRECT:
      case OPCode::REF_NULL:
      case OPCode::REF_IS_NULL:
      case OPCode::REF_FUNC:
      case OPCode::DROP:
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
      case OPCode::I32_CONST:
      case OPCode::I64_CONST:
      case OPCode::F32_CONST:
      case OPCode::F64_CONST:
      case OPCode::I32_EQZ:
      case OPCode::I32_EQ:
      case OPCode::I32_NE:
      case OPCode::I32_LT_S:
      case OPCode::I32_LT_U:
      case OPCode::I32_GT_S:
      case OPCode::I32_GT_U:
      case OPCode::I32_LE_S:
      case OPCode::I32_LE_U:
      case OPCode::I32_GE_S:
      case OPCode::I32_GE_U:
      case OPCode::I64_EQZ:
      case OPCode::I64_EQ:
      case OPCode::I64_NE:
      case OPCode::I64_LT_S:
      case OPCode::I64_LT_U:
      case OPCode::I64_GT_S:
      case OPCode::I64_GT_U:
      case OPCode::I64_LE_S:
      case OPCode::I64_LE_U:
      case OPCode::I64_GE_S:
      case OPCode::I64_GE_U:
      case OPCode::I32_CLZ:
      case OPCode::I32_CTZ:
      case OPCode::I32_POPCNT:
      case OPCode::I32_ADD:
      case OPCode::I32_SUB:
      case OPCode::I32_MUL:
      case OPCode::I32_DIV_S:
      case OPCode::I32_DIV_U:
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
      case OPCode::I64_CTZ:
      case OPCode::I64_POPCNT:
      case OPCode::I64_ADD:
      case OPCode::I64_SUB:
      case OPCode::I64_MUL:
      case OPCode::I64_DIV_S:
      case OPCode::I64_DIV_U:
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
    uint32_t const postParseFuncBROffset = br_.getOffset();
    assert((postParseFuncBROffset - preParseFuncBROffset) == funcBodySize);

    funcBody.ins = std::move(instructions);
    module_.functionInfos_.push_back(std::move(funcBody));
    assert(module_.functionInfos_.size() == funcSignatureIndex + 1U);

    if (stackUsage != 0U) {
      backend_.emit.append(inc_sp(stackUsage));
    }

    if (funcTypeInfo.results.size() == 1U) {
      assert(validationStack.size() == 1U && "validation stack should have one element for return value");
      assert(OperandStack::toWasmType(validationStack.top()) == funcTypeInfo.results[0] && "validation stack top should be the return value type");
      validationStack.pop();

      // prepare return value
      op.subROP(funcTypeInfo.results[0] == WasmType::I64);
      backend_.emit.append(ldr_simm_ar2r(REG::R0, REG::R28, 0U, false));
    }
    assert(validationStack.empty() && "validation stack should be empty after parsing function body");
    OPCodeTemplate const insRET = 0xd65f03c0; // big endian
    backend_.emit.append(insRET);
  }

  assert(funcNumbers == module_.functionInfos_.size() && "parse code functionBodys exception");
}
void Frontend::parseNameSection() {
  // uint32_t const stringLength = br_.readLEB128<uint32_t>();
  // std::string name{};
  // for (uint32_t i = 0; i < stringLength; i++) {
  //   name += static_cast<char>(br_.readByte());
  // }
}

void Frontend::logParsedInfo() {
  LOGGER << "========================= type section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < module_.type_.size(); i++) {
    LOGGER << "type[" << i << "] params num = " << module_.type_[i].params.size() << " result num = " << module_.type_[i].results.size()
           << LOGGER_END;
  }
  LOGGER << "========================= func section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < module_.func_.size(); i++) {
    LOGGER << "func[" << i << "] signatureIndex = " << module_.func_[i].signatureIndex << LOGGER_END;
  }
  LOGGER << "========================= export section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < module_.export_.size(); i++) {
    LOGGER << "export[" << i << "] name:\"" << module_.export_[i].exportName << "\" type = " << static_cast<uint32_t>(module_.export_[i].type)
           << " index = " << module_.export_[i].funcIndex << LOGGER_END;
  }
  LOGGER << "========================= code section =========================" << LOGGER_END;
  LOGGER << "function number = " << module_.functionInfos_.size() << LOGGER_END;
  for (uint32_t i = 0; i < module_.functionInfos_.size(); i++) {
    LOGGER << "body[" << i << "] size = " << module_.functionInfos_[i].bodySize << " numLocalDecl = " << module_.functionInfos_[i].locals.size()
           << LOGGER_END;
    LOGGER << "Instructions:" << LOGGER_END;
    for (auto const &ins : module_.functionInfos_[i].ins) {
      LOGGER << static_cast<uint32_t>(ins.opCode) << LOGGER_END;
    }
  }
  LOGGER << "========================= name section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < module_.names_.size(); i++) {
    LOGGER << "name[" << i << "] name " << module_.names_[i].name << "nameSubsectionType " << module_.names_[i].nameSubsectionType << LOGGER_END;
  }
}