#pragma once

#include "../typedefs.h"
#include "relational/Value.h"

enum class SyntaxType
{
    Empty,
    Keyword,
    Identifier,
    Value
};

enum class KeyWordType
{
    L_Bracket,
    R_Bracket,
    Comma,
    Semicolon,
    And,
    Or,
    Not,
};

bool isLogicPredicate(KeyWordType type)
{
    return type == KeyWordType::And || type == KeyWordType::Or || type == KeyWordType::Not;
}

struct Token
{
    SyntaxType type = SyntaxType::Empty;
    std::variant<KeyWordType, IdentifierName, Value> var;
};
using TokenPtr = std::shared_ptr<Token>;

static bool isPunct(char ch)
{
    return ch == '(' || ch == ')' || ch == ',' || ch == ';';
}

bool isIllegalChar(char ch)
{
    return !isPunct(ch) && !Poco::Ascii::isAlphaNumeric(ch) && !Poco::Ascii::isSpace(ch);
}

bool isLegalChar(char ch)
{
    return Poco::Ascii::isAlphaNumeric(ch) || ch == '\'' || ch == '_';
}

Token getToken(const char*& begin, const char* end)
{
    while (Poco::Ascii::isSpace(*begin) && begin < end)
        ++begin;
    if (begin == end)
        return Token{};

    if (isPunct(*begin))
    {
        char ch = *begin++;
        if (ch == '(')
            return Token{.type = SyntaxType::Keyword, .var = KeyWordType::L_Bracket};
        if (ch == ')')
            return Token{.type = SyntaxType::Keyword, .var = KeyWordType::R_Bracket};
        if (ch == ',')
            return Token{.type = SyntaxType::Keyword, .var = KeyWordType::Comma};
        if (ch == ';')
            return Token{.type = SyntaxType::Keyword, .var = KeyWordType::Semicolon};
    }


    const char* word_begin = begin;
    while (isLegalChar(*begin) && begin < end)
        ++begin;
    assert (begin != word_begin);

    std::string word(word_begin, begin - word_begin);

    if (word.size() >= 2 && word[0] == '\'' && word.back() == '\'')
        return Token{.type = SyntaxType::Value, .var = Value(trimQuote(word))};
    if (std::all_of(word.begin(), word.end(), [](char ch){return Poco::Ascii::isDigit(ch);}))
        return Token{.type = SyntaxType::Value, .var = Value(restrictStoi<int64_t>(word))};
    // TODO: add Decimal, DateTime type Value

    if (word == "AND")
        return Token{.type = SyntaxType::Keyword, .var = KeyWordType::And};
    if (word == "OR")
        return Token{.type = SyntaxType::Keyword, .var = KeyWordType::Or};
    if (word == "NOT")
        return Token{.type = SyntaxType::Keyword, .var = KeyWordType::Not};

    return Token{.type = SyntaxType::Identifier, .var = IdentifierName::New(word)};
}