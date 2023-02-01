//
// Created by Hello Peter on 2023/1/27.
//
#pragma once
#include "../typedefs.h"
#include "utils/TimeUtils.h"
#include "DynamicArray.h"


class Value;
using Values = std::vector<Value>;

struct ArrayLabel {};

class Value
{
public:
    Value() : type(ValueType::Null)
    {

    }

    // 以此法构造的 Value 是一个数组，元素应该稍后被填充，元素的类型用 ValueType 表示（注意，非数组的 Value 元素类型也是用 ValueType 表示，需要用 isArray() 加以区分）
    Value(ArrayLabel, ValueType type_) : is_array(true), type(type_), var(DynamicArray(type))
    {
        // 在未正确实现拷贝构造函数时，DynamicArray 必须原地构造
//        var.emplace<DynamicArray>(type);
    }

    Value(ArrayLabel, ValueType type_, DynamicArray da) : is_array(true), type(type_), var(std::move(da))
    {
        // 在未正确实现拷贝构造函数时，DynamicArray 必须原地构造
//        var.emplace<DynamicArray>(type);
    }

    Value(bool b) : type(ValueType::Bool), var(b)
    {

    }

    // NOTE: 如果不为字符串常量特化，那么构造函数传入字符串常量会匹配到 bool 形参
    Value(const char* str) : Value(std::string(str)) {}

    Value(const std::string& string) : type(ValueType::String), var(string)
    {

    }

    Value(int i) : type(ValueType::Number), var(double(i))
    {

    }

    Value(double decimal) : type(ValueType::Number), var(decimal)
    {

    }

    Value(DateTime date_time) : type(ValueType::DateTime), var(date_time)
    {

    }

    ValueType getValueType() const
    {
        return type;
    }

    bool isNull() const
    {
        return type == ValueType::Null;
    }

    bool isBool() const
    {
        return type == ValueType::Bool;
    }

    bool isNumber() const
    {
        return type == ValueType::Number;
    }

    bool isString() const
    {
        return type == ValueType::String;
    }

    bool isDateTime() const
    {
        return type == ValueType::DateTime;
    }

    bool isArray() const
    {
        return is_array;
    }

    template<typename T>
    T as(size_t i = 0) const
    {
        if (!checkType<T>(type))
            THROW(Poco::LogicException("Value type error"));
        if (i == 0 && !is_array)
            return std::get<T>(var);
        if (is_array)
        {
            return get<DynamicArray>(var).get<T>(i);
        }
        THROW(Poco::NotImplementedException());
    }

    void doArrayHandler(const ValueArrayHandler<Bool>& bool_handler,
                        const ValueArrayHandler<Number>& number_handler,
                        const ValueArrayHandler<String>& string_handler,
                        const ValueArrayHandler<DateTime>& datetime_handler) const
    {
        if (!is_array)
            THROW(Poco::LogicException("only array value can call doArrayHandler()"));
        get<DynamicArray>(var).applyHandler(bool_handler, number_handler, string_handler, datetime_handler);
    }

    template<typename T>
    void doArrayHandler(const ValueArrayHandler<T>& array_handler) const
    {
        if (!is_array)
            THROW(Poco::LogicException("only array value can call doArrayHandler()"));
        if constexpr (std::is_same_v<T, Bool>)
            get<DynamicArray>(var).applyHandler(array_handler, PanicValueArrayHandler<Number>, PanicValueArrayHandler<String>, PanicValueArrayHandler<DateTime>);
        else if constexpr (std::is_same_v<T, Number>)
            get<DynamicArray>(var).applyHandler(PanicValueArrayHandler<Bool>, array_handler, PanicValueArrayHandler<String>, PanicValueArrayHandler<DateTime>);
        else if constexpr (std::is_same_v<T, String>)
            get<DynamicArray>(var).applyHandler(PanicValueArrayHandler<Bool>, PanicValueArrayHandler<Number>, array_handler, PanicValueArrayHandler<DateTime>);
        else if constexpr (std::is_same_v<T, DateTime>)
            get<DynamicArray>(var).applyHandler(PanicValueArrayHandler<Bool>, PanicValueArrayHandler<Number>, PanicValueArrayHandler<String>, array_handler);
        else
            THROW(UnreachableException());
    }


    void serialize(WriteBufferHelper& helper) const
    {
        helper.writeNumber<bool>(is_array);
        helper.writeNumber<ValueType>(type);
        if (!is_array)
        {
            switch (type)
            {
                case ValueType::Null:
                    THROW(Poco::NotImplementedException());
                case ValueType::Bool:
                    helper.writeNumber<Bool>(get<Bool>(var));
                    break;
                case ValueType::Number:
                    helper.writeNumber<Number>(get<Number>(var));
                    break;
                case ValueType::String:
                    helper.writeString(get<String>(var));
                    break;
                case ValueType::DateTime:
                    helper.writeDateTime(get<DateTime>(var));
                    break;
            }
        }
        else
        {
            get<DynamicArray>(var).serialize(helper);
        }
    }

    static Value deserialize(ReadBufferHelper& helper)
    {
        bool is_array = helper.readNumber<bool>();
        ValueType type = helper.readNumber<ValueType>();
        if (!is_array)
        {
            switch (type)
            {
                case ValueType::Null:
                    THROW(Poco::NotImplementedException());
                case ValueType::Bool:
                    return Value(helper.readNumber<Bool>());
                    break;
                case ValueType::Number:
                    return Value(helper.readNumber<Number>());
                    break;
                case ValueType::String:
                    return Value(helper.readString());
                    break;
                case ValueType::DateTime:
                    return Value(helper.readDateTime());
                    break;
            }
        }
        else
        {
            return Value(ArrayLabel{}, type, DynamicArray::deserialize(helper));
        }
    }

    bool operator<(const Value& rhs) const
    {
        if (type != rhs.type || is_array || rhs.is_array || isNull() || isBool())
            THROW(Poco::InvalidArgumentException("Value operator< invalid argument for " +
                totalType(is_array, type) + " vs " +
                totalType(rhs.is_array, rhs.type)));
        return var < rhs.var;
    }

    bool operator<=(const Value& rhs) const
    {
        if (type != rhs.type || is_array || rhs.is_array || isNull() || isBool())
            THROW(Poco::InvalidArgumentException("Value operator<= invalid argument for " +
                                                 totalType(is_array, type) + " vs " +
                                                 totalType(rhs.is_array, rhs.type)));
        return var <= rhs.var;
    }

    bool operator>(const Value& rhs) const
    {
        if (type != rhs.type || is_array || rhs.is_array || isNull() || isBool())
            THROW(Poco::InvalidArgumentException("Value operator> invalid argument for " +
                                                 totalType(is_array, type) + " vs " +
                                                 totalType(rhs.is_array, rhs.type)));
        return var > rhs.var;
    }

    bool operator>=(const Value& rhs) const
    {
        if (type != rhs.type || is_array || rhs.is_array || isNull() || isBool())
            THROW(Poco::InvalidArgumentException("Value operator>= invalid argument for " +
                                                 totalType(is_array, type) + " vs " +
                                                 totalType(rhs.is_array, rhs.type)));
        return var >= rhs.var;
    }

    bool operator!=(const Value& rhs) const
    {
        if (type != rhs.type)
            THROW(Poco::InvalidArgumentException("Value operator!= invalid argument for " +
                                                 totalType(is_array, type) + " vs " +
                                                 totalType(rhs.is_array, rhs.type)));
        return var != rhs.var;
    }

    bool operator==(const Value& rhs) const
    {
        if (type != rhs.type)
            THROW(Poco::InvalidArgumentException("Value operator== invalid argument for " +
                                                 totalType(is_array, type) + " vs " +
                                                 totalType(rhs.is_array, rhs.type)));
        return var == rhs.var;
    }

private:
    static std::string totalType(bool is_array_, ValueType type_)
    {
        if (is_array_)
            return "Arr[" + valueTypeToString(type_) + "]";
        return valueTypeToString(type_);
    }

    bool is_array = false;
    ValueType type;
    std::variant<bool, double, std::string, DateTime, DynamicArray> var;
};
