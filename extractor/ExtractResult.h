#pragma once

#include "typedefs.h"
#include "relational/Table.h"

struct StringInFile {
    StringInFile(const std::string &str_, size_t offset_in_file_) : str(str_), offset_in_file(offset_in_file_) {}

    std::string prefix;
    std::string str; // a line or a word.
    size_t offset_in_file;
};
using StringInFiles = std::vector<StringInFile>;

// when is_valid == true:
// WordExtractor 只填充 words;
// JsonExtractor 只填充 words, kvs;
// CsvExtractor 只填充 table.
struct ExtractResult {
    bool is_valid = false; // is not eof, and has meaningful data
    StringInFiles words;
    std::unordered_map<std::string, Value> kvs;
    Table table;
};
