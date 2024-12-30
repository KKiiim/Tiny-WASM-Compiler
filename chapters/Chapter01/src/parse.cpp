#include <array>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "parse.hpp"
#include "util.hpp"

Parser::Parser(std::string const &wasmPath) {
  br_.readWasmBinary(wasmPath);
}

void Parser::startCompilation() {
  validateMagicNumber();
  validateVersion();
  std::cout << "validate success" << std::endl;

  while (br_.hasNextByte()) {
    SectionType const sectionType{br_.readByte()};
    uint32_t const sectionSize{br_.readByte()}; // FIXME(): ignore invalid size check
    switch (sectionType) {
    case SectionType::CUSTOM:
      break;
    case SectionType::TYPE:
      parseTypeSection();
      break;
    case SectionType::IMPORT:
      break;
    case SectionType::FUNCTION:
      parseFunctionSection();
      break;
    case SectionType::TABLE:
      break;
    case SectionType::MEMORY:
      break;
    case SectionType::GLOBAL:
      break;
    case SectionType::EXPORT:
      break;
    case SectionType::START:
      break;
    case SectionType::ELEMENT:
      break;
    case SectionType::CODE:
      break;
    case SectionType::DATA:
      break;
    case SectionType::DATA_COUNT:
      break;
    case SectionType::PLACEHOLDER:
      break;
    }
  }
}

void Parser::validateMagicNumber() {
  constexpr std::array<uint8_t, 5U> wasmBinaryMagic{{0U, 0x61U, 0x73U, 0x6DU, 0U}};
  static_assert((wasmBinaryMagic.size() - 1U) == 4U, "Binary magic length needs to be four");
  while (br_.getOffset() < (wasmBinaryMagic.size() - 1U)) {
    if (wasmBinaryMagic[br_.getOffset()] != br_.readByte()) {
      throw std::runtime_error("Wrong_Wasm_magic_number");
    }
  }
}

void Parser::validateVersion() {
  uint32_t const moduleWasmVersion{br_.readLEU32()};
  constexpr uint32_t supportedWasmVersion{1U};

  if (moduleWasmVersion != supportedWasmVersion) {
    throw std::runtime_error("Wasm_Version_not_supported");
  }
}
void Parser::parseTypeSection() {
  uint32_t const typeNumbers{br_.readByte()};
  uint32_t counter = 0U;
  while (typeNumbers != counter) {
    counter++;

    uint32_t const typeType = br_.readByte();
    if (typeType != 0x60U) {
      throw std::runtime_error("type_section_wrong_type");
    }

    uint32_t const paramsNum = br_.readByte();
    uint32_t const resNum = br_.readByte();
    type_.push_back({paramsNum, resNum});
  }
}
void Parser::parseFunctionSection() {
  uint32_t const funcNumbers{br_.readByte()};
  uint32_t counter = 0U;
  while (funcNumbers != counter) {
    counter++;

    uint32_t const index = br_.readByte();
    func_.push_back({index});
  }
}
void Parser::parseExportSection() {
  uint32_t const exportNumbers{br_.readByte()};
  uint32_t counter = 0U;
  while (exportNumbers != counter) {
    counter++;

    uint32_t const stringLength = br_.readByte();
    std::string exportName{};
    for (uint32_t i = 0; i < stringLength; i++) {
      exportName += static_cast<char>(br_.readByte());
    }
    uint32_t const kind = br_.readByte();
    uint32_t const funcIndex = br_.readByte();
    export_.push_back({exportName, kind, funcIndex});
  }
}
void Parser::parseCodeSection() {
  uint32_t const funcNumbers{br_.readByte()};
  uint32_t counter = 0U;
  // parse each function body
  while (funcNumbers != counter) {
    counter++;

    uint32_t const funcBodySize = br_.readByte(); // FIXME(): check it
    br_.jump(funcBodySize);                       // TODO(): parse code body
    //  uint32_t const localDeclCount = br_.readByte();
  }
}
void Parser::parseNameSection() {
  uint32_t const stringLength = br_.readByte();
  std::string name{};
  for (uint32_t i = 0; i < stringLength; i++) {
    name += static_cast<char>(br_.readByte());
  }
}