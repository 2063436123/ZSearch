#pragma once

#include "typedefs.h"
#include "Token.h"

// 目前支持的语法：
// term: 字符串
// AND, OR, NOT

class TokenNode;
using TokenNodePtr = std::shared_ptr<TokenNode>;

struct TokenNode {
    TokenNode(TokenPtr token_) : token(token_) {}
    TokenPtr token;
    std::vector<TokenNodePtr> children;
};

class QueryParser {
public:
    QueryParser(std::string query) : origin_query(query), begin(origin_query.c_str()), end(begin + origin_query.size()) {
        for (const auto& ch : origin_query)
            if (isIllegalChar(ch))
                throw UnexpectedChar();
    }

    TokenNodePtr parse()
    {
        parseImpl(nullptr);
        if (bracket_cnt > 0)
            throw UnmatchedToken("too much left bracket");
        return last_node;
    }

private:
    TokenNodePtr parseImpl(TokenNodePtr last_node_, bool only_need_right = false)
    {
        last_node = std::move(last_node_);
        TokenPtr token = getNextToken();
        TokenNodePtr node = std::make_shared<TokenNode>(token);

        if (token->type == SyntaxType::Keyword)
        {
            KeyWordType type = get<KeyWordType>(token->var);
            if (type == KeyWordType::And || type == KeyWordType::Or)
            {
                if (!last_node)
                    throw UnexpectedToken("And, Or need left token");
                auto l_token = last_node;
                if (l_token->token->type == SyntaxType::Empty || l_token->token->type == SyntaxType::Keyword && !isLogicPredicate(get<KeyWordType>(l_token->token->var)))
                    throw UnexpectedToken("And, Or need normal left token");
                node->children.push_back(l_token);

                auto r_token = parseImpl(node, true);
                if (r_token->token->type == SyntaxType::Empty || r_token->token->type == SyntaxType::Keyword && !isLogicPredicate(get<KeyWordType>(r_token->token->var)))
                    throw UnexpectedToken("And, Or need normal right token");
                node->children.push_back(r_token);
            }
            if (type == KeyWordType::Not)
            {
                auto r_token = parseImpl(node, true);
                if (r_token->token->type == SyntaxType::Empty || r_token->token->type == SyntaxType::Keyword)
                    throw UnexpectedToken("Not need normal right token");
                node->children.push_back(r_token);
            }
            if (type == KeyWordType::L_Bracket)
            {
                ++bracket_cnt;
                node = parseImpl(nullptr);
            }
            if (type == KeyWordType::R_Bracket)
            {
                if (--bracket_cnt < 0)
                    throw UnmatchedToken("too much right bracket");
                only_need_right = true;
            }
        }
        else if (token->type == SyntaxType::Empty)
        {
            // TODO;
            return nullptr;
        }
        else
        {
        }

        if (!only_need_right)
            parseImpl(node);
        return node;
    }

    TokenPtr getNextToken()
    {
        auto ptr = std::make_shared<Token>();
        *ptr = getToken(begin, end);
        return ptr;
    }

    TokenNodePtr last_node = nullptr;

    int bracket_cnt = 0;

    std::string origin_query;
    const char* begin;
    const char* end;
};