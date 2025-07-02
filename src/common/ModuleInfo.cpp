#include <stdexcept>

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
  uint32_t const signatureIndex = funcIndex2TypeIndex_[functionIndex];
  auto const &funcTypeInfo = typeInfo_[signatureIndex];
  confirm(signature.size() >= 2U, "signature must have at least 2 characters '(' and ')'");
  confirm(((funcTypeInfo.results.size() + funcTypeInfo.params.size()) == (signature.size() - 2U)),
          "Signature should match the number of parameters in the function type");

  SignatureType const signaturePrefixType = static_cast<SignatureType>(signature[0]);
  if (funcTypeInfo.results.empty()) {
    confirm(signaturePrefixType == SignatureType::PARAMSTART, "Signature should start with '(' when there are no results");
  } else {
    SignatureType const retType = wasmType2SignatureType(funcTypeInfo.results[0]);
    if (signaturePrefixType != retType) {
      throw std::runtime_error("Signature prefix type does not match function return type");
    }
  }

  uint32_t sigIndex = funcTypeInfo.results.empty() ? 1U : 2U; // Skip return type if exists, skip '('
  // check parameters
  for (uint32_t i = 0; i < funcTypeInfo.params.size(); ++i) {
    if (wasmType2SignatureType(funcTypeInfo.params[i]) == static_cast<SignatureType>(signature[sigIndex])) {
      sigIndex++;
    } else {
      LOG_ERROR << "Signature parameter type does not match function parameter type at index " << std::to_string(i) << LOG_END;
      return false;
    }
  }
  confirm(static_cast<SignatureType>(signature[sigIndex]) == SignatureType::PARAMEND, "Signature should end with ')' after parameters");
  return true;
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