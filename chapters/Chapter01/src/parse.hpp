
#include <cstdint>
#include <string>
#include <vector>

#include "byteCodeReader.hpp"

class Parser {
public:
  explicit Parser(std::string const &wasmPath);

  void startCompilation();

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
  struct TypeInfo {
    uint32_t paramsNum;
    uint32_t resultsNum;
  };
  struct FuncInfo {
    uint32_t signatureIndex;
  };
  struct ExportInfo {
    std::string exportName;
    uint32_t exportKind;
    uint32_t exportFuncIndex;
  };
  struct CodeInfo {};
  struct NameInfo {
    std::string name;
    uint32_t nameSubsectionType;
  };
  void validateMagicNumber();
  void validateVersion();
  void parseTypeSection();
  void parseFunctionSection();
  void parseExportSection();
  void parseCodeSection();
  void parseNameSection();

private:
  ByteCodeReader br_{};

  std::vector<TypeInfo> type_{};
  std::vector<FuncInfo> func_{};
  std::vector<ExportInfo> export_{};
  std::vector<CodeInfo> code_{};
  std::vector<NameInfo> names_{};
};