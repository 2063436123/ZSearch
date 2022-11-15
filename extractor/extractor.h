#pragma once

#include "typedefs.h"
#include "storage/Reader.h"

class Extractor {
public:
    virtual std::tuple<StringInFiles, > extract(const StringInFile &line) = 0;
    virtual ~Extractor() = default;
};

class WordExtractor : public Extractor {
public:
    StringInFiles extract(const StringInFile &sif)
    {
        StringInFiles res;

        // this split logic is referred to Poco::StringTokenizer.
        auto str = sif.str;
        std::string separators(" ,.\t\n");
        size_t offset_in_file = sif.offset_in_file;

        auto begin = str.begin(), end = str.end();
        auto it = str.begin();

        std::string token;
        for (; it != end; ++it)
        {
            if (separators.find(*it) != std::string::npos)
            {
                size_t old_token_size = token.size();
                size_t left_trim_number = trimInPlace(token).first;
                if (!token.empty()) res.emplace_back(token, offset_in_file + (it - begin) - old_token_size + left_trim_number);
                token.clear();
            }
            else
            {
                token += *it;
            }
        }

        if (!token.empty())
        {
            size_t old_token_size = token.size();
            size_t left_trim_number = trimInPlace(token).first;
            if (!token.empty()) res.emplace_back(token, offset_in_file + (it - begin) - old_token_size + left_trim_number);
        }

        return res;
    }
};

class CSVExtractor : public Extractor {
public:
    StringInFiles extract(const StringInFile &line) override
    {

    }
};