#pragma once
#include "../typedefs.h"
#include "Value.h"


enum class SymbolType
{
    True,
    False,
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
    std::variant<IdentifierName, Value> var;

    Symbol(SymbolType type_) : type(type_)
    {
        if (type_ == SymbolType::Id || type_ == SymbolType::Value)
            throw Poco::InvalidArgumentException("non-value need SymbolType::Other...!");
    }

    Symbol(SymbolType type_, const IdentifierName& str) : type(type_)
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

    size_t getChildSize() const
    {
        return children.size();
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