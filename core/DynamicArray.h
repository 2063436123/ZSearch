#pragma once

#include "../typedefs.h"

using Bool = bool;
using Number = double;
using String = std::string;
using DateTime = DateTime;

enum class ValueType
{
    Null,
    Bool,
    Number,
    String,
    DateTime
};
//    ValueList // tuple like (1, 2, 3), nesting is not supported.

template<typename T>
using ValueArrayHandler = std::function<void(std::vector<T>* /* data */)>;

template<typename T>
auto EmptyValueArrayHandler = [](std::vector<T>*) -> void {};

template<typename T>
auto PanicValueArrayHandler = [](std::vector<T>*) -> void { THROW(UnreachableException()); };

template<typename T>
bool checkType(ValueType type)
{
    if (std::is_same_v<T, Bool> && type == ValueType::Bool)
        return true;
    if (std::is_same_v<T, Number> && type == ValueType::Number)
        return true;
    if (std::is_same_v<T, String> && type == ValueType::String)
        return true;
    if (std::is_same_v<T, DateTime> && type == ValueType::DateTime)
        return true;
    return false;
}

class DynamicArray
{
public:
    DynamicArray(ValueType type_) : type(type_) {
        safe_new();
    }

    DynamicArray(const DynamicArray& rhs)
    {
        type = rhs.type;
        safe_new();
        switch (type)
        {
            case ValueType::Null:
            case ValueType::Bool:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Number:
                *(std::vector<Number>*)data = *(std::vector<Number>*)rhs.data;
                break;
            case ValueType::String:
                *(std::vector<String>*)data = *(std::vector<String>*)rhs.data;
                break;
            case ValueType::DateTime:
                *(std::vector<DateTime>*)data = *(std::vector<DateTime>*)rhs.data;
                break;
        }
    }

    DynamicArray(DynamicArray&& rhs)
    {
        type = rhs.type;
        data = rhs.data;
        rhs.data = nullptr;
    }

    DynamicArray& operator=(const DynamicArray&) = delete;
    DynamicArray& operator=(DynamicArray&&) = delete;

    // select and apply correct handler
    void applyHandler(const ValueArrayHandler<Number>& number_handler,
                      const ValueArrayHandler<String>& string_handler,
                      const ValueArrayHandler<DateTime>& datetime_handler) const
    {
        switch (type)
        {
            case ValueType::Null:
            case ValueType::Bool:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Number:
                number_handler((std::vector<Number>*)data);
                break;
            case ValueType::String:
                string_handler((std::vector<String>*)data);
                break;
            case ValueType::DateTime:
                datetime_handler((std::vector<DateTime>*)data);
                break;
        }
    }

    template<typename T>
    std::vector<T>* getRealArrayRef() const
    {
        if (!checkType<T>(type))
            THROW(Poco::LogicException("DynamicArray::getRealArrayRef type error"));
        return (std::vector<T>*)data;
    }

    template<typename T>
    T get(size_t i) const
    {
        return getRealArrayRef<T>()->operator[](i);
    }

    template<typename T>
    std::vector<T> getRealArray() const
    {
        return *getRealArrayRef<T>();
    }

    ~DynamicArray()
    {
        safe_delete();
    }

private:
    void safe_new()
    {
        switch (type)
        {
            case ValueType::Null:
            case ValueType::Bool:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Number:
                data = new std::vector<Number>;
                break;
            case ValueType::String:
                data = new std::vector<String>;
                break;
            case ValueType::DateTime:
                data = new std::vector<DateTime>;
                break;
        }
    }

    void safe_delete()
    {
        switch (type)
        {
            case ValueType::Null:
            case ValueType::Bool:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Number:
                delete (std::vector<Number>*)data;
                break;
            case ValueType::String:
                delete (std::vector<String>*)data;
                break;
            case ValueType::DateTime:
                delete (std::vector<DateTime>*)data;
                break;
        }
    }

    ValueType type;
    void* data;
};