#include "Table.h"
#include "Function.h"
#include "PlanNode.h"

TEST(table, Base)
{
    Table table("table1");
    table.addColumn(std::make_shared<ColumnString>("student"));
    table.addColumn(std::make_shared<ColumnDecimal>("score"));
    table.addColumn(std::make_shared<ColumnDateTime>("date"));

    Rows rows;
    auto student_data = std::make_shared<ColumnDataBase<String>>();
    student_data->insert("ljz");
    student_data->insert("wx");
    auto score_data = std::make_shared<ColumnDataBase<Decimal>>();
    score_data->insert(100.0);
    score_data->insert(80.2);
    auto date_data = std::make_shared<ColumnDataBase<DateTime>>();
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

TEST(table, SimpleAggFunc)
{
    Table table("table1");
    table.addColumn(std::make_shared<ColumnDecimal>("score"));

    Rows rows;
    auto score_data = std::make_shared<ColumnDataBase<Decimal>>();
    score_data->insert(100.0);
    score_data->insert(99.0);
    score_data->insert(80.2);
    score_data->insert(50.0);
    rows.columns.push_back(std::make_shared<ColumnDecimal>("score", score_data));

    table.insertRows(rows);

    auto column = std::dynamic_pointer_cast<ColumnDecimal>(table["score"]);
    ASSERT_NE(column, nullptr);

    SumOperator<Decimal> op;
    column->handleEverything(std::ref(op)); // NOTE: use std::ref, avoid copying SumOperator
    ASSERT_EQ(op.result(), 329.2);

    AvgOperator<Decimal> op2;
    column->handleEverything(std::ref(op2));
    ASSERT_EQ(op2.result().value_or(0), 82.3);

    CountOperator<Decimal> op3;
    column->handleEverything(std::ref(op3));
    ASSERT_EQ(op3.result(), 4);

    MaxOperator<Decimal> op4;
    column->handleEverything(std::ref(op4));
    ASSERT_EQ(op4.result().value_or(0), 100.0);

    MinOperator<Decimal> op5;
    column->handleEverything(std::ref(op5));
    ASSERT_EQ(op5.result().value_or(0), 50.0);
}

TEST(table, SerializeAndDeserialize)
{
    Table table("table1");
    table.addColumn(std::make_shared<ColumnString>("student"));
    table.addColumn(std::make_shared<ColumnDecimal>("score"));
    table.addColumn(std::make_shared<ColumnDateTime>("date"));

    Rows rows;
    auto student_data = std::make_shared<ColumnDataBase<String>>();
    student_data->insert("ljz");
    student_data->insert("wx");
    auto score_data = std::make_shared<ColumnDataBase<Decimal>>();
    score_data->insert(100.0);
    score_data->insert(80.2);
    auto date_data = std::make_shared<ColumnDataBase<DateTime>>();
    date_data->insert(DateTime("2011-04-19 04:33:15"));
    date_data->insert(DateTime("2021-01-29 00:33:15"));

    rows.columns.push_back(std::make_shared<ColumnString>("student", student_data));
    rows.columns.push_back(std::make_shared<ColumnDecimal>("score", score_data));
    rows.columns.push_back(std::make_shared<ColumnDateTime>("date", date_data));

    table.insertRows(rows);

    auto score_column_ptr = std::dynamic_pointer_cast<ColumnDecimal>(table["score"]);
    score_column_ptr->data->insert(123.6);

    table.setDocId(100);
    ASSERT_EQ(table.docId(), 100);

    WriteBuffer write_buffer;
    WriteBufferHelper write_helper(write_buffer);
    table.serialize(write_helper);

    auto string_ref = write_buffer.string_ref();
    std::string cp1(string_ref.first, string_ref.second);
    write_buffer.clear();

    ReadBuffer read_buffer;
    read_buffer.append(string_ref.first, string_ref.second);
    ReadBufferHelper read_helper(read_buffer);
    table.deserialize(read_helper);

    table.serialize(write_helper);
    auto string_ref2 = write_buffer.string_ref();
    std::string cp2(string_ref2.first, string_ref2.second);
    ASSERT_EQ(cp1, cp2);
}

TEST(plan_node, SimpleFilter)
{
    Rows rows;
    auto number_data = std::make_shared<ColumnDataBase<Int>>(std::initializer_list<Int>{100, 200, 300, 500});
    rows.columns.push_back(std::make_shared<ColumnInt>("number", number_data));

    // number < 300
    {
        ExpressionPtr where = std::make_shared<Expression>(Symbol(SymbolType::Less));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Id, ColumnName{.column_name = "number"})));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Value, Value(300ll))));

        PlanNodePtr filter = std::make_shared<FilterNode>(where);
        Rows res = filter->transform(rows);

        ASSERT_EQ(res.size(), 2);
        auto number_column = std::dynamic_pointer_cast<ColumnInt>(res.columns[0]);
        ASSERT_NE(number_column, nullptr);

        ASSERT_EQ(number_column->data->size(), 2);
        ASSERT_EQ(number_column->data->operator[](0), 100);
        ASSERT_EQ(number_column->data->operator[](1), 200);
    }

    // number != 100
    {
        ExpressionPtr where = std::make_shared<Expression>(Symbol(SymbolType::NotEqual));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Id, ColumnName{.column_name = "number"})));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Value, Value(100ll))));

        PlanNodePtr filter = std::make_shared<FilterNode>(where);
        Rows res = filter->transform(rows);

        ASSERT_EQ(res.size(), 3);
        auto number_column = std::dynamic_pointer_cast<ColumnInt>(res.columns[0]);
        ASSERT_NE(number_column, nullptr);

        ASSERT_EQ(number_column->data->size(), 3);
        ASSERT_EQ(number_column->data->operator[](0), 200);
        ASSERT_EQ(number_column->data->operator[](1), 300);
        ASSERT_EQ(number_column->data->operator[](2), 500);
    }

    // number > 100
    {
        ExpressionPtr where = std::make_shared<Expression>(Symbol(SymbolType::Great));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Id, ColumnName{.column_name = "number"})));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Value, Value(100ll))));

        PlanNodePtr filter = std::make_shared<FilterNode>(where);
        Rows res = filter->transform(rows);

        ASSERT_EQ(res.size(), 3);
        auto number_column = std::dynamic_pointer_cast<ColumnInt>(res.columns[0]);
        ASSERT_NE(number_column, nullptr);

        ASSERT_EQ(number_column->data->size(), 3);
        ASSERT_EQ(number_column->data->operator[](0), 200);
        ASSERT_EQ(number_column->data->operator[](1), 300);
        ASSERT_EQ(number_column->data->operator[](2), 500);
    }

    // number <= 300
    {
        ExpressionPtr where = std::make_shared<Expression>(Symbol(SymbolType::LessEqual));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Id, ColumnName{.column_name = "number"})));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Value, Value(300ll))));

        PlanNodePtr filter = std::make_shared<FilterNode>(where);
        Rows res = filter->transform(rows);

        ASSERT_EQ(res.size(), 3);
        auto number_column = std::dynamic_pointer_cast<ColumnInt>(res.columns[0]);
        ASSERT_NE(number_column, nullptr);

        ASSERT_EQ(number_column->data->size(), 3);
        ASSERT_EQ(number_column->data->operator[](0), 100);
        ASSERT_EQ(number_column->data->operator[](1), 200);
        ASSERT_EQ(number_column->data->operator[](2), 300);
    }

    // number >= 300
    {
        ExpressionPtr where = std::make_shared<Expression>(Symbol(SymbolType::GreatEqual));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Id, ColumnName{.column_name = "number"})));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Value, Value(300ll))));

        PlanNodePtr filter = std::make_shared<FilterNode>(where);
        Rows res = filter->transform(rows);

        ASSERT_EQ(res.size(), 2);
        auto number_column = std::dynamic_pointer_cast<ColumnInt>(res.columns[0]);
        ASSERT_NE(number_column, nullptr);

        ASSERT_EQ(number_column->data->size(), 2);
        ASSERT_EQ(number_column->data->operator[](0), 300);
        ASSERT_EQ(number_column->data->operator[](1), 500);
    }

    // number == 100
    {
        ExpressionPtr where = std::make_shared<Expression>(Symbol(SymbolType::Equal));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Id, ColumnName{.column_name = "number"})));
        where->addChild(std::make_shared<Expression>(Symbol(SymbolType::Value, Value(100ll))));

        PlanNodePtr filter = std::make_shared<FilterNode>(where);
        Rows res = filter->transform(rows);

        ASSERT_EQ(res.size(), 1);
        auto number_column = std::dynamic_pointer_cast<ColumnInt>(res.columns[0]);
        ASSERT_NE(number_column, nullptr);

        ASSERT_EQ(number_column->data->size(), 1);
        ASSERT_EQ(number_column->data->operator[](0), 100);
    }
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}