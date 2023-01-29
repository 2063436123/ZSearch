#pragma once

#include "../typedefs.h"
#include "utils/SerializeUtils.h"


// 使用 Key 来表示 a.b.c 这种 json 中的子对象名
class Key
{
public:
    Key(const char* str) : Key(std::string(str)) { }

    Key(const std::string& str)
    {
        Poco::StringTokenizer tk(str, ".", Poco::StringTokenizer::Options::TOK_TRIM);
        for (const auto& token : tk)
        {
            if (token.empty())
                THROW(Poco::LogicException("Key format error"));
            tokens.push_back(token);
        }
    }

    void push_back(const std::string& token)
    {
        tokens.push_back(token);
    }

    void pop_back()
    {
        if (tokens.empty())
            THROW(Poco::RangeException());
        tokens.pop_back();
    }

    auto begin() const
    {
        return tokens.begin();
    }

    auto end() const
    {
        return tokens.end();
    }

    std::string string() const
    {
        if (tokens.empty())
            return "";
        std::string ret;
        for (int i = 0; i < tokens.size() - 1; i++)
            ret += tokens[i] + '.';
            ret += tokens.back();
        return ret;
    }

    bool operator==(const Key& rhs) const
    {
        return tokens == rhs.tokens;
    }

    void serialize(WriteBufferHelper& helper) const
    {
        helper.writeString(string());
    }

    static Key deserialize(ReadBufferHelper& helper)
    {
        return Key(helper.readString());
    }

private:
    std::vector<std::string> tokens;
};

namespace std
{
    template<>
    struct hash<Key>
    {
        std::size_t operator()(const Key& key) const
        {
            return Poco::hashRange(key.begin(), key.end());
        }
    };
}