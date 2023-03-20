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

class Predicate {
public:
    Predicate(const AggregateFunction& agg_, const String& id_, const CompareFunction& compare_, const Value& value_)
        : agg(agg_), id(id_), compare(compare_), value(value_) {}

    bool determine(const std::unordered_map<Key, Value>& kvs) const
    {
        // 无参聚合函数
        if (id.empty())
            return compare(agg(Value{}), value);

        // 单参聚合函数
        auto iter = kvs.find(id);
        if (iter == kvs.end())
            return false;
        if (iter->second.isArray() && agg == nullptr)
            return false;
        assert(agg);
        Value v = agg(iter->second);
        return compare(v, value);
    }

private:
    AggregateFunction agg;
    String id;
    CompareFunction compare;
    Value value;
};
