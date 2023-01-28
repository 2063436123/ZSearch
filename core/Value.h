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
    Value(ArrayLabel is_array_, ValueType type_) : is_array(true), type(type_), var(DynamicArray(type))
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

    Value(int64_t i) : type(ValueType::Number), var(double(i))
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

    void doArrayHandler(const ValueArrayHandler<Number>& number_handler,
                        const ValueArrayHandler<String>& string_handler,
                        const ValueArrayHandler<DateTime>& datetime_handler) const
    {
        if (!is_array)
            THROW(Poco::LogicException("only array value can call doArrayHandler"));
        get<DynamicArray>(var).applyHandler(number_handler, string_handler, datetime_handler);
    }

    // TODO: 完善 ==, != 成员函数
//    bool operator==(const Value& rhs) const
//    {
//        return var.index() == rhs.var.index();
//    }
//
//    bool operator!=(const Value& rhs) const
//    {
//        return !(*this == rhs);
//    }

private:
    bool is_array = false;
    ValueType type;
    std::variant<bool, double, std::string, DateTime, DynamicArray> var;
};
