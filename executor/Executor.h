#pragma once
#include "../typedefs.h"
#include "core/Database.h"

using DocIds = std::unordered_set<size_t>;

class Executor
{
public:
    Executor(Database& db_) : db(db_) {}

    virtual std::pair<bool, std::any> execute(const std::any &) = 0;

    virtual void clear() {}

    virtual ~Executor() = default;

protected:
    Database& db;
};
using ExecutorPtr = std::shared_ptr<Executor>;

using Scores = std::multimap<size_t, size_t, std::greater<>>;

class ExecutePipeline
{
public:
    ExecutePipeline& addExecutor(ExecutorPtr executor)
    {
        executors.push_back(executor);
        return *this;
    }

    // 注意，ExecutePipeline 被第二次使用前必须调用 clear() 方法来清除 executors 中的状态
    std::pair<bool, std::any> executePipeline() const
    {
        std::pair<bool, std::any> inter_data{true, DEFAULT_PATCH_SIZE};
        for (int i = 0; i < executors.size(); i++)
        {
            inter_data = executors[i]->execute(inter_data.second);
            if (!inter_data.first)
            {
                if (i == 0)
                    return {false, {}};
                THROW(Poco::LogicException("executor return eof too early!"));
            }
        }
        return inter_data;
    }

    // 收集全量数据后返回
    std::any execute()
    {
        // 清除状态，以便使用新的数据开始新的一轮执行流
        clear();

        std::any data;

        while (true)
        {
            std::pair<bool, std::any> inter_res = executePipeline();
            if (!inter_res.first)
                break;

            if (inter_res.second.type() == typeid(DocIds))
            {
                DocIds inter_data = std::any_cast<DocIds>(inter_res.second);
                if (data.has_value())
                    std::any_cast<DocIds>(data).insert(inter_data.begin(), inter_data.end());
                else
                    data = inter_data;
            }
            else if (inter_res.second.type() == typeid(Scores))
            {
                Scores inter_data = std::any_cast<Scores>(inter_res.second);
                if (data.has_value())
                    std::any_cast<Scores>(data).insert(inter_data.begin(), inter_data.end());
                else
                    data = inter_data;
            }
        }

        return data;
    }

    void clear()
    {
        for (int i = 0; i < executors.size(); i++)
            executors[i]->clear();
    }

private:
    std::vector<ExecutorPtr> executors;
};
