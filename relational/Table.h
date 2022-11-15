#include "../typedefs.h"
#include "Column.h"

// 逻辑行，引用底层的列式存储
struct RowRef
{
    size_t index; // row number, begin from 1
    std::vector<ColumnPtr> columns;
};

// 物理行
struct Rows
{
    std::vector<ColumnPtr> columns;

    size_t size() const {
        if (!columns.empty())
            return columns[0]->size();
        return 0;
    }
};

class Table
{
public:
    Table(const std::string& table_name_) : table_name(table_name_) {}

    void addColumn(ColumnPtr column) {
        columns.emplace(column->column_name, column);
    }

    size_t size() const {
        if (!columns.empty())
            return columns.begin()->second->size();
        return 0;
    }

    void insertRows(const Rows& rows) const {
        for (const auto& column : rows.columns)
        {
            auto iter = columns.find(column->column_name);
            if (iter != columns.end() && iter->second->type == column->type)
            {
                iter->second->appendBlock(column);
            }
        }
    }

    ColumnPtr operator[](const std::string& column_name) {
        auto iter = columns.find(column_name);
        if (iter == columns.end())
            throw Poco::NotFoundException("can't found column " + column_name + " in table " + table_name);
        return iter->second;
    }


private:
    std::string table_name;
    std::unordered_map<std::string, ColumnPtr> columns;
};