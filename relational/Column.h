#pragma once

#include "../typedefs.h"
#include "ColumnTypeAndData.h"
#include "utils/SerializerUtils.h"

class ColumnBase
{
public:
    ColumnBase(ColumnType type_, const std::string &name) : type(type_), column_name(name) {}

    ColumnType type;
    std::string column_name;

    virtual std::shared_ptr<ColumnBase> copy() const = 0;

    virtual size_t size() const = 0;

    // append all content of column_ptr into current column::data.
    virtual void appendBlock(std::shared_ptr<ColumnBase> column_ptr) = 0;

    virtual void setOrdered() = 0;

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeNumber(static_cast<int>(type));
        serializeImpl(helper);
    }
    virtual void serializeImpl(WriteBufferHelper &helper) const = 0;
    static std::shared_ptr<ColumnBase> deserialize(ReadBufferHelper &helper);
};
using ColumnPtr = std::shared_ptr<ColumnBase>;

#define COLUMN_IMPL(ClassType) \
class Column##ClassType : public ColumnBase \
{ \
public: \
    Column##ClassType(const std::string &name, ColumnDataPtr<ClassType> data_ = nullptr) : ColumnBase(ColumnType::ClassType, name), \
                                                                                   data(data_) {                                    \
        if (!data)   \
            data = std::make_shared<ColumnDataBase<ClassType>>();\
                                                                                   } \
    std::shared_ptr<ColumnBase> copy() const override {                                                                             \
        return std::make_shared<Column##ClassType>(column_name, data->copy()); \
    }                                     \
    \
    size_t size() const override \
    { \
        return data->size(); \
    }                                     \
    \
    void appendBlock(std::shared_ptr<ColumnBase> column_ptr_) override \
    { \
        auto column_ptr = std::dynamic_pointer_cast<Column##ClassType>(column_ptr_); \
        if (!column_ptr) \
            throw Poco::BadCastException("can't convert ColumnBase to Column" #ClassType); \
        data->appendBlock(column_ptr->data); \
    } \
    \
    void setOrdered() override \
    {                          \
        data->setOrdered(); \
    }                          \
                               \
    void serializeImpl(WriteBufferHelper &helper) const override \
    {                          \
        helper.writeString(column_name); \
        data->serialize(helper); \
    } \
    \
    static std::shared_ptr<Column##ClassType> deserialize(ReadBufferHelper &helper) \
    {                              \
        auto column_name = helper.readString(); \
        auto data = ColumnDataBase<ClassType>::deserialize(helper);                                                                 \
        return std::make_shared<Column##ClassType>(column_name, data); \
    } \
    \
    void handleEverything(std::function<void(ClassType, bool)> handler) const       \
    {                          \
        for (size_t i = 0; i < data->size(); i++)                                       \
            handler((*data)[i], data->is_null(i));\
    }\
    \
    ColumnDataPtr<ClassType> data; \
};

COLUMN_IMPL(Int)
COLUMN_IMPL(String)
COLUMN_IMPL(Decimal)
COLUMN_IMPL(DateTime)
COLUMN_IMPL(Blob)


std::shared_ptr<ColumnBase> ColumnBase::deserialize(ReadBufferHelper &helper)
{
    auto type = static_cast<ColumnType>(helper.readNumber<int>());
    std::shared_ptr<ColumnBase> ret;
    if (type == ColumnType::Int)
        ret = ColumnInt::deserialize(helper);
    if (type == ColumnType::String)
        ret = ColumnString::deserialize(helper);
    if (type == ColumnType::Decimal)
        ret = ColumnDecimal::deserialize(helper);
    if (type == ColumnType::DateTime)
        ret = ColumnDateTime::deserialize(helper);
    if (type == ColumnType::Blob)
        ret = ColumnBlob::deserialize(helper);
    return ret;
}