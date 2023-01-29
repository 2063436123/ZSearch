#include "../typedefs.h"
#include "utils/json.hpp"
#include <gtest/gtest.h>

using json = nlohmann::json;

void dfs(const json& data)
{
    if (data.is_object() || data.is_array())
    {
        for (auto iter = data.begin(); iter != data.end(); ++iter)
        {
            std::cout << "key: " << iter.key() << std::endl;
            dfs(iter.value());
        }
    }
    else
    {
        std::cout << to_string(data) << std::endl;
    }
}

TEST(Json, base)
{
    std::string raw = R"(
        {
          "widget": {
            "debug": "on",
            "window": {
              "title": "Sample Konfabulator Widget",
              "name": "main_window",
              "width": 500,
              "height": 500
            },
            "image": {
              "src": "Images/Sun.png",
              "name": "sun1",
              "hOffset": 250,
              "vOffset": 250,
              "alignment": "center"
            },
            "text": {
              "data": "Click Here",
              "size": 36,
              "style": "bold",
              "name": "text1",
              "hOffset": 250,
              "vOffset": 100,
              "alignment": "center",
              "onMouseUp": "sun1.opacity = (sun1.opacity / 100) * 90;"
            }
          }
        }
    )";

    json data = json::parse(raw);
    EXPECT_EQ(data["widget"].is_object(), true);
    dfs(data);
}

int main() {
    testing::InitGoogleTest();

    return RUN_ALL_TESTS();
}