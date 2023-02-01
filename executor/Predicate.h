#pragma once

#include "../typedefs.h"
#include "AggregateFunction.h"
#include "CompareFunction.h"

/*
aggExpr : Agg_op '(' ID ')' | ID;
value : INT | STRING;
valueList : '(' (value ',')* value ')';
where : aggExpr CmpOp valueList;
*/
using AggregateFunction = std::function<Value(const Value&)>;
using CompareFunction = std::function<bool(const Value&, const Value&)>;

class Predicate {
public:
    Predicate(const AggregateFunction& agg_, const String& id_, const CompareFunction& compare_, const Value& value_)
        : agg(agg_), id(id_), compare(compare_), value(value_) {}

    bool determine(const std::unordered_map<Key, Value>& kvs) const
    {
        auto iter = kvs.find(id);
        if (iter == kvs.end())
            return false;
        if (iter->second.isArray() && agg == nullptr)
            return false;
        if (!iter->second.isArray() && agg != nullptr)
            return false;
        Value v;
        if (agg)
            v = agg(iter->second);
        else
            v = iter->second;
        return compare(v, value);
    }

private:
    AggregateFunction agg;
    String id;
    CompareFunction compare;
    Value value;
};