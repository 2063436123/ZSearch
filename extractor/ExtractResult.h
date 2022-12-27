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

struct ExtractResult {
    bool is_valid = false; // is not eof, and has meaningful data
    StringInFiles words;
    Table table;
};
