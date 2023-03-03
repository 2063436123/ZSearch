#pragma once

#include "../typedefs.h"
#include "TimeUtils.h"

class BufferBase
{
public:
    std::pair<const char *, size_t> string_ref() const
    {
        return {buf.c_str(), buf.size()};
    }

    void clear()
    {
        buf.clear();
    }

protected:
    std::string buf;
};

class WriteBuffer : public BufferBase
{
public:
    void append(const char *src, size_t len)
    {
        buf.append(src, src + len);
    }

    void dumpAllToStream(std::ostream &fout)
    {
        fout << buf;
        buf.clear();
    }

};

class ReadBuffer : public BufferBase
{
public:
    const char *consume(size_t consume_bytes)
    {
        if (buf.size() - next >= consume_bytes)
        {
            const char *next_pointer = buf.c_str() + next;
            next += consume_bytes;
            return next_pointer;
        }
        else
            THROW(Poco::RangeException());
    }

    void append(const char *src, size_t len)
    {
        buf.append(src, len);
    }

    void readAllFromStream(std::istream &fin)
    {
        buf = std::string(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
        next = 0;
    }

private:
    size_t next = 0;
};

class WriteBufferHelper
{
public:
    explicit WriteBufferHelper(WriteBuffer &buffer) : buf(buffer) {}

    template<typename T>
    void writeNumber(T number)
    {
        buf.append(reinterpret_cast<const char *>(&number), sizeof(T));
    }

    void writeString(const std::string &str)
    {
        writeNumber(str.size());
        buf.append(str.c_str(), str.size());
    }

    void writeDateTime(const DateTime &date_time)
    {
        writeString(date_time.string());
    }

    template<template<typename, typename> class C, typename T>
    void writeLinearContainer(const C<T, std::allocator<T>> &container)
    {
        writeNumber(container.size());
        for (auto ele: container)
        {
            if constexpr (std::is_same_v<T, bool>)
                writeNumber(char(ele));
            else if constexpr (std::is_arithmetic_v<T>)
                writeNumber(ele);
            else if constexpr (std::is_same_v<T, std::string>)
                writeString(ele);
            else if constexpr (std::is_same_v<T, DateTime>)
                writeDateTime(ele);
            else
                THROW(UnreachableException("in writeLinearContainer"));
        }
    }

    template<template<typename, typename, typename> class C, typename T>
    void writeSetContainer(const C<T, std::less<T>, std::allocator<T>> &container)
    {
        writeNumber(container.size());
        for (auto ele: container)
        {
            if constexpr (std::is_arithmetic_v<T>)
                writeNumber(ele);
            else if constexpr (std::is_same_v<T, std::string>)
                writeString(ele);
            else if constexpr (std::is_same_v<T, DateTime>)
                writeDateTime(ele);
            else
                THROW(UnreachableException("in writeSetContainer"));
        }
    }

private:
    WriteBuffer &buf;
};

class ReadBufferHelper
{
public:
    explicit ReadBufferHelper(ReadBuffer &buffer) : buf(buffer) {}

    template<typename T>
    T readNumber()
    {
        const char *str = buf.consume(sizeof(T));
        T number = *(T *) (str);
        return number;
    }

    std::string readString()
    {
        auto size = readNumber<size_t>();
        const char *str = buf.consume(size);
        return {str, size};
    }

    DateTime readDateTime()
    {
        return DateTime(readString());
    }

    template<template<typename, typename> class T, typename N>
    auto readLinearContainer()
    {
        T<N, std::allocator<N>> container;
        auto size = readNumber<size_t>();

        for (size_t i = 0; i < size; i++)
        {
            if constexpr (std::is_arithmetic_v<N>)
            {
                auto number = readNumber<N>();
                container.push_back(number);
            }
            else if constexpr (std::is_same_v<N, std::string>)
            {
                auto string = readString();
                container.push_back(string);
            }
            else if constexpr (std::is_same_v<N, DateTime>)
            {
                auto date_time = readDateTime();
                container.push_back(date_time);
            }
            else
                THROW(UnreachableException("in readLinearContainer"));
        }
        return container;
    }

    template<template<typename, typename, typename> class C, typename T>
    auto readSetContainer()
    {
        C<T, std::less<T>, std::allocator<T>> container;
        auto size = readNumber<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            if constexpr (std::is_arithmetic_v<T>)
            {
                auto number = readNumber<T>();
                container.insert(number);
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                auto string = readString();
                container.insert(string);
            }
            else if constexpr (std::is_same_v<T, DateTime>)
            {
                auto date_time = readDateTime();
                container.insert(date_time);
            }
            else
                THROW(UnreachableException("in readSetContainer"));
        }
        return container;
    }

private:
    ReadBuffer &buf;
};
