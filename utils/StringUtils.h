#pragma once

#include "typedefs.h"

/// Removes all leading and trailing whitespace/unprintable in str.
/// return trimmed chars number of characters from left/from right.
template<class S>
std::pair<std::ptrdiff_t, std::ptrdiff_t> trimInPlace(S &str, const std::function<bool(char)>& isUseless)
{
    std::ptrdiff_t first = 0;
    std::ptrdiff_t last = static_cast<std::ptrdiff_t>(str.size()) - 1;

    while (first <= last && isUseless(str[first]))
        ++first;
    while (last >= first && isUseless(str[last]))
        --last;

    if (last >= 0) {
        str.resize(last + 1);
        str.erase(0, first);
    }

    assert(first >= 0);
    return {first, static_cast<std::ptrdiff_t>(str.size()) - 1 - last};
}

// replaces all newline characters in output with the string "\\n"
std::string outputSmooth(const std::string& output)
{
    std::string res;
    for (auto ch : output)
    {
        if (ch == '\n')
            res += "\\n";
        else
            res.push_back(ch);
    }
    return res;
}

// 去除一个输入字符串 str 前后的引号（单引号或双引号）
std::string trimQuote(std::string str)
{
    if (str.empty())
        return str;
    if (str[0] == '\"')
        trimInPlace(str, [](char ch) {
            return ch == '\"';
        });
    else if (str[0] == '\'')
        trimInPlace(str, [](char ch) {
            return ch == '\'';
        });
    return str;
}

// only [-]<number>+ is allowed
template <typename T>
T restrictStoi(const std::string& str) {
    T base{};
    bool is_minus = false;

    if (str.empty())
        THROW(Poco::InvalidArgumentException("restrict_stoi requires an not empty string! -- " + str));

    size_t i = 0;
    if (str[0] == '-')
    {
        is_minus = true;
        ++i;
    }

    for (; i < str.size(); i++)
    {
        auto ch = str[i];
        if (!Poco::Ascii::isDigit(ch))
            THROW(Poco::InvalidArgumentException("restrict_stoi requires a string contains only number char! -- " + str));
        else
            base = base * 10 + ch - '0';
    }

    return is_minus ? 0 - base : base;
}

double restrictStod(const std::string& str) {
    if (str.empty())
        THROW(Poco::InvalidArgumentException("restrict_stod requires an not empty string! -- " + str));

    size_t i = 0;
    if (str[0] == '-')
    {
        ++i;
    }

    for (; i < str.size(); i++)
    {
        auto ch = str[i];
        if (!Poco::Ascii::isDigit(ch) && ch != '.')
            THROW(Poco::InvalidArgumentException("restrict_stod requires a string contains only number char! -- " + str));
    }

    return std::stod(str);
}

bool is_valid_utf8(const std::string& str) {
    int i = 0;
    while (i < str.size()) {
        int len = 0;
        unsigned char c = str[i];
        if ((c & 0x80) == 0x00) {
            len = 1;
        } else if ((c & 0xE0) == 0xC0) {
            len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            len = 4;
        } else {
            return false;
        }
        i++;
        for (int j = 1; j < len; j++) {
            if (i >= str.size() || (str[i] & 0xC0) != 0x80) {
                return false;
            }
            i++;
        }
    }
    return true;
}

std::string fix_utf8(const std::string& str) {
    if (is_valid_utf8(str))
        return str;
    if (str.size() < 8)
        return "???utf-8???";
    int start = 3, end = str.size() - 4;

    // 将一个子字符串的前后边界尽可能少地扩展，使得新的子字符串刚好满足UTF-8编码
    int padding = 0;
    while (start >= 0 && end < str.size() && !is_valid_utf8(str.substr(start, end - start + 1))) {
        padding++;
        if ((start - padding) >= 0 && is_valid_utf8(str.substr(start - padding, end - start + 1 + padding * 2))) {
            start -= padding;
        } else if ((end + padding) < str.size() && is_valid_utf8(str.substr(start - (padding - 1), end - start + 1 + (padding - 1) * 2))) {
            end += padding;
        } else {
            return "";
        }
    }
    return str.substr(start, end - start + 1);
}