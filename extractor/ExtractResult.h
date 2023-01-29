#pragma once

#include "typedefs.h"
#include "core/Key.h"
#include "core/Value.h"

struct StringInFile {
    StringInFile(const std::string &str_, size_t offset_in_file_) : str(str_), offset_in_file(offset_in_file_) {}

    std::string prefix;
    std::string str; // a line or a word.
    size_t offset_in_file;
};
using StringInFiles = std::vector<StringInFile>;

// when is_valid == true:
// WordExtractor 只填充 words;
// JsonExtractor,CsvExtractor 填充 words, kvs;
struct ExtractResult {
    bool is_valid = false; // is not eof, and has meaningful data
    StringInFiles words;
    std::unordered_map<Key, Value> kvs;
};
