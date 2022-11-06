#pragma once

#include <fstream>
#include "typedefs.h"
#include "utils.h"

struct StringInFile {
    StringInFile(const std::string &str_, size_t offset_in_file_) : str(str_), offset_in_file(offset_in_file_) {}

    std::string str; // a line or a word.
    size_t offset_in_file;
};

using StringInFiles = std::vector<StringInFile>;

class Reader {
public:
    virtual StringInFile readUntil(const std::string &endSymbols = "\n") = 0;

    virtual Names getFieldNames() const = 0;

    explicit Reader(const std::filesystem::path &path_)
    {
        if (!std::filesystem::exists(path_))
            throw Poco::FileNotFoundException();
    }

    virtual ~Reader() = default;
};

class TxtReader : public Reader {
public:
    TxtReader(std::filesystem::path path) : Reader(path)
    {
        if (!std::filesystem::is_regular_file(path))
            throw FileTypeUnmatchException("file of " + path.string() + " is not a regular file");
        fin.open(path);
    }

    // trim all space and unprintable characters.
    // return "" only when eof.
    StringInFile readUntil(const std::string &endSymbols = "\n") override
    {
        if (endSymbols == "\n")
        {
            std::string line;
            size_t read_number = 0;
            size_t left_trim_number = 0;
            while (fin && line.empty())
            {
                std::getline(fin, line);
                read_number = line.size() + 1;
                left_trim_number = trimInPlace(line).first;
                offset_in_file += read_number;
            }
            return {line, offset_in_file - read_number + left_trim_number};
        } else
            throw Poco::NotImplementedException();
    }

    Names getFieldNames() const override
    {
        throw Poco::NotImplementedException();
    }

private:
    std::ifstream fin;
    std::size_t offset_in_file = 0;
};
