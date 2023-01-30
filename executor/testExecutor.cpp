#include "TermsExecutor.h"
#include "indexer/Indexer.h"
#include "gtest/gtest.h"

TEST(termsExecutor, base)
try{
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
        EXPECT_EQ(executor.execute(), std::set<size_t>({4}));
    }

    {
        LeafNode<String> l1("loved");
        LeafNode<String> l2("Take");
        InterNode r1(ConjunctionType::OR);
        r1.addChild(&l1).addChild(&l2);

        TermsExecutor executor(db, &r1);
        EXPECT_EQ(executor.execute(), std::set<size_t>({4, 5}));
    }

    {
        LeafNode<String> l10("memory");
        InterNode r2(ConjunctionType::NOT);
        r2.addChild(&l10);

        TermsExecutor executor(db, &r2);
        EXPECT_EQ(executor.execute(), DynamicBitSet(16).set(14).flip().toSet(1));
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
        EXPECT_EQ(executor.execute(), std::set<size_t>({4, 5}));
    }

}catch (Poco::Exception& e)
{
    std::cout << e.message() << std::endl;
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}