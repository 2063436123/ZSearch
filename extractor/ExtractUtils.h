#pragma once
#include "storage/Reader.h"
#include "core/Value.h"
#include "utils/JsonUtils.h"

void extractWords(const std::unique_ptr<Reader>& reader, StringInFiles& res, std::string separators = " ,.\t\n")
{
    reader->reset();
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
void extractKvs(const std::unique_ptr<Reader>& reader, std::unordered_map<Key, Value>& res)
{
    std::unordered_map<Key, nlohmann::json> data = flattenJsonKvs(reader->reset());

    for (const auto&[key, value] : data)
    {
        if (value.is_boolean())
        {
            res.emplace(key, value.get<bool>());
        }
        else if (value.is_string())
        {
            res.emplace(key, value.get<std::string>());
        }
        else if (value.is_number())
        {
            res.emplace(key, value.get<double>());
        }
        else if (value.is_array() && !value.empty()) // 空数组没有意义，经 flatten 后数组元素已经是同一种类型
        {
            ValueType type;
            if (value[0].is_number())
                type = ValueType::Number;
            else if (value[0].is_string())
                type = ValueType::String;
            else
                THROW(Poco::NotImplementedException());

            Value v(ArrayLabel{}, type);

            ValueArrayHandler<Number> hn;
            ValueArrayHandler<String> hs;
            ValueArrayHandler<DateTime> hd = PanicValueArrayHandler<DateTime>;

            if (type == ValueType::Number)
            {
                auto vec0 = value.get<std::vector<Number>>();
                hn = [&vec0](std::vector<Number>* vec) {
                    vec->insert(vec->end(), vec0.begin(), vec0.end());
                };
            }
            else if (type == ValueType::String)
            {
                auto vec0 = value.get<std::vector<String>>();
                hs = [&vec0](std::vector<String>* vec) {
                    vec->insert(vec->end(), vec0.begin(), vec0.end());
                };
            }
            else
                THROW(Poco::NotImplementedException());

            v.doArrayHandler(hn, hs, hd);
            res.emplace(key, v);
        }
        else
        {
            THROW(Poco::NotImplementedException());
        }
    }
}