#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/util.hpp"
#include "common/wasm_type.hpp"
#include "compiler.hpp"

Compiler::Compiler(std::string const &wasmPath) {
  br_.readWasmBinary(wasmPath);
}

ExecutableMemory Compiler::startCompilation() {
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

  // compile();
  std::cout << "compile to machine code success" << std::endl;

  return backend_.emit.getExecutableMemory();
}

void Compiler::validateMagicNumber() {
  constexpr std::array<uint8_t, 5U> wasmBinaryMagic{{0U, 0x61U, 0x73U, 0x6DU, 0U}};
  static_assert((wasmBinaryMagic.size() - 1U) == 4U, "Binary magic length needs to be four");
  while (br_.getOffset() < (wasmBinaryMagic.size() - 1U)) {
    if (wasmBinaryMagic[br_.getOffset()] != br_.readByte<uint8_t>()) {
      throw std::runtime_error("Wrong_Wasm_magic_number");
    }
  }
}

void Compiler::validateVersion() {
  uint32_t const moduleWasmVersion{br_.readLEU32()};
  constexpr uint32_t supportedWasmVersion{1U};

  if (moduleWasmVersion != supportedWasmVersion) {
    throw std::runtime_error("Wasm_Version_not_supported");
  }
}
void Compiler::parseTypeSection() {
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

    type_.push_back({paramInfos, resultInfos});
  }
}
void Compiler::parseFunctionSection() {
  uint32_t const funcNumbers{br_.readLEB128<uint32_t>()};
  uint32_t counter = 0U;
  while (counter < funcNumbers) {
    counter++;

    uint32_t const index = br_.readLEB128<uint32_t>();
    func_.push_back({index});
  }
}
void Compiler::parseExportSection() {
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
    export_.push_back({exportName, type, index});
  }
}
void Compiler::parseCodeSection() {
  uint32_t const funcNumbers{br_.readLEB128<uint32_t>()};
  uint32_t counter = 0U;

  // parse each function body
  while (counter < funcNumbers) {
    counter++;

    FunctionBody funcBody{};
    uint32_t const funcBodySize = br_.readLEB128<uint32_t>();
    funcBody.bodySize = funcBodySize;

    uint32_t const preParseFuncBROffset = br_.getOffset();

    uint32_t const localDeclCount = br_.readLEB128<uint32_t>();
    uint32_t localDeclIndex = 0U;
    while (localDeclIndex++ < localDeclCount) {
      uint32_t const localTypeCount = br_.readLEB128<uint32_t>();
      assert((localTypeCount == 1U) && "only support one type local");

      WasmType const localType = br_.readByte<WasmType>();
      uint32_t const localOffset = backend_.lm.add();

      funcBody.localDecls.push_back({false, localOffset, localType});
    }

    std::vector<WasmInstruction> instructions{};
    while (true) {
      // TODO(): support other elements in ins
      WasmInstruction ins{};
      OPCode const opCode = br_.readByte<OPCode>();
      if (opCode == OPCode::END) {
        break;
      }
      ins.opCode = opCode;
      instructions.push_back(ins);

      backend_.emit.append(opCode);
    }
    uint32_t const postParseFuncBROffset = br_.getOffset();
    assert((postParseFuncBROffset - preParseFuncBROffset) == funcBodySize);

    funcBody.ins = std::move(instructions);
    codeFunctionBodys_.push_back(std::move(funcBody));
  }

  assert(funcNumbers == codeFunctionBodys_.size() && "parse code functionBodys exception");
}
void Compiler::parseNameSection() {
  // uint32_t const stringLength = br_.readLEB128<uint32_t>();
  // std::string name{};
  // for (uint32_t i = 0; i < stringLength; i++) {
  //   name += static_cast<char>(br_.readByte());
  // }
}
// void Compiler::compile() {
//   for (auto const &func : codeFunctionBodys_) {
//     static_cast<void>(func.bodySize);
//     // not supported yet
//     static_cast<void>(func.localDeclCount);
//   }
// }

void Compiler::logParsedInfo() {
  LOGGER << "========================= type section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < type_.size(); i++) {
    LOGGER << "type[" << i << "] params num = " << type_[i].params.size() << " result num = " << type_[i].results.size() << LOGGER_END;
  }
  LOGGER << "========================= func section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < func_.size(); i++) {
    LOGGER << "func[" << i << "] signatureIndex = " << func_[i].signatureIndex << LOGGER_END;
  }
  LOGGER << "========================= export section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < export_.size(); i++) {
    LOGGER << "export[" << i << "] name:\"" << export_[i].exportName << "\" type = " << static_cast<uint32_t>(export_[i].type)
           << " index = " << export_[i].index << LOGGER_END;
  }
  LOGGER << "========================= code section =========================" << LOGGER_END;
  LOGGER << "function number = " << codeFunctionBodys_.size() << LOGGER_END;
  for (uint32_t i = 0; i < codeFunctionBodys_.size(); i++) {
    LOGGER << "body[" << i << "] size = " << codeFunctionBodys_[i].bodySize << " numLocalDecl = " << codeFunctionBodys_[i].localDecls.size()
           << LOGGER_END;
    LOGGER << "Instructions:" << LOGGER_END;
    for (auto const &ins : codeFunctionBodys_[i].ins) {
      LOGGER << static_cast<uint32_t>(ins.opCode) << LOGGER_END;
    }
  }
  LOGGER << "========================= name section =========================" << LOGGER_END;
  for (uint32_t i = 0; i < names_.size(); i++) {
    LOGGER << "name[" << i << "] name " << names_[i].name << "nameSubsectionType " << names_[i].nameSubsectionType << LOGGER_END;
  }
}