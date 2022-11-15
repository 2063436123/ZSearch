#include "Table.h"
#include <fcntl.h>

TEST(table, Base)
{
    Table table("table1");
    table.addColumn(std::make_shared<ColumnString>("student"));
    table.addColumn(std::make_shared<ColumnDecimal>("score"));
    table.addColumn(std::make_shared<ColumnDateTime>("date"));

    Rows rows;
    auto student_data = std::make_shared<ColumnDataString>();
    student_data->insert("ljz");
    student_data->insert("wx");
    auto score_data = std::make_shared<ColumnDataDecimal>();
    score_data->insert(100.0);
    score_data->insert(80.2);
    auto date_data = std::make_shared<ColumnDataDateTime>();
    date_data->insert(DateTime("2011-04-19 04:33:15"));
    date_data->insert(DateTime("2021-01-29 00:33:15"));

    ASSERT_EQ(rows.size(), 0);
    rows.columns.push_back(std::make_shared<ColumnString>("student", student_data));
    ASSERT_EQ(rows.size(), 2);
    rows.columns.push_back(std::make_shared<ColumnDecimal>("score", score_data));
    rows.columns.push_back(std::make_shared<ColumnDateTime>("date", date_data));
    ASSERT_EQ(rows.size(), 2);

    ASSERT_EQ(table.size(), 0);
    table.insertRows(rows);
    ASSERT_EQ(table.size(), 2);

    ASSERT_EQ(table["student"]->is_ordered, false);
    ASSERT_EQ(table["student"]->column_name, "student");
    ASSERT_EQ(table["student"]->type, ColumnType::String);

    ASSERT_THROW(table["unknown"], Poco::NotFoundException);

    auto score_column_ptr = std::dynamic_pointer_cast<ColumnDecimal>(table["score"]);
    ASSERT_NE(score_column_ptr, nullptr);
    ASSERT_EQ(score_column_ptr->size(), 2);
    score_column_ptr->data->insert(123.6);
    ASSERT_EQ(score_column_ptr->size(), 3);

    ASSERT_EQ(score_column_ptr->data->operator[](0), 100.0);
    ASSERT_EQ(score_column_ptr->data->operator[](1), 80.2);
    ASSERT_EQ(score_column_ptr->data->operator[](2), 123.6);
    ASSERT_THROW(score_column_ptr->data->operator[](3), Poco::RangeException);
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}