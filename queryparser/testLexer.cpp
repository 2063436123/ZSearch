#include "Token.h"

TEST(Lexer, Token1)
{
    std::string query0("");
    const char* begin = query0.c_str();
    const char* end = begin + query0.size();
    auto token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Empty);

    std::string query1("hello");
    begin = query1.c_str();
    end = begin + query1.size();
    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Identifier);
    ASSERT_EQ(get<IdentifierName>(token.var), IdentifierName{.column_name = "hello"});

    std::string query2("hello world");
    begin = query2.c_str();
    end = begin + query2.size();
    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Identifier);
    ASSERT_EQ(get<IdentifierName>(token.var), IdentifierName{.column_name = "hello"});
    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Identifier);
    ASSERT_EQ(get<IdentifierName>(token.var), IdentifierName{.column_name = "world"});
}

TEST(Lexer, Token2)
{
    std::string query1("hello AND (world OR ji ) AND  NOT 'fuck' 123");
    const char* begin = query1.c_str();
    const char* end = begin + query1.size();

    auto token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Identifier);
    ASSERT_EQ(get<IdentifierName>(token.var), IdentifierName{.column_name = "hello"});

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Keyword);
    ASSERT_EQ(get<KeyWordType>(token.var), KeyWordType::And);

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Keyword);
    ASSERT_EQ(get<KeyWordType>(token.var), KeyWordType::L_Bracket);

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Identifier);
    ASSERT_EQ(get<IdentifierName>(token.var), IdentifierName{.column_name = "world"});

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Keyword);
    ASSERT_EQ(get<KeyWordType>(token.var), KeyWordType::Or);

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Identifier);
    ASSERT_EQ(get<IdentifierName>(token.var), IdentifierName{.column_name = "ji"});

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Keyword);
    ASSERT_EQ(get<KeyWordType>(token.var), KeyWordType::R_Bracket);

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Keyword);
    ASSERT_EQ(get<KeyWordType>(token.var), KeyWordType::And);

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Keyword);
    ASSERT_EQ(get<KeyWordType>(token.var), KeyWordType::Not);

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Value);
    ASSERT_EQ(get<Value>(token.var), Value("fuck"));

    token = getToken(begin, end);
    ASSERT_EQ(token.type, SyntaxType::Value);
    ASSERT_EQ(get<Value>(token.var), Value(123ll));
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}