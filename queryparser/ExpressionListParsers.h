#pragma once

#include "IParser.h"
#include "utils/StringUtils.h"

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
            node = std::make_shared<ASTWord>(trimQuote(pos->string()));
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
        // SUM
        if (pos->type != TokenType::BareWord)
            return false;
        std::string func_name(pos->begin, pos->end);
        ++pos;

        // (
        if (pos->type != TokenType::OpeningRoundBracket)
            return false;
        ++pos;

        // a_json_key
        std::string column_name;
        if (pos->type == TokenType::StringLiteral)
        {
            column_name = trimQuote(pos->string());
            ++pos;
        }

        // )
        if (pos->type != TokenType::ClosingRoundBracket)
            return false;
        ++pos;

        TokenType compare_type = pos->type;
        Value compare_value;
        if (func_name != "EXISTS") // EXISTS() hasn't compare_name and compare_value
        {
            // <= 100 or == 'a' or > '2022-01-23' or == true or == false
            if (isComparableTokenType(pos->type))
            {
                ++pos;
                if (pos->type == TokenType::Number)
                {
                    compare_value = Value(restrictStod(pos->string()));
                }
                else if (pos->type == TokenType::StringLiteral)
                {
                    compare_value = Value(trimQuote(pos->string()));
                }
                else if (pos->type == TokenType::BareWord)
                {
                    if (Poco::toLower(pos->string()) == "true")
                        compare_value = Value(true);
                    else if (Poco::toLower(pos->string()) == "false")
                        compare_value = Value(false);
                    else
                        return false;
                }
                else
                    return false;
                ++pos;
            }
            // IN (100, 200) or IN ('a', 'b')
            else if (pos->type == TokenType::InRange)
            {
                ++pos;
                if (pos->type != TokenType::OpeningRoundBracket)
                    return false;
                ++pos;

                if (pos->type == TokenType::Number)
                {
                    compare_value = Value(ArrayLabel{}, ValueType::Number);
                    std::vector<double> doubles;
                    while (pos->type == TokenType::Number)
                    {
                        doubles.push_back(restrictStod(pos->string()));
                        ++pos;
                        if (pos->type != TokenType::Comma)
                            break;
                        ++pos;
                    }
                    ValueArrayHandler<double> double_array_handler = [&](std::vector<double>* vec) {
                        *vec = doubles;
                    };
                    compare_value.doArrayHandler(double_array_handler);
                }
                else if (pos->type == TokenType::StringLiteral)
                {
                    compare_value = Value(ArrayLabel{}, ValueType::String);
                    std::vector<std::string> strings;
                    while (pos->type == TokenType::StringLiteral)
                    {
                        strings.push_back(trimQuote(pos->string()));
                        ++pos;
                        if (pos->type != TokenType::Comma)
                            break;
                        ++pos;
                    }
                    ValueArrayHandler<std::string> string_array_handler = [&](std::vector<std::string>* vec) {
                        *vec = strings;
                    };
                    compare_value.doArrayHandler(string_array_handler);
                }

                if (pos->type != TokenType::ClosingRoundBracket)
                    return false;
                ++pos;
            }
            else
            {
                return false;
            }
        }

        node = std::make_shared<ASTHaving>(func_name, column_name, compare_type, compare_value);
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