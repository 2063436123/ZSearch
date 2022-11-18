#pragma once
#include "../typedefs.h"
#include "utils/TimeUtils.h"

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
            throw Poco::InvalidArgumentException("value type unmatch");
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