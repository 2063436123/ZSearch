#include "../typedefs.h"
#include "ParserQuery.h"
#include "Pos.h"
#include "indexer/Indexer.h"
#include "executor/TermsExecutor.h"
#include "Parser.h"
#include "searcher/Searcher.h"

TEST(Lexer, base)
{
    std::string str("SUM('auv') IN (12, 13)");
    Tokens tokens(str.data(), str.data() + str.size(), 100);

    ASSERT_EQ(tokens[0].type, TokenType::BareWord);

    ASSERT_EQ(tokens[1].type, TokenType::OpeningRoundBracket);

    ASSERT_EQ(tokens[2].type, TokenType::StringLiteral);

    ASSERT_EQ(tokens[3].type, TokenType::ClosingRoundBracket);

    ASSERT_EQ(tokens[4].type, TokenType::InRange);

    ASSERT_EQ(tokens[5].type, TokenType::OpeningRoundBracket);

    ASSERT_EQ(tokens[6].type, TokenType::Number);

    ASSERT_EQ(tokens[7].type, TokenType::Comma);

    ASSERT_EQ(tokens[8].type, TokenType::Number);

    ASSERT_EQ(tokens[9].type, TokenType::ClosingRoundBracket);

    ASSERT_EQ(tokens[10].type, TokenType::EndOfStream);
}

TEST(quotedString, base)
{
    auto testFunc = [](const char *cstr, bool expect_error = false) -> void {
        std::string str = cstr;
        const char *pos = str.data();
        Token res = quotedString<'\"', TokenType::StringLiteral>(pos, str.data(),
                                                                       str.data() + str.size());
        if (expect_error)
        {
            ASSERT_EQ(res.type, TokenType::Error);
            return;
        }
        ASSERT_EQ(res.type, TokenType::StringLiteral);
//        std::cout << res.toString() << std::endl;
        ASSERT_TRUE(str.starts_with(res.string()));
    };
//    testFunc(R"('hello world')");
    testFunc(R"("hello world")");
    testFunc(R"(""")", true);
    testFunc(R"("""")");
    testFunc(R"("""" ")");
    testFunc(R"("will" "be")");
    testFunc(R"("will""be")");
}

TEST(ParserQuery, base)
{
    auto judge = [](const std::string& cstr, bool assert_bool) {
        ASTPtr ast;
        Tokens tokens(cstr.data(), cstr.data() + cstr.size(), 100);
        Pos pos(tokens, 5);
        Expected expected;

        ParserQuery parser;
        bool parse_res = parser.parse(pos, ast, expected);

        ASSERT_EQ(parse_res, assert_bool);
    };
    judge("\'word\' LIMIT 10", true);
    judge("\'word\' limit 10", true);
    judge("\'word\' HAVING LIMIT 10", false);
    judge("\"word\" LIMIT 10", false);

    judge("\'word\' HAVING sum('hello') = 0 LIMIT 10", true);
    judge("\'word\' HAVING min('word') >= 'hello' LIMIT 10", true);
    judge("\'word\' HAVING AUTHOR() = 'hello' LIMIT 10", true);

    judge("word LIMIT 10", false);
    judge("\'word\' LIMIT", false);
    judge("\'word\' LIMIT 10", true);
}

template<typename T>
concept HasSingleArgConstructorC = requires(T a) {
    T{a}; // T must have a single argument constructor
};

template <HasSingleArgConstructorC T>
auto foo(T arg) {
    T obj(arg); // create object of type T using single argument constructor
    // do something with obj
    return obj;
}

TEST(Cpp20, base)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    auto f = foo<TermsExecutor>(db);
    auto us = f.execute(nullptr);
    ASSERT_EQ(std::any_cast<std::unordered_set<size_t>>(us), DynamicBitSet(db.maxAllocatedDocId()).fill().toUnorderedSet(1));
}

TEST(parser, integrated_without_having)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    auto [type, ast] = parseQuery(R"('love' LIMIT 10)");
    ASSERT_EQ(QueryErrorType::Non, type);

    auto pipeline = ast->as<ASTQuery>()->toExecutorPipeline(db);
    auto scores = std::any_cast<Scores>(pipeline.execute());

    ASSERT_EQ(scores.size(), 3);
}

TEST(Searcher, integrated_without_having)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    Searcher searcher(db);
    ASSERT_EQ(searcher.search(R"('love' LIMIT 10)").size(), 3);
    ASSERT_EQ(searcher.search(R"('love' LIMIT 2)").size(), 2);
    ASSERT_EQ(searcher.search(R"('you' LIMIT 10)").size(), 4);
}

TEST(Searcher, integrated_with_having)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    Searcher searcher(db);
    auto res = searcher.search(R"(having min('web-app.i-arr') = 100)");
    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0].doc_path, ROOT_PATH + "/articles/single-jsons/webapp.json");

    res = searcher.search(R"(having value('author') = 'ljz')");
    ASSERT_EQ(res.size(), 2);
    ASSERT_EQ(res[0].doc_path, ROOT_PATH + "/articles/single-jsons/webapp.json");
    ASSERT_EQ(res[1].doc_path, ROOT_PATH + "/articles/single-jsons/css.json");
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}