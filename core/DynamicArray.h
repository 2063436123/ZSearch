#pragma once

#include "../typedefs.h"
#include "utils/SerializeUtils.h"

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

std::string valueTypeToString(ValueType type)
{
    switch (type)
    {
        case ValueType::Null:
            return "Null";
        case ValueType::Bool:
            return "Bool";
        case ValueType::Number:
            return "Number";
        case ValueType::String:
            return "String";
        case ValueType::DateTime:
            return "DateTime";
    }
    THROW(UnreachableException());
}

template<typename T>
using ValueArrayHandler = std::function<void(std::vector<T>* /* data */)>;

template<typename T>
auto EmptyValueArrayHandler = [](std::vector<T>*) -> void {};

template<typename T>
auto PanicValueArrayHandler = [](std::vector<T>*) -> void { THROW(UnreachableException(std::string("in PanicValueArrayHandler(), T is ") + typeid(T).name())); };

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

    ValueType getType() const
    {
        return type;
    }

    void* getData() {
        return data;
    }

    DynamicArray(const DynamicArray& rhs)
    {
        type = rhs.type;
        safe_copy(rhs.data);
    }

    DynamicArray(DynamicArray&& rhs) noexcept
    {
        type = rhs.type;
        data = rhs.data;
        rhs.data = nullptr;
    }

    DynamicArray& operator=(const DynamicArray& rhs)
    {
        if (this == &rhs)
            return *this;
        safe_delete();
        type = rhs.type;
        safe_copy(rhs.data);
        return *this;
    }

    DynamicArray& operator=(DynamicArray&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;
        safe_delete();
        type = rhs.type;
        data = rhs.data;
        rhs.data = nullptr;
        return *this;
    }

    bool operator<=>(const DynamicArray& rhs) const
    {
        THROW(UnreachableException()); // 作为 Value 的数组不该被比较大小
    }

    // select and apply correct handler
    void applyHandler(const ValueArrayHandler<Bool>& bool_handler,
                      const ValueArrayHandler<Number>& number_handler,
                      const ValueArrayHandler<String>& string_handler,
                      const ValueArrayHandler<DateTime>& datetime_handler) const
    {
        switch (type)
        {
            case ValueType::Null:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Bool:
                bool_handler((std::vector<Bool>*)data);
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
        std::vector<T>* array_ref = getRealArrayRef<T>();
        if (array_ref->size() <= i)
            THROW(Poco::RangeException("get i >= array size " + std::to_string(i) + " vs " + std::to_string(array_ref->size())));
        return array_ref->operator[](i);
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

    void serialize(WriteBufferHelper& helper) const
    {
        helper.writeNumber<ValueType>(type);
        switch (type)
        {
            case ValueType::Null:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Bool:
                helper.writeLinearContainer(*(std::vector<Bool>*)data);
                break;
            case ValueType::Number:
                helper.writeLinearContainer(*(std::vector<Number>*)data);
                break;
            case ValueType::String:
                helper.writeLinearContainer(*(std::vector<String>*)data);
                break;
            case ValueType::DateTime:
                helper.writeLinearContainer(*(std::vector<DateTime>*)data);
                break;
        }
    }

    static DynamicArray deserialize(ReadBufferHelper& helper)
    {
        ValueType type = helper.readNumber<ValueType>();
        DynamicArray ret(type);
        void* data = ret.data; // TODO: ? private
        switch (type)
        {
            case ValueType::Null:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Bool:
                *(std::vector<Bool>*)data = helper.readLinearContainer<std::vector, Bool>();
                break;
            case ValueType::Number:
                *(std::vector<Number>*)data = helper.readLinearContainer<std::vector, Number>();
                break;
            case ValueType::String:
                *(std::vector<String>*)data = helper.readLinearContainer<std::vector, String>();
                break;
            case ValueType::DateTime:
                *(std::vector<DateTime>*)data = helper.readLinearContainer<std::vector, DateTime>();
                break;
        }
        return ret;
    }

    bool operator==(const DynamicArray& rhs) const
    {
        if (type != rhs.type)
            return false;
        switch (type)
        {
            case ValueType::Null:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Bool:
                return *(std::vector<Bool>*)data == *(std::vector<Bool>*)rhs.data;
                break;
            case ValueType::Number:
                return *(std::vector<Number>*)data == *(std::vector<Number>*)rhs.data;
                break;
            case ValueType::String:
                return *(std::vector<String>*)data == *(std::vector<String>*)rhs.data;
                break;
            case ValueType::DateTime:
                return *(std::vector<DateTime>*)data == *(std::vector<DateTime>*)rhs.data;
                break;
        }
        return false;
    }

    bool operator!=(const DynamicArray& rhs) const
    {
        return !(*this == rhs);
    }

private:
    void safe_copy(void *rhs_data)
    {
        safe_new();
        switch (type)
        {
            case ValueType::Null:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Bool:
                *(std::vector<Bool>*)data = *(std::vector<Bool>*)rhs_data;
                break;
            case ValueType::Number:
                *(std::vector<Number>*)data = *(std::vector<Number>*)rhs_data;
                break;
            case ValueType::String:
                *(std::vector<String>*)data = *(std::vector<String>*)rhs_data;
                break;
            case ValueType::DateTime:
                *(std::vector<DateTime>*)data = *(std::vector<DateTime>*)rhs_data;
                break;
        }
    }

    void safe_new()
    {
        switch (type)
        {
            case ValueType::Null:
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Bool:
                data = new std::vector<Bool>;
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
                THROW(Poco::NotImplementedException());
                break;
            case ValueType::Bool:
                delete (std::vector<Bool>*)data;
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
    void* data = nullptr;
};