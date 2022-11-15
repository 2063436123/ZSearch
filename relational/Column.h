#include "../typedefs.h"
#include "utils/TimeUtils.h"

enum class ColumnType
{
    Int,
    Decimal,
    String,
//    UUID,
//    Date,
    DateTime,
    Blob
};

// column type storage
template<typename T>
class ColumnDataBase
{
public:
    std::string getName() const
    {
        return getNameByType<T>();
    }

    size_t size() const
    {
        return data.size();
    }

    auto begin() const
    {
        return const_cast<const std::vector<T> &>(data).begin();
    }

    auto end() const
    {
        return const_cast<const std::vector<T> &>(data).end();
    }

    T operator[](size_t index) const
    {
        if (index >= data.size())
            throw Poco::RangeException("ColumnDataBase operator[] out of range");
        return data[index];
    }

    T &operator[](size_t index)
    {
        if (index >= data.size())
            throw Poco::RangeException("ColumnDataBase operator[] out of range");
        return data[index];
    }

    void insert(const T &value)
    {
        if (!is_ordered)
        {
            data.push_back(value);
        }
        else
        {
            auto iter = std::lower_bound(data.begin(), data.end(), value);
            // allow duplicate values.
            // if (iter == end() || *iter != value)
            data.insert(iter, value);
        }
    }

    void appendBlock(std::shared_ptr<ColumnDataBase> rhs)
    {
        auto rhs_data = std::dynamic_pointer_cast<ColumnDataBase<T>>(rhs);
        if (!rhs_data)
            throw Poco::InvalidArgumentException(
                    "ColumnDatabase::appendBlock, can't convert ColumnDataPtr to ColumnDataBasePtr");
        std::copy(rhs_data->begin(), rhs_data->end(), std::back_inserter(data));
    }

    void setOrdered()
    {
        is_ordered = true;
    }

private:
    template<typename U>
    std::string getNameByType() const
    {
        if constexpr (std::is_same_v<U, int64_t>)
            return "Int";
        if constexpr (std::is_same_v<U, double>)
            return "Decimal";
        if constexpr (std::is_same_v<U, std::string>)
            return "String";
        if constexpr (std::is_same_v<U, DateTime>)
            return "DateTime";
        if constexpr (std::is_same_v<U, char>)
            return "Blob";
        throw UnreachableException("in getNameByType");
    }

    std::vector<T> data;
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
    if constexpr (std::is_same_v<T, char>)
        return ColumnType::Blob;
    throw UnreachableException("in getType");
}

using Int = int64_t;
using Decimal = double;
using String = std::string;
using DateTime = DateTime;
using Blob = char;

using ColumnDataInt = ColumnDataBase<Int>;
using ColumnDataDecimal = ColumnDataBase<Decimal>;
using ColumnDataString = ColumnDataBase<String>;
using ColumnDataDateTime = ColumnDataBase<DateTime>;
using ColumnDataBlob = ColumnDataBase<Blob>;

class ColumnBase
{
public:
    ColumnBase(ColumnType type_, const std::string &name) : type(type_), column_name(name) {}

    ColumnType type;
    std::string column_name;
    bool is_ordered = false;


    virtual size_t size() const = 0;

    // append all content of column_ptr into current column::data.
    virtual void appendBlock(std::shared_ptr<ColumnBase> column_ptr) = 0;

    virtual void setOrdered() = 0;
};

class ColumnString : public ColumnBase
{
public:
    ColumnString(const std::string &name, ColumnDataPtr<String> data_ = nullptr) : ColumnBase(ColumnType::String, name),
                                                                                   data(data_) {}

    size_t size() const override
    {
        return data ? data->size() : 0;
    }

    void appendBlock(std::shared_ptr<ColumnBase> column_ptr) override
    {
        auto string_column_ptr = std::dynamic_pointer_cast<ColumnString>(column_ptr);
        if (!string_column_ptr)
            throw Poco::BadCastException("can't convert ColumnBase to ColumnString");

        if (!data)
            data = std::make_shared<ColumnDataString>();
        data->appendBlock(string_column_ptr->data);
    }

    void setOrdered() override
    {
        data->setOrdered();
    }

    ColumnDataPtr<String> data;
};

using ColumnPtr = std::shared_ptr<ColumnBase>;

class ColumnDecimal : public ColumnBase
{
public:
    ColumnDecimal(const std::string &name, ColumnDataPtr<Decimal> data_ = nullptr) : ColumnBase(ColumnType::Decimal,
                                                                                                name), data(data_) {}

    size_t size() const override
    {
        return data ? data->size() : 0;
    }

    void appendBlock(std::shared_ptr<ColumnBase> column_ptr) override
    {
        auto decimal_column_ptr = std::dynamic_pointer_cast<ColumnDecimal>(column_ptr);
        if (!decimal_column_ptr)
            throw Poco::BadCastException("can't convert ColumnBase to ColumnDecimal");
        if (!data)
            data = std::make_shared<ColumnDataDecimal>();
        data->appendBlock(decimal_column_ptr->data);
    }

    void setOrdered() override
    {
        data->setOrdered();
    }

    ColumnDataPtr<Decimal> data;
};

class ColumnDateTime : public ColumnBase
{
public:
    ColumnDateTime(const std::string &name, ColumnDataPtr<DateTime> data_ = nullptr) : ColumnBase(ColumnType::DateTime,
                                                                                                  name), data(data_) {}

    size_t size() const override
    {
        return data ? data->size() : 0;
    }

    void appendBlock(std::shared_ptr<ColumnBase> column_ptr) override
    {
        auto datetime_column_ptr = std::dynamic_pointer_cast<ColumnDateTime>(column_ptr);
        if (!datetime_column_ptr)
            throw Poco::BadCastException("can't convert ColumnBase to ColumnDateTime");
        if (!data)
            data = std::make_shared<ColumnDataDateTime>();
        data->appendBlock(datetime_column_ptr->data);
    }

    void setOrdered() override
    {
        data->setOrdered();
    }

    ColumnDataPtr<DateTime> data;
};
