#include "reader.h"
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

TEST(reader, TxtReader)
{
    TxtReader reader(root_path + "/articles/IfIWereToFallInLove.txt");
    int fd = ::open((root_path + "/articles/IfIWereToFallInLove.txt").c_str(), O_RDONLY);

    check_string_in_file(reader, fd,
                         "If I were to fall in love,It would have to be with youYour eyes, your smile,The way you laugh,The things you say and do",
                         0);

    check_string_in_file(reader, fd, "Take me to the places,My heart never knew", 127);

    check_string_in_file(reader, fd, "So, if I were to fall in love,It would have to be with you.", 176);

    // eof 之后的任何 readUntil() 都返回 "".
    // first try
    StringInFile line = reader.readUntil();
    ASSERT_EQ(line.str, "");
    // second try
    line = reader.readUntil();
    ASSERT_EQ(line.str, "");
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}