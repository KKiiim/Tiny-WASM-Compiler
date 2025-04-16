#ifndef SRC_COMMON_MODULEINFO_HPP
#define SRC_COMMON_MODULEINFO_HPP

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "wasm_type.hpp"

enum class SignatureType : uint8_t { I32 = 'i', I64 = 'I', F32 = 'f', F64 = 'F', PARAMSTART = '(', PARAMEND = ')' };

enum class StorageType : uint8_t { STACKMEMORY, LINKDATA, REGISTER, CONSTANT, INVALID };

class ModuleInfo final {
public:
  // TODO(): not sure about the size of infos
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
    uint32_t index;
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
  struct FunctionBody {
    uint32_t bodySize; // does not contain itself
    std::vector<LocalInfo> localDecls;
    std::vector<WasmInstruction> ins;
  };
  struct NameInfo {
    std::string name;
    uint32_t nameSubsectionType;
  };

  class LocalVar final {
  public:
    WasmType wasmType = WasmType::INVALID;
  };

  class FuncParm final {
  public:
    WasmType wasmType = WasmType::INVALID;
  };

  class FunctionInfo final {
  public:
    uint32_t typeIndex = 0U;
    uint32_t numParams = 0U;
    uint32_t numLocals = 0U;

    std::vector<LocalVar> functionsLocalVars{};
    std::vector<FuncParm> functionsParams{};
    std::vector<uint8_t> functionsInstructions{};
  };

  size_t functionNums = 0;

  std::vector<std::string> signatureTypes;

  std::vector<FunctionInfo> functionInfos;

  std::vector<TypeInfo> type_{};
  std::vector<FuncInfo> func_{};
  std::vector<ExportInfo> export_{};
  std::vector<FunctionBody> codeFunctionBodys_{};
  std::vector<NameInfo> names_{};
};

#endif
