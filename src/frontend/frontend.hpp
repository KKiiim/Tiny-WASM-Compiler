#ifndef SRC_FRONTEND_FRONTEND_H
#define SRC_FRONTEND_FRONTEND_H

#include <cstdint>
#include <string>
#include <vector>

#include "../backend/arm64Backend.hpp"
#include "../common/wasm_type.hpp"
#include "../frontend/byteCodeReader.hpp"

#define LOGGER std::cout
#define LOGGER_END std::endl
class Frontend {
public:
  explicit Frontend(std::string const &wasmPath);

  ExecutableMemory startCompilation();
  void logParsedInfo();

private:
  void validateMagicNumber();
  void validateVersion();
  void parseTypeSection();
  void parseFunctionSection();
  void parseExportSection();
  void parseCodeSection();
  void parseNameSection();

  void compile();

private:
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

private:
  BytecodeReader br_{};

  std::vector<TypeInfo> type_{};
  std::vector<FuncInfo> func_{};
  std::vector<ExportInfo> export_{};
  std::vector<FunctionBody> codeFunctionBodys_{};
  std::vector<NameInfo> names_{};

private:
  Arm64Backend backend_{};
};

#endif