#include "SerializeUtils.h"
#include "TimeUtils.h"
#include "StringUtils.h"
#include "ContainerUtils.h"
#include "JsonUtils.h"
#include "DynamicBitSet.h"
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

TEST(vector_bool, serialization)
{
    WriteBuffer wbuf;
    WriteBufferHelper whelper(wbuf);

    std::stringstream io;

    ReadBuffer rbuf;
    ReadBufferHelper rhelper(rbuf);

    std::vector<bool> vec{true, false, false, true};

    whelper.writeLinearContainer(vec);
    wbuf.dumpAllToStream(io);

    rbuf.readAllFromStream(io);
    io.clear();
    auto res = rhelper.readLinearContainer<std::vector, bool>();

    EXPECT_EQ(vec, res);
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

    auto [ra, rb] = syncSort(a, b);

    std::vector<int> res_a{1, 2, 3, 4};
    std::vector<int> res_b{300, 200, 400, 100};
    ASSERT_EQ(ra, res_a);
    ASSERT_EQ(rb, res_b);
}

TEST(sortUtils, RemoveElements)
{
    {
        std::vector<int> a{1, 2};
        std::unordered_set<size_t> index_to_delete({0, 1});

        removeElements(a, index_to_delete);
        EXPECT_EQ(a, std::vector<int>());
    }
    {
        std::vector<int> a{1};
        std::unordered_set<size_t> index_to_delete{0};

        removeElements(a, index_to_delete);
        EXPECT_EQ(a, std::vector<int>());
    }
    {
        std::vector<int> a{};
        std::unordered_set<size_t> index_to_delete{};

        removeElements(a, index_to_delete);
        EXPECT_EQ(a, std::vector<int>());
    }
    {
        std::vector<int> a{1, 2, 3, 4};
        std::unordered_set<size_t> index_to_delete{0, 3};

        removeElements(a, index_to_delete);
        EXPECT_EQ(a, std::vector<int>({2, 3}));
    }
    {
        std::vector<int> a{1, 2, 3, 4, 5};
        std::unordered_set<size_t> index_to_delete{4};

        removeElements(a, index_to_delete);
        EXPECT_EQ(a, std::vector<int>({1, 2, 3, 4}));
    }
    {
        std::vector<int> a{1, 2, 3, 4, 5, 6, 7, 8};
        std::unordered_set<size_t> index_to_delete{0, 2, 3, 5, 6};

        removeElements(a, index_to_delete);
        EXPECT_EQ(a, std::vector<int>({2, 5, 8}));
    }
}

TEST(jsonUtils, dfs1)
{
    std::string raw = R"(
        {
          "widget": {
            "debug": "on",
            "window": {
              "title": "Sample Konfabulator Widget",
              "height": 500
            },
            "image": {
              "src": "Images/Sun.png"
            },
            "arr": [1, 2, 3, 100]
          }
        }
    )";
    auto res = flattenJsonKvs(raw);

    EXPECT_EQ(res.size(), 5);
    EXPECT_EQ(res[Key("widget.window.title")].is_string(), true);
    EXPECT_EQ(res[Key("widget.window.height")].is_number(), true);
    EXPECT_EQ(res[Key("widget.image.src")].is_string(), true);
    EXPECT_EQ(res[Key("widget.debug")].is_string(), true);
    EXPECT_EQ(res[Key("widget.arr")].is_array(), true);

    EXPECT_EQ(res[Key("widget.window.title")].get<std::string>(), "Sample Konfabulator Widget");
    EXPECT_EQ(res[Key("widget.window.height")].get<int>(), 500);
    EXPECT_EQ(res[Key("widget.image.src")], "Images/Sun.png");
    EXPECT_EQ(res[Key("widget.debug")].get<std::string>(), "on");
    EXPECT_EQ(res[Key("widget.arr")][0], 1);
    EXPECT_EQ(res[Key("widget.arr")][1], 2);
    EXPECT_EQ(res[Key("widget.arr")][2], 3);
    EXPECT_EQ(res[Key("widget.arr")][3], 100);
}

TEST(jsonUtils, dfs2)
{
    {
        std::string raw = R"({})";
        auto res = flattenJsonKvs(raw);
        EXPECT_EQ(res.size(), 0);
    }
    {
        std::string raw = R"([])";
        auto res = flattenJsonKvs(raw);
        EXPECT_EQ(res.size(), 1);
        EXPECT_EQ(res.begin()->second.is_array(), true);
        EXPECT_EQ(res.begin()->second.empty(), true);
    }

    {
        std::string raw = R"(
            {
                "arr":
                    [{
                    "widget":
                        {
                            "debug": true,
                            "window": {
                                "title": "Sample Konfabulator Widget",
                                "height": 500
                            }
                        }
                    }],
                "hello": "world"
            }
        )";
        auto res = flattenJsonKvs(raw);
        EXPECT_EQ(res.size(), 4);

        EXPECT_EQ(res[Key("arr.0.widget.window.height")], 500);
        EXPECT_EQ(res[Key("arr.0.widget.window.title")], "Sample Konfabulator Widget");
        EXPECT_EQ(res[Key("arr.0.widget.debug")], true);
        EXPECT_EQ(res[Key("hello")], "world");
    }
}

TEST(dynamicBitSet, base)
{
    DynamicBitSet s1(7);
    EXPECT_EQ(s1.toSet(), std::set<size_t>{});
    s1.fill();
    EXPECT_EQ(s1.toSet(), std::set<size_t>({0, 1, 2, 3, 4, 5, 6}));

    DynamicBitSet s2(65);
    s2.set(65);
    EXPECT_EQ(s2.toSet(1), std::set<size_t>({65}));

    DynamicBitSet s3(3);
    s3.set(2);
    s3.flip();
    EXPECT_EQ(s3.toSet(1), std::set<size_t>({1, 3}));

    DynamicBitSet s4(64);
    s4.set(1);
    s4.set(64);

    EXPECT_EQ(s4.toSet(0), std::set<size_t>({0, 63}));
    {
        DynamicBitSet set_and(64);
        set_and.set(63);
        set_and.set(64);
        set_and &= s4;
        EXPECT_EQ(set_and.toSet(1), std::set<size_t>({64}));
    }
    {
        DynamicBitSet set_or(64);
        set_or.set(63);
        set_or.set(64);
        set_or |= s4;
        EXPECT_EQ(set_or.toSet(1), std::set<size_t>({1, 63, 64}));
    }

    DynamicBitSet s5(128);
    s5.set(1);
    s5.set(64);
    s5.set(65);
    s5.set(128);
    EXPECT_EQ(s5.toSet(1), std::set<size_t>({1, 64, 65, 128}));
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}