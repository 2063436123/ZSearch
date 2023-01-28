#pragma once
#include "storage/Reader.h"

void extractWords(const std::unique_ptr<Reader>& reader, StringInFiles& res, std::string separators = " ,.\t\n")
{
    while(true)
    {
        auto line = reader->readUntil();
        if (line.str.empty())
            break;

        // this split logic is referred to Poco::StringTokenizer.
        auto str = line.str;
        size_t offset_in_file = line.offset_in_file;

        auto begin = str.begin(), end = str.end();
        auto it = str.begin();

        std::string token;
        for (; it != end; ++it)
        {
            if (separators.find(*it) != std::string::npos)
            {
                size_t old_token_size = token.size();
                size_t left_trim_number = trimInPlace(token, [](char ch) {
                    return Poco::Ascii::isSpace(ch) || !Poco::Ascii::isPrintable(ch);
                }).first;
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
            size_t left_trim_number = trimInPlace(token, [](char ch) {
                return Poco::Ascii::isSpace(ch) || !Poco::Ascii::isPrintable(ch);
            }).first;
            if (!token.empty()) res.emplace_back(token, offset_in_file + (it - begin) - old_token_size + left_trim_number);
        }
    }
}

// for json format
void extractKvs(const std::unique_ptr<Reader>& reader, StringInFiles& res)
{

}