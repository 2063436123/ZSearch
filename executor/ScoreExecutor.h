#pragma once

#include "Executor.h"
#include "utils/DynamicBitSet.h"

/*
 by BM25 algorithm.
 TODO: 现在的模型只能处理 AND terms 查询，对于 OR、NOT 等条件，BM25 的评分将偏失
*/
class ScoreExecutor : public Executor
{
public:
    ScoreExecutor(Database& db_, const std::unordered_map<std::string, double>& word_freq_) : Executor(db_), word_freq(word_freq_) {}

    std::any execute(const std::any &doc_ids_) override
    {
        auto doc_ids = std::any_cast<DocIds>(doc_ids_);
        std::map<size_t, size_t, std::greater<>> scores; // score -> doc_id

        for (size_t doc_id : doc_ids)
        {
            double score = determineScore(doc_id);
            scores.emplace(score * SCORE_GRANULARITY, doc_id);
        }

        // 提取 doc_ids order by score
        return scores;
    }

private:
    // 计算查询与指定文档的相关性
    double determineScore(size_t doc_id)
    {
        double score = 0;
        for (auto iter : word_freq)
        {
            std::string word = iter.first;
            auto term_ptr = db.findTerm(word);
            auto document_ptr = db.findDocument(doc_id);

            // 1.单词权重
            double df = term_ptr->posting_list.size();
            if (df == 0) // TODO: 如果没有任何文档包含此单词（纯 AND terms 不会出现此情况），降低其权重为最低
                df = db.maxAllocatedDocId();
            double idf = log((db.maxAllocatedDocId() - df + 0.5) / (df + 0.5));

            // 2.单词与文档的相关性
            auto tf_iter = std::lower_bound(term_ptr->posting_list.begin(), term_ptr->posting_list.end(), doc_id);
            double tf = 0.0;
            if (tf_iter != term_ptr->posting_list.end())
                tf = 1.0 * term_ptr->statistics_list[tf_iter - term_ptr->posting_list.begin()].offsets_in_file.size() / document_ptr->getWordCount();

            double K = k1 * (1 - b + b * (document_ptr->getWordCount() / db.getAvgWordCount()));
            double sqd = (k1 + 1) * tf / (K + tf);

            // 3.单词与查询的相关性
            double freq_in_query = iter.second;
            double sqq = 1.0;
            if (word_freq.size() >= 10) // 对于较长的 query，才计算单词与 query 的相关性
                sqq = (k3 + 1) * freq_in_query / (k3 + freq_in_query);

            score += idf * sqd * sqq;
        }
        return score;
    }

    const double k1 = 1.5, k3 = 1.5, b = 0.75;
    std::unordered_map<std::string, double> word_freq;
};