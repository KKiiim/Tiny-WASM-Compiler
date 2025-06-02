#ifndef SRC_COMMON_MODULEINFO_HPP
#define SRC_COMMON_MODULEINFO_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "wasm_type.hpp"

enum class SignatureType : uint8_t { I32 = 'i', I64 = 'I', F32 = 'f', F64 = 'F', PARAMSTART = '(', PARAMEND = ')' };

enum class StorageType : uint8_t { STACKMEMORY, LINKDATA, REGISTER, CONSTANT, INVALID };

class ModuleInfo final {
public:
  struct TypeInfo {
    std::vector<WasmType> params;
    std::vector<WasmType> results;
  };
  struct FuncInfo {
    uint32_t signatureIndex; ///< Index of the function type this function is conforming to
  };
  struct ExportInfo {
    std::string exportName;
    WasmImportExportType type;
    uint32_t funcIndex;
  };

  struct WasmInstruction {
    OPCode opCode;
    // others need supported
  };
  struct LocalInfo {
    bool isParam;
    uint32_t offset;
    WasmType type;
  };
  struct FunctionInfo {
    uint32_t bodySize; // does not contain itself
    uint32_t paramsNumber;
    std::vector<LocalInfo> locals; // params and locals
    std::vector<WasmInstruction> ins;
    uint64_t startAddressOffset; ///< Bytes offset from the start of the executable memory
  };
  struct NameInfo {
    std::string name;
    uint32_t nameSubsectionType;
  };

  SignatureType wasmType2SignatureType(WasmType const type) const;
  bool validateSignature(uint32_t const functionIndex, std::string const &signature) const;

  // Parsed from code section. functionInfos_[i]: functionIndex i to FunctionInfo
  std::vector<FunctionInfo> functionInfos_;
  // Parsed from type section. type_[i]: signatureIndex i to TypeInfo
  std::vector<TypeInfo> type_;
  // Parsed from function section. func_[i]: functionIndex i to signatureIndex(FuncInfo)
  std::vector<FuncInfo> func_;

  std::vector<ExportInfo> export_;
  std::unordered_map<std::string, uint32_t> exportFuncNameToIndex_;

  std::vector<NameInfo> names_;
};

#endif
