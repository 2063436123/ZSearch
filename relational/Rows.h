#pragma once

#include <utility>

#include "../typedefs.h"
#include "Column.h"

// 物理行，在 ClickHouse 中被叫做 chunk.
class Rows
{
public:
    std::vector<ColumnPtr> getColumns() const
    {
        return columns;
    }

    void addColumn(ColumnPtr&& ptr)
    {
        columns.push_back(ptr);
    }

    void addColumn(const ColumnPtr& ptr)
    {
        columns.push_back(ptr->copy());
    }

    ColumnPtr operator[](const std::string& column_name) const
    {
        auto iter = std::find_if(columns.begin(), columns.end(), [&column_name](const ColumnPtr& ptr) {
            return ptr->column_name == column_name;
        });
        if (iter == columns.end())
            THROW(Poco::NotFoundException("can't found column " + column_name + " in Rows"));
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

private:
    std::vector<ColumnPtr> columns;
};

// 逻辑行，引用底层的列式存储
struct RowsRef
{
public:
    std::vector<ColumnPtr> getColumns() const
    {
        return columns;
    }

    RowsRef(const RowsRef& rhs)
    {
        *this = rhs;
    }

    RowsRef(RowsRef&& rhs)
    {
        *this = rhs;
    }

    RowsRef& operator=(const RowsRef& rhs)
    {
        if (columns != rhs.columns)
            THROW(Poco::InvalidArgumentException("unmatched columns type when reset RowsRef"));
        indexs = rhs.indexs;
        return *this;
    }

    RowsRef& operator=(RowsRef&& rhs)
    {
        if (columns != rhs.columns)
            THROW(Poco::InvalidArgumentException("unmatched columns type when reset RowsRef"));
        indexs = std::move(rhs.indexs);
        return *this;
    }

    ColumnPtr operator[](const std::string& column_name) const
    {
        auto iter = std::find_if(columns.begin(), columns.end(), [&column_name](const ColumnPtr& ptr) {
            return ptr->column_name == column_name;
        });
        if (iter == columns.end())
            THROW(Poco::NotFoundException("can't found column " + column_name + " in RowsRef"));
        return *iter;
    }

    explicit RowsRef(std::vector<ColumnPtr> columns_) : columns(std::move(columns_)) { }

    void addIndex(size_t index)
    {
        indexs.insert(index);
    }

    Rows materialize() const
    {
        Rows rows;
        for (const auto& column : columns)
        {
            ColumnPtr ptr = column->copyWithoutData();
            for (size_t index : indexs)
            {
                ptr->insert((*column)[index]);
            }
            rows.addColumn(std::move(ptr));
        }
        return rows;
    }

    RowsRef operator&(const RowsRef& rhs) const
    {
        if (columns != rhs.columns)
            THROW(Poco::InvalidArgumentException("unmatched columns type in RowsRef"));
        RowsRef res(columns);
        std::set_intersection(indexs.begin(), indexs.end(), rhs.indexs.begin(), rhs.indexs.end(), std::inserter(res.indexs, res.indexs.begin()));
        return res;
    }

    RowsRef operator|(const RowsRef& rhs) const
    {
        if (columns != rhs.columns)
            THROW(Poco::InvalidArgumentException("unmatched columns type in RowsRef"));
        RowsRef res(columns);
        std::set_union(indexs.begin(), indexs.end(), rhs.indexs.begin(), rhs.indexs.end(), std::inserter(res.indexs, res.indexs.begin()));
        return res;
    }

    RowsRef operator-(const RowsRef& rhs) const
    {
        if (columns != rhs.columns)
            THROW(Poco::InvalidArgumentException("unmatched columns type in RowsRef"));
        RowsRef res(columns);
        std::set_difference(indexs.begin(), indexs.end(), rhs.indexs.begin(), rhs.indexs.end(), std::inserter(res.indexs, res.indexs.begin()));
        return res;
    }

private:
    std::set<size_t> indexs; // row number, begin from 1
    std::vector<ColumnPtr> columns;
};