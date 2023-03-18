#pragma once
#include "IParser.h"

class ParserKeyWord : public IParserBase
{
private:
    std::string_view s;

public:
    constexpr ParserKeyWord(std::string_view s_) : s(s_) { assert(!s.empty()); }

    constexpr const char* getName() const override { return s.data(); }

protected:
    bool parseImpl(Pos &pos, ASTPtr &node, Expected &expected) override
    {
        if (pos->type != TokenType::BareWord)
            return false;
        const char * current_word = s.begin();

        while (true)
        {
            expected.add(pos, current_word);

            if (pos->type != TokenType::BareWord)
                return false;

            const char *const next_whitespace = std::find_if(current_word, s.end(), [](char ch){
                return ch == ' ' || ch == '\0';
            });

            const size_t word_length = next_whitespace - current_word;
            if (word_length != pos->size())
                return false;

            if (0 != strncmp(pos->begin, current_word, word_length))
                return false;

            ++pos;

            // 没有找到下一个空白时，返回 s.end()，即'\0'，退出循环
            if (!*next_whitespace)
                break;

            current_word = next_whitespace + 1;
        }

        return true;
    }
};

class ParserToken : public IParserBase
{
private:
    TokenType token_type;
public:
    ParserToken(TokenType token_type_) : token_type(token_type_) {}

protected:
    constexpr const char* getName() const override { return "token"; }

    bool parseImpl(Pos &pos, ASTPtr &node, Expected &expected) override
    {
        if (pos->type != token_type)
        {
//            expected.add(pos, getTokenName(token_type);
            return false;
        }
        setNodeByToken(pos.get(), node);
        ++pos;
        return true;
    }

private:
    void setNodeByToken(const Token& token, ASTPtr &node) const
    {
        if (token.type == TokenType::Number)
        {
            node = std::make_shared<ASTLimit>(restrictStod(std::string(token.begin, token.end)));
        }
        else
        {
            node = nullptr;
        }
    }
};