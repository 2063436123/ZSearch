#include "typedefs.h"
#include "relational/Table.h"

struct StringInFile {
    StringInFile(const std::string &str_, size_t offset_in_file_) : str(str_), offset_in_file(offset_in_file_) {}

    std::string prefix;
    std::string str; // a line or a word.
    size_t offset_in_file;
};
using StringInFiles = std::vector<StringInFile>;


union ExtractResult {
    StringInFiles words;
    Table table;
};
