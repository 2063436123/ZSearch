#pragma once
#include "../typedefs.h"
#include "Value.h"

struct ColumnName
{
    std::string table_name;
    std::string column_name;
    std::string getName() const {
        return table_name + '.' + column_name;
    }
};

enum class SymbolType
{
    Id, // equals to ColumnName in general; single
    Value, // single
    // = < > <= >= !=, a Op b
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Great,
    GreatEqual,
    InTuple, // a InTuple b, b like (1, 2, 3)
    And, // a "AND" b
    Or, // a "OR" b
    Not // "NOT" a
};

struct Symbol {
    SymbolType type;
    std::variant<ColumnName, Value> var;

    Symbol(SymbolType type_) : type(type_)
    {
        if (type_ == SymbolType::Id || type_ == SymbolType::Value)
            throw Poco::InvalidArgumentException("non-value need SymbolType::Other...!");
    }

    Symbol(SymbolType type_, const ColumnName& str) : type(type_)
    {
        if (type_ != SymbolType::Id)
            throw Poco::InvalidArgumentException("ColumnName type need SymbolType::Id!");
        var = str;
    }

    Symbol(SymbolType type_, const Value& val) : type(type_)
    {
        if (type != SymbolType::Value)
            throw Poco::InvalidArgumentException("Value type need SymbolType::Value!");
        var = val;
    }
};

class Expression;
using ExpressionPtr = std::shared_ptr<Expression>;
using Expressions = std::vector<ExpressionPtr>;

// express predicates with conjunctive.
class Expression
{
public:
    Expression(Symbol symbol_) : symbol(symbol_) {}

    SymbolType getSymbolType() const
    {
        return symbol.type;
    }

    Symbol getSymbol() const
    {
        return symbol;
    }

    ExpressionPtr getChild(size_t index = 0) const
    {
        assert(children.size() > index);
        return children[index];
    }

    void addChild(ExpressionPtr child)
    {
        children.push_back(std::move(child));
    }

private:
    Symbol symbol;
    Expressions children;
};