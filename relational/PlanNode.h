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

private:
    std::unordered_map<IdentifierName, AggOperator> agg_op;
    std::vector<IdentifierName> group_by;
};

class JoinNode : public PlanNode
{
public:
};

// only filter columns, remain rows count unchanged.
class ProjectionNode : public PlanNode
{
public:
};

// only filter rows, remain column species number unchanged.
class FilterNode : public PlanNode
{
public:
    FilterNode(ExpressionPtr filter_) : PlanNode(NodeType::Filter), filter(std::move(filter_)) {}

    Rows transform(const Rows &input) override
    {
        assert(filter);
        return filterPredicate(RowsRef(input.getColumns()), filter).materialize();
    }

    static RowsRef filterPredicate(const RowsRef& input, const ExpressionPtr& flt)
    {
        RowsRef res(input.getColumns());
        if (flt->getSymbolType() == SymbolType::And)
        {
            RowsRef left_res = filterPredicate(input, flt->getChild(0));
            for (size_t i = 1; i < flt->getChildSize(); i++)
            {
                RowsRef right_res = filterPredicate(input, flt->getChild(i));
                left_res = left_res & right_res;
            }
            res = std::move(left_res);
        }
        else if (flt->getSymbolType() == SymbolType::Or)
        {

        }
        else if (flt->getSymbolType() == SymbolType::Not)
        {

        }
        else if (flt->getSymbolType() == SymbolType::InTuple)
        {

        }
        else
        {
            res = filterExpression(RowsRef(input.getColumns()), flt);
        }
        return res;
    }

    static RowsRef filterExpression(const RowsRef &input, ExpressionPtr flt)
    {
        RowsRef res(input.getColumns());

        if (flt->getSymbolType() == SymbolType::True)
        {
            assert(!res.getColumns().empty());
            for (size_t i = 0; i < res.getColumns()[0]->size(); i++)
                res.addIndex(i);
            return res;
        }
        if (flt->getSymbolType() == SymbolType::False)
            return res;

        auto id = get<IdentifierName>(flt->getChild(0)->getSymbol().var);
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
            THROW(Poco::InvalidArgumentException("unexpected filter type in filterExpression"));

        for (size_t i = 0; i < id_column->size(); i++)
        {
            Value va = id_column->operator[](i);
            if (op(va, value))
                res.addIndex(i);
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
        THROW(UnreachableException("ScanNode has no rows input"));
    }

    Rows read()
    {
        TablePtr table = db.findTable(table_name);
        if (!table || table->name() != table_name)
            THROW(Poco::NotFoundException("can't found this table - " + table_name));
        return table->dumpRows();
    }

private:
    Database& db;
    std::string table_name;
};