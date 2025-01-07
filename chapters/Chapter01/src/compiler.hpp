
#include <cstdint>
#include <string>
#include <vector>

#include "byteCodeReader.hpp"
#include "emit.hpp"
#include "wasm_type.hpp"

#define LOGGER std::cout
#define LOGGER_END std::endl
class Compiler {
public:
  explicit Compiler(std::string const &wasmPath);

  void startCompilation();
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
    uint32_t paramsNum;
    uint32_t resultsNum;
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
  struct FunctionBody {
    uint32_t bodySize; // does not contain itself
    uint32_t localDeclCount;
    std::vector<WasmInstruction> ins;
  };
  struct NameInfo {
    std::string name;
    uint32_t nameSubsectionType;
  };

private:
  ByteCodeReader br_{};

  std::vector<TypeInfo> type_{};
  std::vector<FuncInfo> func_{};
  std::vector<ExportInfo> export_{};
  std::vector<FunctionBody> codeFunctionBodys_{};
  std::vector<NameInfo> names_{};

private:
  Emit emit_{};
};