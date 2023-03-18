#pragma once

#include "core/Value.h"
#include "executor/Executor.h"
#include "executor/TermsExecutor.h"
#include "executor/HavingExecutor.h"
#include "executor/LimitExecutor.h"

class IAST;

using ASTPtr = std::shared_ptr<IAST>;
using ASTs = std::vector<ASTPtr>;

class IAST : std::enable_shared_from_this<IAST>
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

template<typename T>
concept ConstructorOnlyNeedDatabase = requires(T a) {
    T{a}; // T must have a single argument constructor
};

template <ConstructorOnlyNeedDatabase Executor>
ExecutorPtr toExecutorHelper(Database& db, ASTPtr ast) {
    if (!ast)
        return std::make_shared<Executor>(db);
    return ast->toExecutor(db);
}

class ASTQuery : public IAST
{
public:
    ASTQuery(const ASTPtr& word_list_, const ASTPtr& having_expression_, const ASTPtr& limit_length_)
        : word_list(word_list_), having_expression(having_expression_), limit_length(limit_length_) { }

    ExecutorPtr toExecutor(Database &) const override
    {
        THROW(UnreachableException("ASTQuery::toExecutor"));
    }

    ExecutePipeline toExecutorPipeline(Database & db) const
    {
        ExecutePipeline pipeline;

        pipeline.addExecutor(toExecutorHelper<TermsExecutor>(db, word_list))
            .addExecutor(toExecutorHelper<HavingExecutor>(db, having_expression))
            .addExecutor(toExecutorHelper<LimitExecutor>(db, limit_length));

        return pipeline;
    }

private:
    ASTPtr word_list;
    ASTPtr having_expression;
    ASTPtr limit_length;
};

class ASTWord : public IAST
{
public:
    ASTWord(const std::string &word_) : word(word_) {}

    ExecutorPtr toExecutor(Database &db) const override
    {
        // TODO: to TermsExecutor
        THROW(Poco::NotImplementedException("ASTWord::toExeccutor"));
    }

private:
    std::string word;
};

class ASTLimit : public IAST
{
public:
    ASTLimit(double v_) : v(v_) {}

    ExecutorPtr toExecutor(Database &db) const override
    {
        THROW(UnreachableException("ASTLiteral::toExecutor"));
    }

private:
    double v;
};
