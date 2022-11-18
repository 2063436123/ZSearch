#pragma once

#include "../typedefs.h"
#include "Column.h"

// 物理行，在 ClickHouse 中被叫做 chunk.
struct Rows
{
    std::vector<ColumnPtr> columns;

    ColumnPtr operator[](const std::string& column_name) const
    {
        auto iter = std::find_if(columns.begin(), columns.end(), [&column_name](const ColumnPtr& ptr) {
            return ptr->column_name == column_name;
        });
        if (iter == columns.end())
            throw Poco::NotFoundException("can't found column " + column_name + " in Rows");
        return *iter;
    }

    // set columns type, name to rows. no data copy.
    void depict(const Rows& rows)
    {
        columns.resize(rows.columns.size());
        for (size_t i = 0; i < rows.columns.size(); i++)
        {
            columns[i] = rows.columns[i]->copyWithoutData();
        }
    }

    size_t size() const {
        if (!columns.empty())
            return columns[0]->size();
        return 0;
    }
};
