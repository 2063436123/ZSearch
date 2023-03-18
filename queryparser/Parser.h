#pragma once

#include "ParserQuery.h"

enum class QueryErrorType
{
    Non,
    Empty,
    SyntaxError
};

static std::tuple<QueryErrorType, ASTPtr> executeQuery(const char *begin, const char* end)
{
    ASTPtr ast;
    Tokens tokens(begin, end, 100);
    Pos pos(tokens, 5);
    Expected expected;

    // empty query
    if (pos->isEnd() || pos->type == TokenType::Semicolon)
        return {QueryErrorType::Empty, nullptr};

    ParserQuery parser;
    bool parse_res = parser.parse(pos, ast, expected);

    // syntax error
    if (!parse_res)
    {
        assert(ast == nullptr);
        return {QueryErrorType::SyntaxError, nullptr};
    }

    assert(ast != nullptr);
    return {Non, ast};
}