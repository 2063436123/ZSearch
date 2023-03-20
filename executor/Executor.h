#pragma once
#include "../typedefs.h"
#include "core/Database.h"

using DocIds = std::unordered_set<size_t>;

class Executor
{
public:
    Executor(Database& db_) : db(db_) {}

    virtual std::any execute(const std::any &) const = 0;

    virtual ~Executor() = default;

protected:
    Database& db;
};
using ExecutorPtr = std::shared_ptr<Executor>;

class ExecutePipeline
{
public:
    ExecutePipeline& addExecutor(ExecutorPtr executor)
    {
        executors.push_back(executor);
        return *this;
    }

    std::any execute() const
    {
        std::any inter_data;
        for (int i = 0; i < executors.size(); i++)
        {
            inter_data = executors[i]->execute(inter_data);
        }
        return inter_data;
    }

private:
    std::vector<ExecutorPtr> executors;
};

using Scores = std::multimap<size_t, size_t, std::greater<>>;