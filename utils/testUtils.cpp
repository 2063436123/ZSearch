#include "SerializerUtils.h"
#include "TimeUtils.h"
#include "StringUtils.h"
#include "SortUtils.h"
#include <fcntl.h>

TEST(WriteBuffer, dumpAllToStream)
{
    WriteBuffer buf;
    buf.append("hello", 5);

    std::ofstream fout("./tmp");
    buf.dumpAllToStream(fout);
    fout.close();

    int fd = open("./tmp", O_RDONLY);
    ASSERT_GT(fd, 0);
    char cs[100];
    ASSERT_EQ(read(fd, cs, 100), 5);
    ASSERT_EQ(strncmp(cs, "hello", 5), 0);
}

TEST(WriteReadBufferHelper, Base)
{
    WriteBuffer write_buf;
    WriteBufferHelper write_helper(write_buf);

    write_helper.writeString("hello");
    write_helper.writeNumber(int(-100));
    std::vector<size_t> numbers{1, 2, 3, 5};
    write_helper.writeLinearContainer(numbers);
    std::vector<std::string> strings{"str1", "str2", "str3"};
    write_helper.writeLinearContainer(strings);
    std::set<size_t> numbers_set{100, 20, 30};
    write_helper.writeSetContainer(numbers_set);

    ReadBuffer read_buf;
    auto str_ref = write_buf.string_ref();
    read_buf.append(str_ref.first, str_ref.second);
    ReadBufferHelper read_helper(read_buf);

    ASSERT_EQ(read_helper.readString(), "hello");
    ASSERT_EQ(read_helper.readNumber<int>(), -100);
    auto read_numbers = read_helper.readLinearContainer<std::vector, size_t>();
    ASSERT_EQ(numbers, read_numbers);
    auto read_strings = read_helper.readLinearContainer<std::vector, std::string>();
    ASSERT_EQ(strings, read_strings);
    auto read_numbers_set = read_helper.readSetContainer<std::set, size_t>();
    ASSERT_EQ(read_numbers_set, numbers_set);

    ASSERT_THROW(read_helper.readNumber<int>(), Poco::RangeException);
}

TEST(time, DateTime)
{
    ASSERT_THROW(DateTime("2011-03-28 15:00:44 "), DateTimeFormatException);
    ASSERT_THROW(DateTime(" 2011-03-28 15:00:44 "), DateTimeFormatException);
    ASSERT_THROW(DateTime("2011-3-28 15:00:44 "), DateTimeFormatException);
    DateTime time1("2011-03-28 15:00:44");
    ASSERT_EQ(time1.string(), "2011-03-28 15:00:44");
    DateTime time2("2011-03-28 15:00:45");
    ASSERT_LT(time1, time2);
}

TEST(stringUtils, stoi)
{
    ASSERT_EQ(restrictStoi<int>("-12"), -12);
    ASSERT_EQ(restrictStoi<int>("102"), 102);
    ASSERT_EQ(restrictStoi<int>("0"), 0);
    ASSERT_EQ(restrictStoi<int>("-0"), 0);

    ASSERT_THROW(restrictStoi<int>(""), Poco::InvalidArgumentException);
    ASSERT_THROW(restrictStoi<int>("  "), Poco::InvalidArgumentException);
    ASSERT_THROW(restrictStoi<int>("1 "), Poco::InvalidArgumentException);
    ASSERT_THROW(restrictStoi<int>(" -1"), Poco::InvalidArgumentException);
}

TEST(sortUtils, SyncSort)
{
    std::vector<int> a{4, 2, 1, 3};
    std::vector<int> b{100, 200, 300, 400};

    auto [ra, rb] = sync_sort(a, b);

    std::vector<int> res_a{1, 2, 3, 4};
    std::vector<int> res_b{300, 200, 400, 100};
    ASSERT_EQ(ra, res_a);
    ASSERT_EQ(rb, res_b);
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}