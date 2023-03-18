#pragma once
#include "../typedefs.h"

enum class TokenType
{
    Whitespace,
    Comment,
    BareWord, // keyword or identifier
    Number,
    StringLiteral,
    QuotedIdentifier,
    OpeningRoundBracket, // (
    ClosingRoundBracket, // )
    OpeningSquareBracket, // [
    ClosingSquareBracket, // ]
    OpeningCurlyBrace, // {
    ClosingCurlyBrace, // }
    Comma, // ,
    Semicolon, // ;
    Dot, // . -> for json key
    Asterisk, // * -> multiplication operator or on it's own "SELECT *"
    Slash, // /
    Plus, // +
    Minus, // -
    Equals, // =
    NotEquals, // !=
    Less, // <
    Greater, // >
    LessOrEquals, // <=
    GreaterOrEquals, // >=
    Concatenation, // ||

    EndOfStream,

    Error
};

struct Token
{
    TokenType type;
    const char *begin;
    const char *end;

    std::string toString()
    {
        if (!begin)
            return "";
        return std::string(begin, end);
    }

    size_t size() const { return end - begin; }

    Token() = default;
    Token(TokenType type_, const char *begin_, const char *end_) : type(type_), begin(begin_), end(end_) {}

    bool isSignificant() const { return type != TokenType::Whitespace && type != TokenType::Comment; }
    bool isError() const { return type > TokenType::EndOfStream; }
    bool isEnd() const { return type == TokenType::EndOfStream; }
};


// 提取 string literal，即被 quote 包围的任意字符（注意 literal 中 两个连续的引号被视为一个同类引号），例如 'A''B' -> literal: A'B
template<char quote, TokenType success_token>
Token quotedString(const char *&pos, const char *const token_begin, const char *end)
{
    ++pos;
    while (true)
    {
        pos = std::find_if(pos, end, [](char ch) {
            return ch == quote || ch == '\\';
        });
        if (pos >= end)
            return Token(TokenType::Error, token_begin, end);

        if (*pos == quote)
        {
            ++pos;
            if (pos < end && *pos == quote)
            {
                ++pos;
                continue;
            }
            return Token(success_token, token_begin, pos);
        }

        assert(*pos == '\\');
        ++pos;
        if (pos >= end)
            return Token(TokenType::Error, token_begin, pos);
        ++pos;
    }
}

class Lexer
{
public:
    Lexer(const char *begin_, const char *end_, size_t max_query_size_ = 0)
            : begin(begin_), pos(begin_), end(end_), max_query_size(max_query_size_) {}

    Token nextToken()
    {
        Token res = nextTokenImpl();
        if (max_query_size && res.end > begin + max_query_size)
            res.type = TokenType::Error;
        if (res.isSignificant())
            prev_significant_token_type = res.type;
        return res;
    }

private:
    const char * const begin;
    const char * pos;
    const char * const end;

    const size_t max_query_size;
    Token nextTokenImpl()
    {
        if (pos >= end)
            return Token(TokenType::EndOfStream, end, end);
        const char * const token_begin = pos;

        switch (*pos)
        {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case '\v':
            {
                ++pos;
                while (pos < end && isspace(*pos))
                    ++pos;
                return Token(TokenType::Whitespace, token_begin, pos);
            }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                // parse number
                while (pos < end && isnumber(*pos))
                    ++pos;
                if (pos < end && *pos == '.')
                {
                    ++pos;
                    while (pos < end && isnumber(*pos))
                        ++pos;
                }
                return Token(TokenType::Number, token_begin, pos);
            }

            case '\'':
                return quotedString<'\'', TokenType::StringLiteral>(pos, token_begin, end);
            case '`':
                return quotedString<'`', TokenType::QuotedIdentifier>(pos, token_begin, end);

            case '(':
                return Token(TokenType::OpeningRoundBracket, token_begin, ++pos);
            case ')':
                return Token(TokenType::ClosingRoundBracket, token_begin, ++pos);
            case '[':
                return Token(TokenType::OpeningSquareBracket, token_begin, ++pos);
            case ']':
                return Token(TokenType::ClosingSquareBracket, token_begin, ++pos);
            case '{':
                return Token(TokenType::OpeningCurlyBrace, token_begin, ++pos);
            case '}':
                return Token(TokenType::ClosingCurlyBrace, token_begin, ++pos);
            case ',':
                return Token(TokenType::Comma, token_begin, ++pos);
            case ';':
                return Token(TokenType::Semicolon, token_begin, ++pos);

            // ambiguous: 浮点数会被优先解析，例如 x.0.1.1 结果为 x Dot 0.1 Dot 1 而非 x Dot 0 Dot 1 Dot 1
            case '.':
            {
                if (pos > begin && pos + 1 < end && (isalnum(pos[1]) || pos[1] == '_'))
                    if (prev_significant_token_type == TokenType::BareWord || prev_significant_token_type == TokenType::Number)
                        return Token(TokenType::Dot, token_begin, ++pos);
                return Token(TokenType::Error, token_begin, pos);
            }

            case '+':
                return Token(TokenType::Plus, token_begin, ++pos);
            case '-':
                return Token(TokenType::Minus, token_begin, ++pos);
            case '*':
                return Token(TokenType::Asterisk, token_begin, ++pos);
            case '/':
                return Token(TokenType::Slash, token_begin, ++pos);
            case '=':
                return Token(TokenType::Equals, token_begin, ++pos);
            case '!':
            {
                ++pos;
                if (pos < end && *pos == '=')
                    return Token(TokenType::NotEquals, token_begin, ++pos);
                return Token(TokenType::Error, token_begin, pos);
            }
            case '<':
            {
                ++pos;
                if (pos < end && *pos == '=')
                    return Token(TokenType::LessOrEquals, token_begin, ++pos);
                return Token(TokenType::Less, token_begin, pos);
            }
            case '>':
            {
                ++pos;
                if (pos < end && *pos == '=')
                    return Token(TokenType::GreaterOrEquals, token_begin, ++pos);
                return Token(TokenType::Greater, token_begin, pos);
            }

            default:
            {
                if (isalnum(*pos) || *pos == '_')
                {
                    ++pos;
                    while (pos < end && (isalnum(*pos) || *pos == '_'))
                        ++pos;
                    return Token(TokenType::BareWord, token_begin, pos);
                }
            }
        }
        return Token(TokenType::Error, token_begin, pos);
    }

    /// This is needed to disambiguate tuple access operator from floating point number (.1).
    TokenType prev_significant_token_type = TokenType::Whitespace;
};