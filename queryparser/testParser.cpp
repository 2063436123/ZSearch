#include "QueryParser.h"

TEST(DISABLED_Parser, base)
{
    {
        QueryParser parser0("Hello");
        auto tree0 = parser0.parse();
        ASSERT_EQ(tree0->token->type, SyntaxType::Identifier);
        ASSERT_EQ(tree0->children.size(), 0);
    }
    {
        QueryParser parser1("Hello AND world");
        auto tree1 = parser1.parse();
        ASSERT_EQ(tree1->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(tree1->token->var), KeyWordType::And);
        ASSERT_EQ(tree1->children.size(), 2);
        ASSERT_EQ(tree1->children[0]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(tree1->children[0]->token->var), IdentifierName::New("Hello"));
        ASSERT_EQ(tree1->children[1]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(tree1->children[1]->token->var), IdentifierName::New("world"));
    }

    {
        QueryParser parser1("a AND b AND NOT c");
        auto tree1 = parser1.parse();
        ASSERT_EQ(tree1->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(tree1->token->var), KeyWordType::And);
        ASSERT_EQ(tree1->children.size(), 2);

        auto left = tree1->children[0];
        auto right = tree1->children[1];
        ASSERT_EQ(left->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(left->token->var), KeyWordType::And);

        ASSERT_EQ(right->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(right->token->var), KeyWordType::Not);

        ASSERT_EQ(left->children.size(), 2);
        ASSERT_EQ(right->children.size(), 1);

        ASSERT_EQ(left->children[0]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(left->children[0]->token->var), IdentifierName::New("a"));
        ASSERT_EQ(left->children[1]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(left->children[1]->token->var), IdentifierName::New("b"));

        ASSERT_EQ(right->children[0]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(right->children[0]->token->var), IdentifierName::New("c"));
    }

    {
        QueryParser parser1("NOT b AND (NOT c)");
        auto tree1 = parser1.parse();
        ASSERT_EQ(tree1->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(tree1->token->var), KeyWordType::And);
        ASSERT_EQ(tree1->children.size(), 2);

        auto left = tree1->children[0];
        auto right = tree1->children[1];
        ASSERT_EQ(left->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(left->token->var), KeyWordType::Not);

        ASSERT_EQ(right->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(right->token->var), KeyWordType::Not);

        ASSERT_EQ(left->children.size(), 1);
        ASSERT_EQ(right->children.size(), 1);

        ASSERT_EQ(left->children[0]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(left->children[0]->token->var), IdentifierName::New("b"));

        ASSERT_EQ(right->children[0]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(right->children[0]->token->var), IdentifierName::New("c"));
    }

    {
        QueryParser parser1("NOT b AND (a OR c)");
        auto tree1 = parser1.parse();
        ASSERT_EQ(tree1->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(tree1->token->var), KeyWordType::And);
        ASSERT_EQ(tree1->children.size(), 2);

        auto left = tree1->children[0];
        auto right = tree1->children[1];
        ASSERT_EQ(left->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(left->token->var), KeyWordType::Not);

        ASSERT_EQ(right->token->type, SyntaxType::Keyword);
        ASSERT_EQ(get<KeyWordType>(right->token->var), KeyWordType::Or);

        ASSERT_EQ(left->children.size(), 1);
        ASSERT_EQ(right->children.size(), 2);

        ASSERT_EQ(left->children[0]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(left->children[0]->token->var), IdentifierName::New("b"));

        ASSERT_EQ(right->children[0]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(right->children[0]->token->var), IdentifierName::New("a"));
        ASSERT_EQ(right->children[1]->token->type, SyntaxType::Identifier);
        ASSERT_EQ(get<IdentifierName>(right->children[1]->token->var), IdentifierName::New("c"));
    }
}


int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}