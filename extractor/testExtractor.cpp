#include "Extractor.h"

TEST(extractor, WordExtractor)
{
    auto reader = std::make_unique<TxtLineReader>(ROOT_PATH + "/articles/ABC.txt");
    ASSERT_NE(reader, nullptr);
    WordExtractor extractor(std::move(reader));

    StringInFiles sifs = extractor.extract().words;

    ASSERT_EQ(sifs.size(), 11);

    ASSERT_EQ(sifs[0].str, "a");
    ASSERT_EQ(sifs[1].str, "b");
    ASSERT_EQ(sifs[2].str, "c");
    ASSERT_EQ(sifs[3].str, "d");
    ASSERT_EQ(sifs[4].str, "e");
    ASSERT_EQ(sifs[5].str, "f");
    ASSERT_EQ(sifs[6].str, "g");
    ASSERT_EQ(sifs[7].str, "h");
    ASSERT_EQ(sifs[8].str, "i");
    ASSERT_EQ(sifs[9].str, "jjj");
    ASSERT_EQ(sifs[10].str, "kkk");

    ASSERT_EQ(sifs[0].offset_in_file, 0);
    ASSERT_EQ(sifs[1].offset_in_file, 2);
    ASSERT_EQ(sifs[2].offset_in_file, 4);
    ASSERT_EQ(sifs[3].offset_in_file, 7);
    ASSERT_EQ(sifs[4].offset_in_file, 9);
    ASSERT_EQ(sifs[5].offset_in_file, 11);
    ASSERT_EQ(sifs[6].offset_in_file, 13);
    ASSERT_EQ(sifs[7].offset_in_file, 19);
    ASSERT_EQ(sifs[8].offset_in_file, 22);
    ASSERT_EQ(sifs[9].offset_in_file, 25);
    ASSERT_EQ(sifs[10].offset_in_file, 30);
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}