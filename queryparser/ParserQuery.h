#pragma once

#include "IParser.h"
#include "CommonParsers.h"
#include "ExpressionListParsers.h"
#include "ASTQuery.h"

class ParserQuery : public IParserBase
{
protected:
    constexpr const char *getName() const override { return "Query"; }

    bool parseImpl(Pos &pos, ASTPtr &node, Expected &expected) override
    {
        // word_list [LIMIT number];
        // or
        // [word_list] HAVING exp_elem [LIMIT number];
        // 注意，word_list 需要用 '' 括起

        ParserWordList s_word_list;

        ParserKeyWord s_having("HAVING");
        ParserAggregatedExpression exp_elem;

        ParserKeyWord s_limit("LIMIT");
        ParserToken token_number(TokenType::Number);

        ASTPtr word_list;
        ASTPtr having_expression;
        ASTPtr limit_length;

        s_word_list.parse(pos, word_list, expected);

        if (s_having.ignore(pos, expected))
        {
            if (!exp_elem.parse(pos, having_expression, expected))
                return false;
        }

        if (s_limit.ignore(pos, expected))
        {
            if (!token_number.parse(pos, limit_length, expected))
                return false;
        }

        if (pos->type != TokenType::EndOfStream)
            return false;

        if (!word_list && !having_expression)
            return false;

        node = std::make_shared<ASTQuery>(word_list, having_expression, limit_length);

        return true;
    }
};