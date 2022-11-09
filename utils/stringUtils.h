#pragma once

#include "typedefs.h"

/// Removes all leading and trailing whitespace/unprintable in str.
/// return trimmed chars number of characters from left/from right.
template<class S>
std::pair<std::ptrdiff_t, std::ptrdiff_t> trimInPlace(S &str)
{
    std::ptrdiff_t first = 0;
    std::ptrdiff_t last = static_cast<std::ptrdiff_t>(str.size()) - 1;

    while (first <= last &&
           (Poco::Ascii::isSpace(str[first]) || !Poco::Ascii::isPrintable(str[first]))
            )
        ++first;
    while (last >= first &&
           (Poco::Ascii::isSpace(str[last]) || !Poco::Ascii::isPrintable(str[last]))
            )
        --last;

    if (last >= 0) {
        str.resize(last + 1);
        str.erase(0, first);
    }

    assert(first >= 0);
    return {first, static_cast<std::ptrdiff_t>(str.size()) - 1 - last};
}

std::string output_smooth(const std::string& output)
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