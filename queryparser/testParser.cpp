#include "../typedefs.h"
#include "ParserQuery.h"
#include "Pos.h"
#include "indexer/Indexer.h"
#include "executor/TermsExecutor.h"

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
        ASSERT_TRUE(str.starts_with(res.toString()));
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
    judge("\'word\' HAVING LIMIT 10", true);

    judge("\'word\' having LIMIT 10", false);

    judge("\"word\" HAVING LIMIT 10", false);
    judge("word HAVING LIMIT 10", false);

    judge("\'word\' HAVING LIMIT", false);

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
    // TODO: check
    std::cout << std::any_cast<std::unordered_set<size_t>>(us).size() << std::endl;
    ASSERT_EQ(std::any_cast<std::unordered_set<size_t>>(us), DynamicBitSet(db.maxAllocatedDocId()).fill().toUnorderedSet(1));
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}