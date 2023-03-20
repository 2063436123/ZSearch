#include "TermsExecutor.h"
#include "HavingExecutor.h"
#include "ScoreExecutor.h"
#include "LimitExecutor.h"
#include "indexer/Indexer.h"
#include "gtest/gtest.h"

const int IfI = 8, WhenYou = 11, WhatCan = 4, WEBAPP = 6, Alice = 10;

TEST(termsExecutor, base)
{
    Database db(ROOT_PATH + "/database1", true);

    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");
    EXPECT_EQ(db.maxAllocatedDocId(), 11);

    EXPECT_EQ(db.findDocument(IfI)->getPath().filename(), "IfIWereToFallInLove.txt");
    EXPECT_EQ(db.findDocument(WhenYou)->getPath().filename(), "WhenYouAreOld.txt");
    EXPECT_EQ(db.findDocument(WhatCan)->getPath().filename(), "WhatCanIHoldYouWith.txt");

    {
        LeafNode<String> l1("fall");
        LeafNode<String> l2("Take");
        InterNode r1(ConjunctionType::AND);
        r1.addChild(&l1).addChild(&l2);

        TermsExecutor executor(db, &r1);
        EXPECT_EQ(std::any_cast<DocIds>(executor.execute(nullptr)), DocIds({IfI, Alice}));
    }

    {
        LeafNode<String> l1("loved");
        LeafNode<String> l2("Take");
        InterNode r1(ConjunctionType::OR);
        r1.addChild(&l1).addChild(&l2);

        TermsExecutor executor(db, &r1);
        EXPECT_EQ(std::any_cast<DocIds>((executor.execute(nullptr))), DocIds({IfI, WhenYou, Alice}));
    }

    {
        LeafNode<String> l10("memory");
        InterNode r2(ConjunctionType::NOT);
        r2.addChild(&l10);

        TermsExecutor executor(db, &r2);
        EXPECT_EQ(std::any_cast<DocIds>(executor.execute(nullptr)),
                  DynamicBitSet(db.maxAllocatedDocId()).set(WhatCan).set(Alice).flip().toUnorderedSet(1));
    }

    {
        LeafNode<String> l1("loved");
        LeafNode<String> l2("Take");
        LeafNode<String> l3("you");
        InterNode r1(ConjunctionType::OR);
        r1.addChild(&l1).addChild(&l2).addChild(&l3);

        LeafNode<String> l10("memory");
        InterNode r2(ConjunctionType::NOT);
        r2.addChild(&l10);

        InterNode r3(ConjunctionType::AND);
        r3.addChild(&r1).addChild(&r2);

        TermsExecutor executor(db, &r3);
        EXPECT_EQ(std::any_cast<DocIds>(executor.execute(nullptr)), DocIds({IfI, WhenYou}));
    }
}

TEST(CompareFunction, base)
{
    {
        Value v1(172), v2(172.0);
        EXPECT_EQ(compareEqual(v1, v2), true);
        Value v3("hello"), v4("hello");
        EXPECT_EQ(compareNotEqual(v3, v4), false);
        Value v5("hello"), v6("hello1");
        EXPECT_EQ(compareEqual(v5, v6), false);
        Value arr(ArrayLabel{}, ValueType::Number);
        EXPECT_THROW(compareEqual(v1, v3), Poco::InvalidArgumentException);
    }
    {
        Value v1(172), v2(172.0), v3(169), v4(100000);
        EXPECT_EQ(compareLess(v1, v2), false);
        EXPECT_EQ(compareGreater(v3, v2), false);
        EXPECT_EQ(compareGreaterOrEqual(v1, v2), true);
        EXPECT_EQ(compareLess(v3, v4), true);
        EXPECT_EQ(compareLessOrEqual(v3, v2), true);

        Value vs1("hello"), vs2("hella"), vs3("hell"), vs4("helko");
        EXPECT_EQ(compareLess(vs2, vs1), true);
        EXPECT_EQ(compareLess(vs3, vs1), true);
        EXPECT_EQ(compareLess(vs4, vs3), true);
        EXPECT_EQ(compareGreaterOrEqual(vs2, vs1), false);
    }
    {
        Value v1(100), v2(500);

        Value arr1(ArrayLabel{}, ValueType::Number);
        arr1.doArrayHandler<Number>([](std::vector<Number>* vec) { vec->assign({100, 200, 300}); });

        Value arr2(ArrayLabel{}, ValueType::Number);
        arr2.doArrayHandler<Number>([](std::vector<Number>* vec) { vec->assign({300}); });

        EXPECT_EQ(compareIn(v1, arr1), true);
        EXPECT_EQ(compareNotIn(v1, arr1), false);
        EXPECT_EQ(compareNotIn(v2, arr1), true);
        EXPECT_EQ(compareIn(v2, arr1), false);

        EXPECT_THROW(compareIn(arr1, arr2), Poco::InvalidArgumentException);
        EXPECT_THROW(compareIn(v1, v2), Poco::InvalidArgumentException);
        EXPECT_THROW(compareIn(arr1, v2), Poco::InvalidArgumentException);


        Value v3("hello"), v4("worl");
        Value arr3(ArrayLabel{}, ValueType::String);
        arr3.doArrayHandler<String>([](std::vector<String>* vec) { vec->assign({"hehlo", "hello", "world"}); });
        EXPECT_EQ(compareIn(v3, arr3), true);
        EXPECT_EQ(compareIn(v4, arr3), false);
        EXPECT_THROW(compareIn(v1, arr3), Poco::InvalidArgumentException);
        EXPECT_EQ(compareNotIn(v4, arr3), true);
    }
}

TEST(AggregateFunction, base)
{
    {
        Value arr1(ArrayLabel{}, ValueType::Number);
        arr1.doArrayHandler<Number>([](std::vector<Number>* vec) { vec->assign({100, 200, 300, -1}); });
        EXPECT_EQ(sumFunction(arr1).as<Number>(), 599);
        EXPECT_EQ(countFunction(arr1).as<Number>(), 4);
        EXPECT_EQ(avgFunction(arr1).as<Number>(), 149.75);
        EXPECT_EQ(maxFunction(arr1).as<Number>(), 300);
        EXPECT_EQ(minFunction(arr1).as<Number>(), -1);
    }
    {
        Value arr1(ArrayLabel{}, ValueType::String);
        arr1.doArrayHandler<String>([](std::vector<String>* vec) { vec->assign({"hello", "hell", "hella"}); });
        EXPECT_THROW(sumFunction(arr1).as<String>(), Poco::InvalidArgumentException);
        EXPECT_EQ(countFunction(arr1).as<Number>(), 3);
        EXPECT_THROW(avgFunction(arr1).as<String>(), Poco::InvalidArgumentException);
        EXPECT_EQ(maxFunction(arr1).as<String>(), "hello");
        EXPECT_EQ(minFunction(arr1).as<String>(), "hell");
    }
    {
        Value arr1(ArrayLabel{}, ValueType::DateTime);
        arr1.doArrayHandler<DateTime>([](std::vector<DateTime>* vec) { vec->assign({DateTime(1000), DateTime(100000)}); });
        EXPECT_THROW(sumFunction(arr1).as<DateTime>(), Poco::InvalidArgumentException);
        EXPECT_EQ(countFunction(arr1).as<Number>(), 2);
        EXPECT_THROW(avgFunction(arr1).as<DateTime>(), Poco::InvalidArgumentException);
        EXPECT_EQ(maxFunction(arr1).as<DateTime>().string(), "1970-01-02 03:46:40");
        EXPECT_EQ(minFunction(arr1).as<DateTime>().string(), "1970-01-01 00:16:40");
    }
    {
        Value arr1(ArrayLabel{}, ValueType::Bool);
        arr1.doArrayHandler<Bool>([](std::vector<Bool>* vec) { vec->assign({ false }); });
        EXPECT_THROW(sumFunction(arr1).as<Bool>(), Poco::InvalidArgumentException);
        EXPECT_EQ(countFunction(arr1).as<Number>(), 1);
        EXPECT_THROW(avgFunction(arr1).as<Bool>(), Poco::InvalidArgumentException);
        EXPECT_THROW(maxFunction(arr1).as<Bool>(), Poco::InvalidArgumentException);
        EXPECT_THROW(minFunction(arr1).as<Bool>(), Poco::InvalidArgumentException);
    }
    {
        EXPECT_THROW(Value(ArrayLabel{}, ValueType::Null), Poco::NotImplementedException);
    }
}

TEST(havingExecutor, base)
{
    Database db(ROOT_PATH + "/database1", true);

    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    EXPECT_EQ(db.findDocument(WEBAPP)->getPath().filename(), "webapp.json");

    auto all_doc_ids = DynamicBitSet(db.maxAllocatedDocId()).flip().toUnorderedSet(1);
    {
        Value arr1(ArrayLabel{}, ValueType::Number);
        arr1.doArrayHandler<Number>([](std::vector<Number>* vec){ vec->assign({1000, 2000, 600}); });
        // sum(web-app.i-arr) in (1000, 2000, 600)
        LeafNode<Predicate> l1(Predicate(sumFunction, "web-app.i-arr", compareIn, arr1));

        HavingExecutor executor(db, &l1);
        EXPECT_EQ(std::any_cast<DocIds>(executor.execute(all_doc_ids)), DocIds({WEBAPP}));
    }

    {
        Value arr1(ArrayLabel{}, ValueType::Number);
        arr1.doArrayHandler<Number>([](std::vector<Number>* vec){ vec->assign({1000, 2000}); });
        // sum(web-app.i-arr) not in (1000, 2000, 600) AND b-arr.size = 2
        LeafNode<Predicate> l1(Predicate(sumFunction, "web-app.i-arr", compareNotIn, arr1));
        LeafNode<Predicate> l2(Predicate(countFunction, "web-app.b-arr", compareEqual, 2));

        InterNode r1(ConjunctionType::AND);
        r1.addChild(&l1).addChild(&l2);

        HavingExecutor executor(db, &r1);
        EXPECT_EQ(std::any_cast<DocIds>(executor.execute(all_doc_ids)), DocIds({WEBAPP}));
    }
}

TEST(ScoreExecutor, base)
{
    Database db(ROOT_PATH + "/database1", true);

    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    EXPECT_EQ(db.findDocument(WEBAPP)->getPath().filename(), "webapp.json");
    auto all_doc_ids = DynamicBitSet(db.maxAllocatedDocId()).flip().toUnorderedSet(1);

    EXPECT_EQ(db.findDocument(IfI)->getPath().filename(), "IfIWereToFallInLove.txt");
    EXPECT_EQ(db.findDocument(WhenYou)->getPath().filename(), "WhenYouAreOld.txt");
    EXPECT_EQ(db.findDocument(WhatCan)->getPath().filename(), "WhatCanIHoldYouWith.txt");

    {
        LeafNode<String> l1("you");
        auto terms_executor = std::make_shared<TermsExecutor>(db, &l1);
        auto score_executor = std::make_shared<ScoreExecutor>(db, std::unordered_map<std::string, double>{{"you", 1.0}});

        ExecutePipeline pipeline;
        pipeline.addExecutor(terms_executor).addExecutor(score_executor);
        auto doc_id_vs_score = std::any_cast<Scores>(pipeline.execute());
        Scores expected({{167, IfI}, {138, WhatCan}, {60, WhenYou}, {2, Alice}});
        EXPECT_EQ(doc_id_vs_score, expected);
    }
}

TEST(LimitExecutor, base)
{
    Database db(ROOT_PATH + "/database1", true);

    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles-cnn");

    {
        LeafNode<String> l1("you");
        auto terms_executor = std::make_shared<TermsExecutor>(db, &l1);
        auto score_executor = std::make_shared<ScoreExecutor>(db, std::unordered_map<std::string, double>{{"you", 1.0}});
        auto limit_executor = std::make_shared<LimitExecutor>(db, 10);

        ExecutePipeline pipeline;
        pipeline.addExecutor(terms_executor).addExecutor(score_executor).addExecutor(limit_executor);
        auto doc_id_vs_score = std::any_cast<Scores>(pipeline.execute());
        EXPECT_EQ(doc_id_vs_score.size(), 10);
    }
}

TEST(Executor, deleted_document)
{
    Database db(ROOT_PATH + "/database1", true);

    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    EXPECT_EQ(db.findDocument(WEBAPP)->getPath().filename(), "webapp.json");
    auto all_doc_ids = DynamicBitSet(db.maxAllocatedDocId()).flip().toUnorderedSet(1);

    EXPECT_EQ(db.findDocument(IfI)->getPath().filename(), "IfIWereToFallInLove.txt");
    EXPECT_EQ(db.findDocument(WhenYou)->getPath().filename(), "WhenYouAreOld.txt");
    EXPECT_EQ(db.findDocument(WhatCan)->getPath().filename(), "WhatCanIHoldYouWith.txt");

    {
        LeafNode<String> l1("you");
        TermsExecutor terms_executor(db, &l1);
        ScoreExecutor score_executor(db, std::unordered_map<std::string, double>{{"you", 1.0}});

        db.deleteDocument(IfI);
        DocIds inter_data = std::any_cast<DocIds>(terms_executor.execute(nullptr));
        EXPECT_EQ(inter_data, DocIds({IfI, WhatCan, WhenYou, Alice})); // 注意即使文档被删除，TermsExecutor 也不会削减输出结果 --> 需要后续步骤去确认文档的有效性

        db.deleteDocument(WhenYou);
        auto doc_id_vs_score = std::any_cast<Scores>(score_executor.execute(inter_data));
        Scores expected({{141, WhatCan}, {2, Alice}});
        EXPECT_EQ(doc_id_vs_score, expected);
    }
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}