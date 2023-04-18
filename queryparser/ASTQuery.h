#pragma once

#include "executor/ScoreExecutor.h"

template<typename T>
concept ConstructorOnlyNeedDatabase = requires(T a) {
    T{a}; // T must have a single argument constructor which will be used for constructing by database
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

    // for displayed results detail generation.
    std::optional<std::string> getTerm() const
    {
        if (!word_list)
            return std::nullopt;
        return word_list->as<ASTWord>()->getWord();
    }

    ExecutePipeline toExecutorPipeline(Database & db) const
    {
        ExecutePipeline pipeline;

        std::unordered_map<std::string, double> word_freq;
        if (word_list)
            word_freq.emplace(word_list->as<ASTWord>()->getWord(), 1.0);

        pipeline.addExecutor(toExecutorHelper<TermsExecutor>(db, word_list))
                .addExecutor(toExecutorHelper<HavingExecutor>(db, having_expression))
                .addExecutor(std::make_shared<ScoreExecutor>(db, word_freq))
                .addExecutor(toExecutorHelper<LimitExecutor>(db, limit_length));

        return pipeline;
    }

private:
    ASTPtr word_list;
    ASTPtr having_expression;
    ASTPtr limit_length;
};
