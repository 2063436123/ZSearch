#pragma once

#include "IParser.h"

class ParserWordList : public IParserBase
{
public:
    const char *getName() const override
    {
        return "WordList";
    }

    bool parseImpl(Pos &pos, ASTPtr &node, Expected &expected) override
    {
        if (pos->type == TokenType::StringLiteral)
        {
            node = std::make_shared<ASTWord>(std::string{pos->begin, pos->end});
            ++pos;
            return true;
        }
        return false;
    }
};

class ParserAggregatedExpression : public IParserBase
{
public:
    const char *getName() const override
    {
        return "AggregatedExpression";
    }

    bool parseImpl(Pos &pos, ASTPtr &node, Expected &expected) override
    {
        return true;
    }
};

//struct ParserExpressionImpl
//{
//
//};
//
//class ParserList
//{
//
//};
//
//// combine ParserExpression and ParserList
//class ParserExpressionList;