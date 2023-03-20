#pragma once

#include "core/Value.h"
#include "executor/Executor.h"
#include "executor/TermsExecutor.h"
#include "executor/HavingExecutor.h"
#include "executor/LimitExecutor.h"

class IAST;

using ASTPtr = std::shared_ptr<IAST>;
using ASTs = std::vector<ASTPtr>;

class IAST : public std::enable_shared_from_this<IAST>
{
public:
    template<typename To>
    std::shared_ptr<To> as()
    {
        return std::dynamic_pointer_cast<To>(shared_from_this());
    }

    virtual ExecutorPtr toExecutor(Database &) const = 0;

    void addChild(const ASTPtr &child)
    {
        children.push_back(child);
    }

    const ASTs &getChildren() const
    {
        return children;
    }

private:
    ASTs children;
};

class ASTWord : public IAST
{
public:
    ASTWord(const std::string &word_) : word(word_) {}

    std::string getWord() const
    {
        return word;
    }

    ExecutorPtr toExecutor(Database &db) const override
    {
        return std::make_shared<TermsExecutor>(db, ConjunctionTree(new LeafNode<std::string>(word), true));
    }

private:
    std::string word;
};

class ASTHaving : public IAST
{
public:
    ASTHaving(std::string func_name_, std::string column_name_, TokenType compare_type_, Value compare_value_)
        : func_name(std::move(func_name_)), column_name(std::move(column_name_)), compare_type(compare_type_), compare_value(std::move(compare_value_)) {}

    ExecutorPtr toExecutor(Database &db) const override
    {
        return std::make_shared<HavingExecutor>(db, ConjunctionTree(
                    new LeafNode<Predicate>(Predicate(getAggByName(func_name), column_name, getCompByType(compare_type), compare_value)
                ), true));
    }

private:
    std::string func_name;
    std::string column_name; // can be empty —— AUTHOR(), MTIME(), EXISTS(), VALUE()
    TokenType compare_type;
    Value compare_value; // single value or multi values like (1, 2, 3)
};

class ASTLimit : public IAST
{
public:
    ASTLimit(int limit_number_) : limit_number(limit_number_) {}

    ExecutorPtr toExecutor(Database &db) const override
    {
        return std::make_shared<LimitExecutor>(db, limit_number);
    }

private:
    int limit_number;
};
