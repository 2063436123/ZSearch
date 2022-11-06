#include "extractor.h"
#include <fcntl.h>

void check_string_in_file(TxtReader &reader, int file_fd, const char *expected_str, size_t expected_offset)
{
    assert(strlen(expected_str));

    char buf;
    StringInFile line = reader.readUntil();

    ASSERT_EQ(line.str, expected_str);
    ASSERT_EQ(line.offset_in_file, expected_offset);

    lseek(file_fd, line.offset_in_file, SEEK_SET);
    read(file_fd, &buf, 1);
    ASSERT_EQ(buf, expected_str[0]);
}

TEST(extractor, WordExtractor)
{
    WordExtractor extractor;

    TxtReader reader(root_path + "/articles/ABC.txt");

    StringInFile line = reader.readUntil();
    StringInFiles sifs = extractor.extract(line);
    ASSERT_EQ(sifs.size(), 3);
    ASSERT_EQ(sifs[0].str, "a");
    ASSERT_EQ(sifs[1].str, "b");
    ASSERT_EQ(sifs[2].str, "c");
    ASSERT_EQ(sifs[0].offset_in_file, 0);
    ASSERT_EQ(sifs[1].offset_in_file, 2);
    ASSERT_EQ(sifs[2].offset_in_file, 4);

    sifs = extractor.extract(reader.readUntil());
    ASSERT_EQ(sifs.size(), 2);
    ASSERT_EQ(sifs[0].str, "d");
    ASSERT_EQ(sifs[1].str, "e");
    ASSERT_EQ(sifs[0].offset_in_file, 7);
    ASSERT_EQ(sifs[1].offset_in_file, 9);

    sifs = extractor.extract(reader.readUntil());
    ASSERT_EQ(sifs.size(), 2);
    ASSERT_EQ(sifs[0].str, "f");
    ASSERT_EQ(sifs[1].str, "g");
    ASSERT_EQ(sifs[0].offset_in_file, 11);
    ASSERT_EQ(sifs[1].offset_in_file, 13);

    sifs = extractor.extract(reader.readUntil());
    ASSERT_EQ(sifs.size(), 1);
    ASSERT_EQ(sifs[0].str, "h");
    ASSERT_EQ(sifs[0].offset_in_file, 19);

    sifs = extractor.extract(reader.readUntil());
    ASSERT_EQ(sifs.size(), 1);
    ASSERT_EQ(sifs[0].str, "i");
    ASSERT_EQ(sifs[0].offset_in_file, 22);

    sifs = extractor.extract(reader.readUntil());
    ASSERT_EQ(sifs.size(), 1);
    ASSERT_EQ(sifs[0].str, "jjj");
    ASSERT_EQ(sifs[0].offset_in_file, 25);

    sifs = extractor.extract(reader.readUntil());
    ASSERT_EQ(sifs.size(), 1);
    ASSERT_EQ(sifs[0].str, "kkk");
    ASSERT_EQ(sifs[0].offset_in_file, 30);

    ASSERT_EQ(reader.readUntil().str, "");
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}