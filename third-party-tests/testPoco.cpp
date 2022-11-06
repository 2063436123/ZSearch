#include <Poco/StringTokenizer.h>
#include <iostream>
#include <string>
#include <gtest/gtest.h>

TEST(Poco, StringTokenizer) {
    std::string line = "hello world fuck";
    Poco::StringTokenizer tk(line, " ", Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY |
                                        Poco::StringTokenizer::Options::TOK_TRIM);
    ASSERT_EQ(tk.count(), 3);
    ASSERT_EQ(tk[0], "hello");
    ASSERT_EQ(tk[1], "world");
    ASSERT_EQ(tk[2], "fuck");
}

int main() {
    testing::InitGoogleTest();

    return RUN_ALL_TESTS();
}