#include <stdexcept>
#include <string>

#include "ModuleInfo.hpp"

#include "src/common/logger.hpp"

SignatureType ModuleInfo::wasmType2SignatureType(WasmType const type) {
  switch (type) {
  case WasmType::I32:
    return SignatureType::I32;
  case WasmType::I64:
    return SignatureType::I64;
  case WasmType::F32:
    return SignatureType::F32;
  case WasmType::F64:
    return SignatureType::F64;
  default:
    throw std::runtime_error("Unsupported WasmType for signature validation");
  }
}

bool ModuleInfo::validateSignature(uint32_t const functionIndex, std::string const &signature) const {
  std::string const &expectedSignature = getTypeInfo(functionIndex).signature;
  return expectedSignature == signature;
}
void ModuleInfo::SignatureMap::set(std::string const &signatureString) {
  if (strToPureIndex.find(signatureString) == strToPureIndex.end()) {
    // set new
    uint32_t const pureSignatureIndex = strToPureIndex.size();
    strToPureIndex[signatureString] = pureSignatureIndex;
    LOG_DEBUG << "New pure signature index: " << pureSignatureIndex << " for signature: " << signatureString << LOG_END;
  }
}
uint32_t ModuleInfo::SignatureMap::get(std::string const &signatureString) const {
  LOG_DEBUG << "Get pure signature index for signature: " << signatureString << LOG_END;
  if (strToPureIndex.find(signatureString) == strToPureIndex.end()) {
    throw std::runtime_error("Signature string not found in pure signature index map");
  }
  return strToPureIndex.at(signatureString);
}