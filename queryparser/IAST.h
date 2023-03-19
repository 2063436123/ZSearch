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
