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

//TEST(extractor, CSVExtractor)
//{
//    auto reader = std::make_unique<TxtLineReader>(ROOT_PATH + "/articles/small.csv");
//    ASSERT_NE(reader, nullptr);
//    CSVExtractor extractor("small", std::move(reader));
//
//    auto res = extractor.extract();
//    ASSERT_EQ(res.is_valid, true);
//    auto table = res.table;
//
//    ASSERT_EQ(table.docId(), 0);
//    ASSERT_EQ(table.size(), 3);
//    ASSERT_EQ(table["id"]->type, ColumnType::Int);
//    ASSERT_EQ(table["price"]->type, ColumnType::Decimal);
//    ASSERT_EQ(table["dish_id"]->type, ColumnType::Int);
//    ASSERT_EQ(table["created_at"]->type, ColumnType::DateTime);
//    ASSERT_EQ(table["desc"]->type, ColumnType::String);
//
//    auto column_id = std::dynamic_pointer_cast<ColumnInt>(table["id"]);
//    auto column_price = std::dynamic_pointer_cast<ColumnDecimal>(table["price"]);
//    auto column_dish_id = std::dynamic_pointer_cast<ColumnInt>(table["dish_id"]);
//    auto column_created_at = std::dynamic_pointer_cast<ColumnDateTime>(table["created_at"]);
//    auto column_desc = std::dynamic_pointer_cast<ColumnString>(table["desc"]);
//
//    ASSERT_EQ(column_id && column_price && column_dish_id && column_created_at && column_desc, true);
//
//    ASSERT_EQ(column_id->data->operator[](0), 1);
//    ASSERT_EQ(column_id->data->operator[](1), 2);
//    ASSERT_EQ(column_id->data->operator[](2), 3);
//
//    ASSERT_EQ(column_price->data->operator[](0), 0.4);
//    ASSERT_EQ(column_price->data->operator[](1), 0.6);
//    ASSERT_EQ(column_price->data->operator[](2), 0.4);
//
//    ASSERT_EQ(column_dish_id->data->is_null(0), true);
//    ASSERT_EQ(column_dish_id->data->is_null(1), false);
//    ASSERT_EQ(column_dish_id->data->operator[](1), 2);
//    ASSERT_EQ(column_dish_id->data->is_null(2), false);
//    ASSERT_EQ(column_dish_id->data->operator[](2), 3);
//
//    ASSERT_EQ(column_created_at->data->operator[](0), DateTime("2011-03-28 15:00:44"));
//    ASSERT_EQ(column_created_at->data->operator[](1), DateTime("2011-03-28 15:01:13"));
//    ASSERT_EQ(column_created_at->data->operator[](2), DateTime("2011-03-28 15:01:40"));
//
//    ASSERT_EQ(column_desc->data->operator[](0), "hello ");
//    ASSERT_EQ(column_desc->data->operator[](1), " world");
//    ASSERT_EQ(column_desc->data->operator[](2), "a b c");
//}


int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}