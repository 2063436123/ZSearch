#pragma once

#include "utils/SerializerUtils.h"
#include "../typedefs.h"
#include "utils/TimeUtils.h"

enum class ColumnType
{
    Int,
    Decimal,
    String,
    DateTime
};

// column type storage
template<typename T>
class ColumnDataBase
{
public:
    ColumnDataBase() = default;

    explicit ColumnDataBase(const std::initializer_list<T>& list) : data(list) {
        null_map.insert(null_map.end(), data.size(), 0);
    }

    size_t size() const
    {
        return data.size();
    }

//    auto begin() const
//    {
//        return const_cast<const std::vector<T> &>(data).begin();
//    }
//
//    auto end() const
//    {
//        return const_cast<const std::vector<T> &>(data).end();
//    }

    T operator[](size_t index) const
    {
        if (index >= data.size())
            THROW(Poco::RangeException("ColumnDataBase operator[] out of range"));
        return data[index];
    }

    bool is_null(size_t index) const
    {
        if (index >= null_map.size())
            THROW(Poco::RangeException("ColumnDataBase is_null(size_t) out of range"));
        return null_map[index];
    }

    T &operator[](size_t index)
    {
        if (index >= data.size())
            THROW(Poco::RangeException("ColumnDataBase operator[] out of range"));
        return data[index];
    }

    void insert(const T &value)
    {
        assert(data.size() == null_map.size());
        if (!is_ordered)
        {
            data.push_back(value);
            null_map.push_back(0);
        }
        else
        {
            auto iter = std::lower_bound(data.begin(), data.end(), value);
            // allow duplicate values.
            // if (iter == end() || *iter != value)
            data.insert(iter, value);
            null_map.insert(iter - data.begin() + null_map.begin(), 0);
        }
    }

    void insertNull()
    {
        assert(data.size() == null_map.size());
        if (!is_ordered)
        {
            data.push_back(T{});
            null_map.push_back(1);
        }
        else
        {
            // allow duplicate nulls.
            // nulls will be set at front.
            data.insert(data.begin(), T{});
            null_map.insert(null_map.begin(), 1);
        }
    }

    void appendBlock(std::shared_ptr<ColumnDataBase<T>> rhs)
    {
        assert(data.size() == null_map.size());
//        auto rhs_data = std::dynamic_pointer_cast<ColumnDataBase<T>>(rhs);
//        if (!rhs_data)
//            THROW(Poco::InvalidArgumentException(
//                    "ColumnDatabase::appendBlock, can't convert ColumnDataPtr to ColumnDataBasePtr"));
        if (!is_ordered)
        {
            std::copy(rhs->data.begin(), rhs->data.end(), std::back_inserter(data));
            std::copy(rhs->null_map.begin(), rhs->null_map.end(), std::back_inserter(null_map));
        }
        else
        {
            THROW(Poco::NotImplementedException("appendBlock"));
        }
    }

    void appendBlock(const std::vector<T>& data_)
    {
        assert(data.size() == null_map.size());
        if (!is_ordered)
        {
            std::copy(data_.begin(), data_.end(), std::back_inserter(data));
            null_map.insert(null_map.end(), data_.size(), 0);
        }
        else
        {
            THROW(Poco::NotImplementedException("appendBlock"));
        }
    }

    void setOrdered()
    {
        is_ordered = true;
    }

    std::shared_ptr<ColumnDataBase<T>> copy() const
    {
        auto ret = std::make_shared<ColumnDataBase<T>>();
        ret->data = data;
        ret->null_map = null_map;
        ret->is_ordered = is_ordered;
        return ret;
    }

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeNumber(is_ordered);
        helper.writeLinearContainer(data);
        helper.writeLinearContainer(null_map);
    }

    static std::shared_ptr<ColumnDataBase<T>> deserialize(ReadBufferHelper &helper)
    {
        auto ret = std::make_shared<ColumnDataBase<T>>();
        if (helper.readNumber<bool>())
            ret->setOrdered();
        ret->appendBlock(helper.readLinearContainer<std::vector, T>());
        ret->null_map = helper.readLinearContainer<std::vector, uint8_t>();
        return ret;
    }

private:
    std::vector<T> data;
    std::vector<uint8_t> null_map;
    bool is_ordered = false;
};

template<typename T>
using ColumnDataPtr = std::shared_ptr<ColumnDataBase<T>>;

template<typename T>
ColumnType getType()
{
    if constexpr (std::is_same_v<T, int64_t>)
        return ColumnType::Int;
    if constexpr (std::is_same_v<T, double>)
        return ColumnType::Decimal;
    if constexpr (std::is_same_v<T, std::string>)
        return ColumnType::String;
    if constexpr (std::is_same_v<T, DateTime>)
        return ColumnType::DateTime;
    THROW(UnreachableException("in getNodeType"));
}

ColumnType getTypeByName(const std::string& name)
{
    if (name == "Int")
        return ColumnType::Int;
    if (name == "Decimal")
        return ColumnType::Decimal;
    if (name == "String")
        return ColumnType::String;
    if (name == "DateTime")
        return ColumnType::DateTime;
    THROW(UnreachableException("in getTypeByName"));
}

template<typename T>
std::string getNameByType()
{
    if constexpr (std::is_same_v<T, int64_t>)
        return "Int";
    if constexpr (std::is_same_v<T, double>)
        return "Decimal";
    if constexpr (std::is_same_v<T, std::string>)
        return "String";
    if constexpr (std::is_same_v<T, DateTime>)
        return "DateTime";
    THROW(UnreachableException("in getNameByType"));
}

