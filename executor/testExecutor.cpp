#include "TermsExecutor.h"
#include "HavingExecutor.h"
#include "AggregateFunction.h"
#include "CompareFunction.h"
#include "indexer/Indexer.h"
#include "gtest/gtest.h"

TEST(termsExecutor, base)
{
    Database db(ROOT_PATH + "/database1", true);

    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");

    EXPECT_EQ(db.findDocument(4)->getPath().filename(), "IfIWereToFallInLove.txt");
    EXPECT_EQ(db.findDocument(5)->getPath().filename(), "WhenYouAreOld.txt");
    EXPECT_EQ(db.findDocument(14)->getPath().filename(), "WhatCanIHoldYouWith.txt");

    {
        LeafNode<String> l1("fall");
        LeafNode<String> l2("Take");
        InterNode r1(ConjunctionType::AND);
        r1.addChild(&l1).addChild(&l2);

        TermsExecutor executor(db, &r1);
        EXPECT_EQ(std::any_cast<std::set<size_t>>(executor.execute(nullptr)), std::set<size_t>({4}));
    }

    {
        LeafNode<String> l1("loved");
        LeafNode<String> l2("Take");
        InterNode r1(ConjunctionType::OR);
        r1.addChild(&l1).addChild(&l2);

        TermsExecutor executor(db, &r1);
        EXPECT_EQ(std::any_cast<std::set<size_t>>(executor.execute(nullptr)), std::set<size_t>({4, 5}));
    }

    {
        LeafNode<String> l10("memory");
        InterNode r2(ConjunctionType::NOT);
        r2.addChild(&l10);

        TermsExecutor executor(db, &r2);
        EXPECT_EQ(std::any_cast<std::set<size_t>>(executor.execute(nullptr)),
                  DynamicBitSet(16).set(14).flip().toSet(1));
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
        EXPECT_EQ(std::any_cast<std::set<size_t>>(executor.execute(nullptr)), std::set<size_t>({4, 5}));
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
        EXPECT_EQ(compareGreaterEqual(v1, v2), true);
        EXPECT_EQ(compareLess(v3, v4), true);
        EXPECT_EQ(compareLessEqual(v3, v2), true);

        Value vs1("hello"), vs2("hella"), vs3("hell"), vs4("helko");
        EXPECT_EQ(compareLess(vs2, vs1), true);
        EXPECT_EQ(compareLess(vs3, vs1), true);
        EXPECT_EQ(compareLess(vs4, vs3), true);
        EXPECT_EQ(compareGreaterEqual(vs2, vs1), false);
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

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}