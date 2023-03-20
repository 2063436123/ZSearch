#include <Poco/StringTokenizer.h>
#include <Poco/StreamCopier.h>
#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include "../typedefs.h"

TEST(Poco, StringTokenizer) {
    std::string line = "hello world fuck";
    Poco::StringTokenizer tk(line, " ", Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY |
                                        Poco::StringTokenizer::Options::TOK_TRIM);
    ASSERT_EQ(tk.count(), 3);
    ASSERT_EQ(tk[0], "hello");
    ASSERT_EQ(tk[1], "world");
    ASSERT_EQ(tk[2], "fuck");
}

TEST(Poco, StreamCopier)
{
    std::ifstream fin(ROOT_PATH + "/articles/single-jsons/css.json");
    std::string full_text;
    Poco::StreamCopier::copyToString64(fin, full_text);
    EXPECT_EQ(full_text,
        "{\n"
        "  \"author\": \"ljz\",\n"
        "  \"widget\": {\n"
        "    \"debug\": \"on\",\n"
        "    \"window\": {\n"
        "      \"title\": \"Sample Konfabulator Widget\",\n"
        "      \"name\": \"main_window\",\n"
        "      \"width\": 500,\n"
        "      \"height\": 500\n"
        "    },\n"
        "    \"image\": {\n"
        "      \"src\": \"Images/Sun.png\",\n"
        "      \"name\": \"sun1\",\n"
        "      \"hOffset\": 250,\n"
        "      \"vOffset\": 250,\n"
        "      \"alignment\": \"center\"\n"
        "    },\n"
        "    \"text\": {\n"
        "      \"data\": \"Click Here\",\n"
        "      \"size\": 36,\n"
        "      \"style\": \"bold\",\n"
        "      \"name\": \"text1\",\n"
        "      \"hOffset\": 250,\n"
        "      \"vOffset\": 100,\n"
        "      \"alignment\": \"center\",\n"
        "      \"onMouseUp\": \"sun1.opacity = (sun1.opacity / 100) * 90;\"\n"
        "    }\n"
        "  }\n"
        "}");
}

int main() {
    testing::InitGoogleTest();

    return RUN_ALL_TESTS();
}