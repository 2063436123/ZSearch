#pragma once

#include "../typedefs.h"
#include "Column.h"

// 物理行，在 ClickHouse 中被叫做 chunk.
struct Rows
{
    std::vector<ColumnPtr> columns;

    size_t size() const {
        if (!columns.empty())
            return columns[0]->size();
        return 0;
    }
};
