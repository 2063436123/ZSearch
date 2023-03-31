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
    LimitExecutor(Database& db_, uint64_t limit_ = 100) : Executor(db_), original_limit(limit_), limit(limit_) {}

    // return doc ids
    std::pair<bool, std::any> execute(const std::any& input) override
    {
        auto doc_ids = std::any_cast<Scores>(input);
        auto iter = doc_ids.begin();

        // 受 vectorization model 影响，需要修改成员字段，这导致一个 (带有LimitExecutor的)Executors 只能被执行一次
        while (limit-- && iter != doc_ids.end())
            ++iter;
        doc_ids.erase(iter, doc_ids.end());
        return {true, doc_ids};
    }

    void clear() override
    {
        limit = original_limit;
    }
private:
    uint64_t original_limit;
    uint64_t limit;
};