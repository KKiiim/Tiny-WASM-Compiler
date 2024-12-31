
#include <cstdint>
#include <string>
#include <vector>

#include "byteCodeReader.hpp"

#define LOGGER std::cout
#define LOGGER_END std::endl
class Parser {
public:
  explicit Parser(std::string const &wasmPath);

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

private:
  ///
  /// @brief WebAssembly section IDs
  ///
  enum class SectionType : uint8_t {
    CUSTOM = 0,
    TYPE,
    IMPORT,
    FUNCTION,
    TABLE,
    MEMORY,
    GLOBAL,
    EXPORT,
    START,
    ELEMENT,
    CODE,
    DATA,
    DATA_COUNT,
    PLACEHOLDER ///< module end
  };
  enum class WasmImportExportType : uint8_t { FUNC = 0x00, TABLE = 0x01, MEM = 0x02, GLOBAL = 0x03 };

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
    uint8_t opCode;
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
};