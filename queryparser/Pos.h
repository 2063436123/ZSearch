#pragma once
#include "Lexer.h"
#include <compare>

/** Parser operates on lazy stream of tokens.
  * It could do lookaheads of any depth.
  */

/** Used as an input for parsers.
  * All whitespace and comment tokens are transparently skipped.
  */
class Tokens
{
private:
    std::vector<Token> data;
    Lexer lexer;

public:
    Tokens(const char* begin, const char* end, size_t max_query_size = 0) : lexer(begin, end, max_query_size) {}

    const Token& operator[] (size_t index)
    {
        while (true)
        {
            if (index < data.size())
                return data[index];

            if (!data.empty() && data.back().isEnd())
                return data.back();

            Token token = lexer.nextToken();
            if (token.isSignificant())
                data.emplace_back(token);
        }
    }

    const Token& max()
    {
        if (data.empty())
            return (*this)[0];
        return data.back();
    }
};

/// To represent position in a token stream.
class TokenIterator
{
private:
    Tokens *tokens;
    size_t index = 0;

public:
    explicit TokenIterator(Tokens & tokens_) : tokens(&tokens_) {}

    const Token & get() { return (*tokens)[index]; }
    const Token & operator*() { return get(); }
    const Token * operator->() { return &get(); }

    TokenIterator & operator++()
    {
        ++index;
        return *this;
    }
    TokenIterator & operator--()
    {
        --index;
        return *this;
    }

    bool operator<(const TokenIterator & rhs) const { return index < rhs.index; }
    bool operator<=(const TokenIterator & rhs) const { return index <= rhs.index; }
    bool operator==(const TokenIterator & rhs) const { return index == rhs.index; }
    bool operator!=(const TokenIterator & rhs) const { return index != rhs.index; }

    bool isValid() { return get().type < TokenType::EndOfStream; }

    /// Rightmost token we had looked.
    const Token & max() { return tokens->max(); }
};

/// Token iterator augmented with depth information. This allows to control recursion depth.
struct Pos : TokenIterator
{
    uint32_t depth = 0;
    uint32_t max_depth = 0;

    Pos(Tokens &tokens_, uint32_t max_depth_) : TokenIterator(tokens_), max_depth(max_depth_) {}

    void increaseDepth()
    {
        ++depth;
        if (max_depth > 0 && depth > max_depth)
            THROW(Poco::LogicException("Maximum parse depth (" + std::to_string(max_depth) + ") exceeded. Consider rising max_parser_depth parameter."));
    }

    void decreaseDepth()
    {
        if (depth == 0)
            THROW(Poco::LogicException("Logical error in parser: incorrect calculation of parse depth"));
        --depth;
    }
};
