#pragma once

#include "../typedefs.h"

class AggOperator
{
public:
    virtual void operator()(const Value& value) = 0;
    virtual Value result() const = 0;
};

class SumOperator : public AggOperator
{
public:
    void operator()(const Value& value) override
    {
        bool is_null = value.getValueType() == ValueType::Null;
        if (is_null)
            return;
        if (!has_call)
        {
            has_call = true;
            sum = value;
        }
        else
        {
            sum = sum + value;
        }
    }

    Value result() const override
    {
        return sum;
    }

private:
    bool has_call = false;
    Value sum;
};

class CountOperator : public AggOperator
{
public:
    void operator()(const Value&) override
    {
        ++cnt;
    }

    Value result() const override
    {
        return (int64_t)cnt;
    }

private:
    size_t cnt = 0;
};

class AvgOperator : public AggOperator
{
public:
    void operator()(const Value& value) override
    {
        sum(value);
        cnt(value);
    }

    Value result() const override
    {
        if (cnt.result().as<Int>() == 0)
            return {};
        return sum.result() / cnt.result();
    }

private:
    SumOperator sum;
    CountOperator cnt;
};

class MaxOperator : public AggOperator
{
public:
    void operator()(const Value& value) override
    {
        bool is_null = value.getValueType() == ValueType::Null;
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

    Value result() const override
    {
        return max;
    }

private:
    bool has_compared = false;
    Value max;
};

class MinOperator : public AggOperator
{
public:
    void operator()(const Value& value) override
    {
        bool is_null = value.getValueType() == ValueType::Null;
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

    Value result() const override
    {
        return min;
    }

private:
    bool has_compared = false;
    Value min;
};