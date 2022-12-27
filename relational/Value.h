#pragma once
#include "../typedefs.h"
#include "utils/TimeUtils.h"

using Int = int64_t;
using Decimal = double;
using String = std::string;
using DateTime = DateTime;

enum class ValueType
{
    Null,
    String,
    Int,
    Decimal,
    DateTime,
    ValueList // tuple like (1, 2, 3), nesting is not supported.
};

class Value;
using Values = std::vector<Value>;
class Value
{
public:
    Value() : type(ValueType::Null)
    {

    }

    Value(const std::string& string) : type(ValueType::String), var(string)
    {

    }

    Value(int64_t i) : type(ValueType::Int), var(i)
    {

    }

    Value(double decimal) : type(ValueType::Decimal), var(decimal)
    {

    }

    Value(DateTime date_time) : type(ValueType::DateTime), var(date_time)
    {

    }

    Value(Values values) : type(ValueType::ValueList)
    {
        for (const auto& value : values)
        {
            if (value.type == ValueType::ValueList)
                throw Poco::InvalidArgumentException("nesting ValueList is not supported!");
        }
        var = values;
    }

    ValueType getValueType() const
    {
        return type;
    }

    template<typename T>
    T as() const
    {
        return std::get<T>(var);
    }

    Value operator+(const Value& rhs) const
    {
        if (type != rhs.type)
            throw Poco::InvalidArgumentException("Value type unmatch");
        if (type == ValueType::Int)
            return std::get<Int>(var) + std::get<Int>(rhs.var);
        if (type == ValueType::Decimal)
            return std::get<Decimal>(var) + std::get<Decimal>(rhs.var);
        throw UnreachableException("Value::operator+ unimplement this type.");
    }

    Value operator/(const Value& rhs) const
    {
        if (ValueType::Int != rhs.type)
            throw Poco::InvalidArgumentException("Value type unmatch");
        if (type == ValueType::Null)
            return {};
        if (type == ValueType::Int)
            return std::get<Int>(var) / std::get<Int>(rhs.var);
        if (type == ValueType::Decimal)
            return std::get<Decimal>(var) / std::get<Int>(rhs.var);
        throw UnreachableException("Value::operator/ unimplement this type.");
    }

    bool operator==(const Value& rhs) const
    {
        return var == rhs.var;
    }

    bool operator!=(const Value& rhs) const
    {
        return var != rhs.var;
    }

    bool operator<(const Value& rhs) const
    {
        if (type != rhs.type)
            throw Poco::InvalidArgumentException("Value type unmatch");
        return var < rhs.var;
    }

    bool operator<=(const Value& rhs) const
    {
        if (type != rhs.type)
            throw Poco::InvalidArgumentException("value type unmatch");
        return var <= rhs.var;
    }

    bool operator>(const Value& rhs) const
    {
        if (type != rhs.type)
            throw Poco::InvalidArgumentException("value type unmatch");
        return var > rhs.var;
    }

    bool operator>=(const Value& rhs) const
    {
        if (type != rhs.type)
            throw Poco::InvalidArgumentException("value type unmatch");
        return var >= rhs.var;
    }

private:
    ValueType type;
    std::variant<std::string, int64_t, double, DateTime, Values> var;
};

// id, such as table.column
// if only term like "hello", equal to only column
struct IdentifierName
{
    std::string table_name;
    std::string column_name;

    bool operator==(const IdentifierName& rhs) const
    {
        return table_name == rhs.table_name && column_name == rhs.column_name;
    }

    std::string getName() const {
        return table_name + '.' + column_name;
    }

    static IdentifierName New(const std::string& str)
    {
        IdentifierName name;
        size_t pos = str.find(".");
        if (pos != std::string::npos)
        {
            name.table_name = str.substr(0, pos);
            name.column_name = str.substr(pos + 1);
        }
        else
        {
            name.column_name = str;
        }
        return name;
    }
};