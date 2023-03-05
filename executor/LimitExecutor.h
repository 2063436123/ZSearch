#pragma once

#include "Executor.h"
#include "ConjunctionTree.h"
#include "utils/DynamicBitSet.h"

/*
terms : terms 'AND' terms
    | terms 'OR' terms
    | 'NOT' terms
    | '(' terms ')'
    | term
    ;
*/
class LimitExecutor : public Executor
{
public:
    LimitExecutor(Database& db_, uint64_t limit_) : Executor(db_), limit(limit_) {}

    // return doc ids
    std::any execute(const std::any& input) override
    {
        auto doc_ids = std::any_cast<std::map<size_t, size_t, std::greater<>>>(input);
        auto iter = doc_ids.begin();
        while (limit-- && iter != doc_ids.end())
            ++iter;
        doc_ids.erase(iter, doc_ids.end());
        return doc_ids;
    }

private:
    uint64_t limit;
};