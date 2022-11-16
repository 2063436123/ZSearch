#pragma once

#include "../typedefs.h"

template<typename T>
class SumOperator
{
public:
    void operator()(T value, bool is_null)
    {
        if (is_null)
            return;
        sum = sum + value;
    }

    T result()
    {
        return sum;
    }

private:
    T sum{};
};

template<typename T>
class CountOperator
{
public:
    void operator()(T, bool is_null)
    {
        ++cnt;
    }

    size_t result()
    {
        return cnt;
    }

private:
    size_t cnt = 0;
};

template<typename T>
class AvgOperator
{
public:
    void operator()(T value, bool is_null)
    {
        sum(value, is_null);
        cnt(value, is_null);
    }

    std::optional<T> result()
    {
        if (cnt.result() == 0)
            return std::nullopt;
        return std::make_optional(sum.result() / cnt.result());
    }

private:
    SumOperator<T> sum;
    CountOperator<T> cnt;
};

template<typename T>
class MaxOperator
{
public:
    void operator()(T value, bool is_null)
    {
        if (is_null)
            return;
        if (!has_compared)
        {
            has_compared = true;
            max = value;
        }
        else
        {
            max = std::max(max, value);
        }
    }

    std::optional<T> result()
    {
        return has_compared ? std::make_optional(max) : std::nullopt;
    }

private:
    bool has_compared = false;
    T max;
};

template<typename T>
class MinOperator
{
public:
    void operator()(T value, bool is_null)
    {
        if (is_null)
            return;
        if (!has_compared)
        {
            has_compared = true;
            min = value;
        }
        else
        {
            min = std::min(min, value);
        }
    }

    std::optional<T> result()
    {
        return has_compared ? std::make_optional(min) : std::nullopt;
    }

private:
    bool has_compared = false;
    T min;
};