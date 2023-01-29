#pragma once

#include <fstream>
#include "typedefs.h"
#include "utils/StringUtils.h"
#include "extractor/ExtractResult.h"

class Reader
{
public:
    virtual std::istream& reset() = 0;

    virtual StringInFile readUntil(const std::string &endSymbols = "\n") = 0;

    virtual Names getFieldNames() const = 0;

    explicit Reader(const std::filesystem::path &path_)
    {
        if (!std::filesystem::exists(path_))
            THROW(Poco::FileNotFoundException());
    }

    virtual ~Reader() = default;
};

class TxtLineReader : public Reader
{
public:
    TxtLineReader(std::filesystem::path path) : Reader(path)
    {
        if (!std::filesystem::is_regular_file(path))
            THROW(FileTypeUnmatchException("file of " + path.string() + " is not a regular file"));
        fin.open(path);
    }

    std::ifstream& reset() override
    {
        // 必须调用 clear，因为第一遍读到了 eof
        fin.clear();
        fin.seekg(0);
        offset_in_file = 0;
        return fin;
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
                left_trim_number = trimInPlace(line, [](char ch) {
                    return Poco::Ascii::isSpace(ch) || !Poco::Ascii::isPrintable(ch);
                }).first;
                offset_in_file += read_number;
            }
            return {line, offset_in_file - read_number + left_trim_number};
        }
        else
            THROW(Poco::NotImplementedException());
    }

    Names getFieldNames() const override
    {
        THROW(Poco::NotImplementedException());
    }

private:
    std::ifstream fin;
    std::size_t offset_in_file = 0;
};
