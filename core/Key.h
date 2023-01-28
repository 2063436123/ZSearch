#pragma once

#include "../typedefs.h"
#include "Poco/Hash.h"

// 使用 Key 来表示 a.b.c 这种 json 中的子对象名
class Key
{
public:
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
        std::string ret;
        for (int i = 0; i < tokens.size() - 1; i++)
            ret += tokens[i] + '.';
        if (!tokens.empty())
            ret += tokens.back();
        return ret;
    }

    bool operator==(const Key& rhs) const
    {
        return tokens == rhs.tokens;
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