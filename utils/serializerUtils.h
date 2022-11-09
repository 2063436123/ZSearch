#include "../typedefs.h"

class BufferBase
{
public:
    std::pair<const char*, size_t> string_ref() const
    {
        return {buf.c_str(), buf.size()};
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

private:
};

class ReadBuffer : public BufferBase
{
public:
    const char *consume(size_t consume_bytes)
    {
        if (buf.size() - next >= consume_bytes)
        {
            const char * next_pointer = buf.c_str() + next;
            next += consume_bytes;
            return next_pointer;
        }
        else
            throw Poco::RangeException();

    }

    void append(const char *src, size_t len)
    {
        buf.append(src, len);
    }

    void readAllFromStream(std::istream &fin)
    {
        buf = std::string(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
    }

private:
    size_t next = 0;
};

class WriteBufferHelper
{
public:
    explicit WriteBufferHelper(WriteBuffer &buffer) : buf(buffer) {}

    template<typename T>
    void writeInteger(T number)
    {
        buf.append(reinterpret_cast<const char *>(&number), sizeof(T));
    }

    void writeString(const std::string &str)
    {
        writeInteger(str.size());
        buf.append(str.c_str(), str.size());
    }

    template<template<typename, typename> class C, typename T>
    void writeLinearContainer(const C<T, std::allocator<T>>& container)
    {
        writeInteger(container.size());
        if constexpr (std::is_integral_v<T>)
        {
            for (auto number: container)
                writeInteger(number);
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            for (const auto& str : container)
                writeString(str);
        }
        else
            assert(0);
    }

    template<template<typename, typename, typename> class C, typename T>
    void writeSetContainer(const C<T, std::less<T>, std::allocator<T>>& container)
    {
        writeInteger(container.size());
        if constexpr (std::is_integral_v<T>)
        {
            for (auto number: container)
                writeInteger(number);
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            for (const auto& str : container)
                writeString(str);
        }
        else
            assert(0);
    }

private:
    WriteBuffer &buf;
};

class ReadBufferHelper
{
public:
    explicit ReadBufferHelper(ReadBuffer &buffer) : buf(buffer) {}

    template<typename T>
    T readInteger()
    {
        const char *str = buf.consume(sizeof(T));
        T number = *(T *) (str);
        return number;
    }

    std::string readString()
    {
        auto size = readInteger<size_t>();
        const char *str = buf.consume(size);
        return {str, size};
    }

    template<template<typename, typename> class T, typename N>
    auto readLinearContainer()
    {
        T<N, std::allocator<N>> container;
        auto size = readInteger<size_t>();

        if constexpr (std::is_integral_v<N>)
        {
            for (size_t i = 0; i < size; i++)
            {
                auto number = readInteger<N>();
                container.push_back(number);
            }
        }
        else if constexpr (std::is_same_v<N, std::string>)
        {
            for (size_t i = 0; i < size; i++)
            {
                auto string = readString();
                container.push_back(string);
            }
        }
        else
            assert(0);
        return container;
    }

    template<template<typename, typename, typename> class C, typename T>
    auto readSetContainer()
    {
        C<T, std::less<T>, std::allocator<T>> container;
        auto size = readInteger<size_t>();
        if constexpr (std::is_integral_v<T>)
        {
            for (size_t i = 0; i < size; i++)
            {
                auto number = readInteger<T>();
                container.insert(number);
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            for (size_t i = 0; i < size; i++)
            {
                auto string = readString();
                container.insert(string);
            }
        }
        else
            assert(0);
        return container;
    }
private:
    ReadBuffer &buf;
};
