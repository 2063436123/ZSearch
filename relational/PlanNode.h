#pragma once

#include <utility>

#include "typedefs.h"
#include "relational/Rows.h"
#include "relational/Expression.h"
#include "core/Database.h"

enum class NodeType
{
    Projection,
    Join,
    Agg,
    Filter,
    Sort,
    Scan
};

class PlanNode;
using PlanNodePtr = std::shared_ptr<PlanNode>;
using PlanNodes = std::vector<PlanNodePtr>;

class PlanNode
{
public:
    PlanNode(NodeType type_) : type(type_) {}

    NodeType getNodeType() const
    {
        return type;
    }

    PlanNodePtr getChild(size_t index = 0) const
    {
        assert(children.size() > index);
        return children[index];
    }

    void addChild(PlanNodePtr child)
    {
        children.push_back(std::move(child));
    }

    virtual Rows transform(const Rows& input) = 0;

private:
    NodeType type;
    std::vector<PlanNodePtr> children;
};


class AggNode : public PlanNode
{
public:
};

class JoinNode : public PlanNode
{
public:
};

class ProjectionNode : public PlanNode
{
public:
};

class FilterNode : public PlanNode
{
public:
    FilterNode(ExpressionPtr filter_) : PlanNode(NodeType::Filter), filter(std::move(filter_)) {}

    Rows transform(const Rows &input) override
    {
        assert(filter);
        Rows res;
        res.depict(input);
        if (filter->getSymbolType() == SymbolType::And)
        {

        }
        else if (filter->getSymbolType() == SymbolType::Or)
        {

        }
        else if (filter->getSymbolType() == SymbolType::Not)
        {

        }
        else if (filter->getSymbolType() == SymbolType::InTuple)
        {

        }
        else
        {
            res = filterExpression(input, filter);
        }
        return res;
    }

    Rows filterExpression(const Rows &input, ExpressionPtr flt) const
    {
        Rows res;
        res.depict(input);
        auto id = get<ColumnName>(flt->getChild(0)->getSymbol().var);
        auto value = get<Value>(flt->getChild(1)->getSymbol().var);
        auto id_column = input[id.column_name];

        std::function<bool(const Value&, const Value&)> op;

        if (flt->getSymbolType() == SymbolType::Equal)
            op = [](const Value& lhs, const Value& rhs) {return lhs == rhs;};
        else if (flt->getSymbolType() == SymbolType::NotEqual)
            op = [](const Value& lhs, const Value& rhs) {return lhs != rhs;};
        else if (flt->getSymbolType() == SymbolType::Less)
            op = [](const Value& lhs, const Value& rhs) {return lhs < rhs;};
        else if (flt->getSymbolType() == SymbolType::LessEqual)
            op = [](const Value& lhs, const Value& rhs) {return lhs <= rhs;};
        else if (flt->getSymbolType() == SymbolType::Great)
            op = [](const Value& lhs, const Value& rhs) {return lhs > rhs;};
        else if (flt->getSymbolType() == SymbolType::GreatEqual)
            op = [](const Value& lhs, const Value& rhs) {return lhs >= rhs;};
        else
            throw Poco::InvalidArgumentException("unexpected filter type in filterExpression");

        for (size_t i = 0; i < id_column->size(); i++)
        {
            Value va = id_column->operator[](i);
            if (op(va, value))
                res.operator[](id.column_name)->insert(va);
        }
        return res;
    }
private:
    ExpressionPtr filter;
};

class SortNode : public PlanNode
{
public:
};

class ScanNode : public PlanNode
{
public:
    ScanNode(Database& db_, std::string table_name_) : PlanNode(NodeType::Scan), db(db_), table_name(std::move(table_name_)) {}

    Rows transform(const Rows &input) override
    {
        throw UnreachableException("ScanNode has no rows input");
    }

    Rows read()
    {
        Table table = db.findTable(table_name);
        if (table.name() != table_name)
            throw Poco::NotFoundException("can't found this table - " + table_name);
        return table.dumpRows();
    }

private:
    Database& db;
    std::string table_name;
};