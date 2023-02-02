#pragma once
#include "../typedefs.h"
#include "core/Database.h"

using DocIds = std::vector<size_t>;

class Executor
{
public:
    Executor(Database& db_) : db(db_) {}

    virtual std::any execute(const std::any &) = 0;

    virtual ~Executor() = default;

protected:
    Database& db;
};

class ExecutePipeline
{
public:
    ExecutePipeline& addExecutor(Executor* executor)
    {
        executors.push_back(executor);
        return *this;
    }

    std::any execute()
    {
        std::any inter_data;
        for (int i = 0; i < executors.size(); i++)
        {
            inter_data = executors[i]->execute(inter_data);
        }
        return inter_data;
    }

private:
    std::vector<Executor*> executors;
};