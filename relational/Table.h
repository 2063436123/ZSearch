#pragma once

#include "../typedefs.h"
#include "Column.h"
#include "Rows.h"
#include "utils/SerializerUtils.h"

class Table
{
public:
    Table() = default;

    Table(const std::string& table_name_) : table_name(table_name_) {}

    Table(const std::string& table_name_, const Rows& rows) : table_name(table_name_) {
        for (const auto& column : rows.getColumns())
        {
            addColumn(column->copy());
        }
    }

    std::string name() const {
        return table_name;
    }

    size_t docId() const {
        return doc_id;
    }

    void setDocId(size_t doc_id_) {
        doc_id = doc_id_;
    }

    void addColumn(ColumnPtr column) {
        columns.emplace(column->column_name, column);
    }

    size_t size() const {
        if (!columns.empty())
            return columns.begin()->second->size();
        return 0;
    }

    void insertRows(const Rows& rows) const {
        for (const auto& column : rows.getColumns())
        {
            auto iter = columns.find(column->column_name);
            if (iter != columns.end() && iter->second->type == column->type)
            {
                iter->second->appendBlock(column);
            }
        }
    }

    ColumnPtr operator[](const std::string& column_name) const {
        auto iter = columns.find(column_name);
        if (iter == columns.end())
            throw Poco::NotFoundException("can't found column " + column_name + " in table " + table_name);
        return iter->second;
    }

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeNumber(doc_id);
        helper.writeString(table_name);
        helper.writeNumber(columns.size());
        for (const auto& pair : columns)
        {
            // key:column_name == value:column->column_name
            // in other words, pair.first is inside pair.second->column_name
            pair.second->serialize(helper);
        }
    }

    static Table deserialize(ReadBufferHelper &helper)
    {
        auto doc_id = helper.readNumber<size_t>();
        auto table_name = helper.readString();
        Table table(table_name);
        table.setDocId(doc_id);

        auto size = helper.readNumber<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            ColumnPtr column = ColumnBase::deserialize(helper);
            table.addColumn(column);
        }
        return table;
    }

    Rows dumpRows() const {
        Rows rows;
        for (const auto& pair : columns)
        {
            rows.addColumn(pair.second);
        }
        return rows;
    }

private:
    size_t doc_id = 0;
    std::string table_name;
    std::unordered_map<std::string, ColumnPtr> columns;
};
using TableMap = std::unordered_map<std::string, Table>;