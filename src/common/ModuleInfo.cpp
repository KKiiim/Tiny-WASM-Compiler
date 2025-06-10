#include <cassert>
#include <stdexcept>

#include "ModuleInfo.hpp"

#include "src/common/logger.hpp"

SignatureType ModuleInfo::wasmType2SignatureType(WasmType const type) const {
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
  uint32_t const signatureIndex = func_[functionIndex].signatureIndex;
  auto const &funcTypeInfo = type_[signatureIndex];
  assert(signature.size() >= 2U && "signature must have at least 2 characters '(' and ')'");
  assert(((funcTypeInfo.results.size() + funcTypeInfo.params.size()) == (signature.size() - 2U)) &&
         "Signature should match the number of parameters in the function type");

  SignatureType const signaturePrefixType = static_cast<SignatureType>(signature[0]);
  if (funcTypeInfo.results.empty()) {
    assert(signaturePrefixType == SignatureType::PARAMSTART && "Signature should start with '(' when there are no results");
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
  assert(static_cast<SignatureType>(signature[sigIndex]) == SignatureType::PARAMEND && "Signature should end with ')' after parameters");
  return true;
}