#pragma once

#include <queryparser/Lexer.h>
#include "core/Value.h"

bool compareLess(const Value& v1, const Value& v2)
{
    return v1 < v2;
}

bool compareLessOrEqual(const Value& v1, const Value& v2)
{
    return v1 <= v2;
}

bool compareGreater(const Value& v1, const Value& v2)
{
    return v1 > v2;
}

bool compareGreaterOrEqual(const Value& v1, const Value& v2)
{
    return v1 >= v2;
}

bool compareEqual(const Value& v1, const Value& v2)
{
    return v1 == v2;
}

bool compareNotEqual(const Value& v1, const Value& v2)
{
    return v1 != v2;
}

bool compareIn(const Value& v1, const Value& v2)
{
    if (v1.getValueType() != v2.getValueType())
        THROW(Poco::InvalidArgumentException("Value type isn't compatible in compareIn(): " +
                                                     valueTypeToString(v1.getValueType()) + " vs " +
                                                     valueTypeToString(v2.getValueType())));
    if (v1.isArray() || !v2.isArray())
        THROW(Poco::InvalidArgumentException("we need <single> compareIn <array>"));

    std::function<void(std::vector<Number>*)> numberHandler = PanicValueArrayHandler<Number>;
    std::function<void(std::vector<String>*)> stringHandler = PanicValueArrayHandler<String>;
    std::function<void(std::vector<DateTime>*)> datetimeHandler = PanicValueArrayHandler<DateTime>;

    bool isIn = false;
    switch (v1.getValueType())
    {
        case ValueType::Null:
            return true;
            THROW(Poco::NotImplementedException()); // TODO: 未实现是因为未确定 null in tuple(null) 的意义
        case ValueType::Bool:
            THROW(UnreachableException()); // 未实现是因为 doArrayHandler 不支持 bool
        case ValueType::Number:
            numberHandler = [v1, &isIn](std::vector<Number>* vec) {
                for (Number number : *vec)
                    if (v1.as<Number>() == number)
                    {
                        isIn = true;
                        break;
                    }
            };
        case ValueType::String:
            stringHandler = [v1, &isIn](std::vector<String>* vec) {
                for (String string : *vec)
                    if (v1.as<String>() == string)
                    {
                        isIn = true;
                        break;
                    }
            };
        case ValueType::DateTime:
            datetimeHandler = [v1, &isIn](std::vector<DateTime>* vec) {
                for (DateTime dateTime : *vec)
                    if (v1.as<DateTime>() == dateTime)
                    {
                        isIn = true;
                        break;
                    }
            };
    }

    v2.doArrayHandler(PanicValueArrayHandler<Bool>, numberHandler, stringHandler, datetimeHandler);

    return isIn;
}

// TODO: compareIn/NotIn 的唯一区别在于函数体内的所有 true/false 都是相反的 -> 可抽取公共函数

bool compareNotIn(const Value& v1, const Value& v2)
{
    if (v1.getValueType() != v2.getValueType())
        THROW(Poco::InvalidArgumentException("Value type isn't compatible in compareIn(): " +
                                             valueTypeToString(v1.getValueType()) + " vs " +
                                             valueTypeToString(v2.getValueType())));
    if (v1.isArray() || !v2.isArray())
        THROW(Poco::InvalidArgumentException("we need <single> compareIn <array>"));

    std::function<void(std::vector<Number>*)> numberHandler = PanicValueArrayHandler<Number>;
    std::function<void(std::vector<String>*)> stringHandler = PanicValueArrayHandler<String>;
    std::function<void(std::vector<DateTime>*)> datetimeHandler = PanicValueArrayHandler<DateTime>;

    bool isNotIn = true;
    switch (v1.getValueType())
    {
        case ValueType::Null:
            return false;
            THROW(Poco::NotImplementedException()); // TODO: 未实现是因为未确定 null in tuple(null) 的意义
        case ValueType::Bool:
            THROW(UnreachableException()); // 未实现是因为 doArrayHandler 不支持 bool
        case ValueType::Number:
            numberHandler = [v1, &isNotIn](std::vector<Number>* vec) {
                for (Number number : *vec)
                    if (v1.as<Number>() == number)
                    {
                        isNotIn = false;
                        break;
                    }
            };
        case ValueType::String:
            stringHandler = [v1, &isNotIn](std::vector<String>* vec) {
                for (String string : *vec)
                    if (v1.as<String>() == string)
                    {
                        isNotIn = false;
                        break;
                    }
            };
        case ValueType::DateTime:
            datetimeHandler = [v1, &isNotIn](std::vector<DateTime>* vec) {
                for (DateTime dateTime : *vec)
                    if (v1.as<DateTime>() == dateTime)
                    {
                        isNotIn = false;
                        break;
                    }
            };
    }

    v2.doArrayHandler(PanicValueArrayHandler<Bool>, numberHandler, stringHandler, datetimeHandler);

    return isNotIn;
}
using CompareFunction = std::function<bool(const Value&, const Value&)>;

CompareFunction getCompByType(TokenType comp_type)
{
    switch (comp_type)
    {
        case TokenType::Less:
            return compareLess;
        case TokenType::LessOrEquals:
            return compareLessOrEqual;
        case TokenType::Greater:
            return compareGreater;
        case TokenType::GreaterOrEquals:
            return compareGreaterOrEqual;
        case TokenType::NotEquals:
            return compareNotEqual;
        case TokenType::Equals:
            return compareEqual;
        case TokenType::InRange:
            return compareIn;
        default:
            THROW(UnreachableException("getCompByType -- " + std::to_string(static_cast<int>(comp_type))));
    }
}