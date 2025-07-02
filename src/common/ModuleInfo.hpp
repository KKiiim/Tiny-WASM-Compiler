#ifndef SRC_COMMON_MODULEINFO_HPP
#define SRC_COMMON_MODULEINFO_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "wasm_type.hpp"

#include "src/common/util.hpp"

enum class SignatureType : uint8_t { I32 = 'i', I64 = 'I', F32 = 'f', F64 = 'F', PARAMSTART = '(', PARAMEND = ')' };

class ModuleInfo final {
public:
  struct TypeInfo {
    std::vector<WasmType> params;
    std::vector<WasmType> results;
    std::string signature; // multi signatureIndex(type index) may match same string(pure signature)
  };
  struct ExportInfo {
    std::string exportName;
    WasmImportExportType type;
    uint32_t funcIndex;
  };

  struct LocalInfo {
    bool isParam;
    /// @brief Offset relative to startAddressOffset of its belonging FunctionInfo
    /// if param:        init with UINT32_MAX as placeholder
    /// if normal local: init with the actual offset
    uint32_t offset;
    WasmType type;
  };
  struct FunctionInfo {
    std::vector<LocalInfo> locals; // params, then locals
    uint32_t belongingBlockIndex;  // index of stack element
  };
  struct NameInfo {
    std::string name;
    uint32_t nameSubsectionType;
  };

  static SignatureType wasmType2SignatureType(WasmType const type);
  bool validateSignature(uint32_t const functionIndex, std::string const &signature) const;
  inline TypeInfo const &getTypeInfo(uint32_t const functionIndex) const {
    return typeInfo_[funcIndex2TypeIndex_[functionIndex]];
  }

  // Parsed from code section: functionIndex i to FunctionInfo
  std::vector<FunctionInfo> functionInfos_;
  // Parsed from type section: funcType index to TypeInfo details
  std::vector<TypeInfo> typeInfo_;
  // Parsed from function section: functionIndex i to signatureIndex(as type index)
  std::vector<uint32_t> funcIndex2TypeIndex_;

  std::vector<ExportInfo> export_;
  std::unordered_map<std::string, uint32_t> exportFuncNameToIndex_;

  std::vector<NameInfo> names_;

  ///< Table
  bool hasTable = false;
  bool tableHasSizeLimit = false;
  uint32_t numberElements{};
  uint32_t tableInitialSize{};
  uint32_t tableMaximumSize{};

  ///< Call
  /// @brief 1-1 Map from string view function signature to pure signature index
  class SignatureMap {
  public:
    // setPureSignatureIndex
    void set(std::string const &signatureString);
    // getPureSignatureIndex
    uint32_t get(std::string const &signatureString) const;

  private:
    std::unordered_map<std::string, uint32_t> strToPureIndex;
  };

  SignatureMap signatureStringToPureSigIndex; // 1-1 mapping

  ///< Global
  struct GlobalInfo {
    bool is64bit;
    uint32_t offset; ///< Offset in global memory. Invalid if not mutable
    bool isMutable;
    ConstUnion value; // if mutable, this is always the initial value
  };
  std::vector<GlobalInfo> globalManager; ///< Global info by global index
};

#endif
