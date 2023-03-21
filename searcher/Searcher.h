#pragma once

#include "typedefs.h"
#include "executor/TermsExecutor.h"
#include "executor/HavingExecutor.h"
#include "executor/ScoreExecutor.h"
#include "executor/LimitExecutor.h"
#include "core/Database.h"
#include "QueryStatistics.h"
#include "queryparser/Parser.h"

struct OldSearchResult
{
    DocumentPtr document;
    size_t offset_in_file;
    size_t related_text_len;

    double score = 0.0;

    bool operator<(const OldSearchResult &rhs) const
    {
        if (score != rhs.score)
            return score < rhs.score;
        if (!document || !rhs.document)
            return false;
        if (document->getId() != rhs.document->getId())
            return document->getId() < rhs.document->getId();
        return offset_in_file < rhs.offset_in_file;
    }
};

struct SearchResult
{
    size_t doc_id;
    std::string doc_path;
    std::vector<std::string> highlight_texts;
    double score;

    bool operator<(const SearchResult &rhs) const
    {
        return score > rhs.score || doc_id < rhs.doc_id;
    }
};

void to_json(nlohmann::json &j, const SearchResult &result)
{
    nlohmann::json::array_t texts;
    for (const auto &text : result.highlight_texts)
    {
        texts.push_back(std::unordered_map<std::string, std::string>{{"text", text}});
    }
    j = nlohmann::json{{"doc_id",               result.doc_id},
                       {"path",                 result.doc_path},
                       {"first_highlight_text", result.highlight_texts[0]},
                       {"highlight_texts",      texts},
                       {"score",                result.score}};
}

using SearchResultSet = std::vector<SearchResult>;

class Searcher
{
public:
    explicit Searcher(Database &db_)
            : db(db_) {}

    SearchResultSet search(const std::string &query)
    {
        if (query.empty())
            return {};

        SearchResultSet res;

        auto transformScoresToResult = [this, &res](ExecutePipeline& pipeline, const std::string& word) -> void {
            auto scores = std::any_cast<Scores>(pipeline.execute());

            auto term_ptr = db.findTerm(word);
            if (!word.empty() && !term_ptr) // term_ptr 被删除，结果集失效 ———— 无法描述结果集
                return;

            for (const auto &iter : scores)
            {
                auto document_ptr = db.findDocument(iter.second);
                if (!document_ptr)
                    continue;

                std::vector<std::string> highlight_texts;
                if (!word.empty()) // query 中有 terms
                {
                    auto cur_doc_index =
                            std::lower_bound(term_ptr->posting_list.begin(), term_ptr->posting_list.end(),
                                             iter.second) - term_ptr->posting_list.begin();
                    assert(cur_doc_index < term_ptr->statistics_list.size());

                    for (auto offset_in_file : term_ptr->statistics_list[cur_doc_index].offsets_in_file)
                    {
                        std::string string_in_file = document_ptr->getString(offset_in_file, word.size(), 80);
                        auto highlight_text = outputSmooth(string_in_file);
                        highlight_texts.push_back(highlight_text);
                    }
                }
                else // query 中有 having 子句，而没有 terms
                {
                    highlight_texts.emplace_back("未产生匹配文本 —— 因为查询未指定 term");
                }

                res.push_back(SearchResult{
                        .doc_id = iter.second,
                        .doc_path = document_ptr->getPath().string(),
                        .highlight_texts = highlight_texts,
                        .score = 1.0 * iter.first / SCORE_GRANULARITY
                });
            }
        };

        Timer search_timer;

        if (std::all_of(query.begin(), query.end(), [](char c) { return !Poco::Ascii::isSpace(c); }))
        {
            // TODO: 在这里用 trie 处理后缀匹配吗
            std::vector<std::string> querys = db.matchTerm(query, 3);

            for (int query_id = 0; query_id < querys.size(); query_id++)
            {
                LeafNode<std::string> leaf_node(querys[query_id]);
                auto terms_executor = std::make_shared<TermsExecutor>(db, &leaf_node);
                auto score_executor = std::make_shared<ScoreExecutor>(db, std::unordered_map<std::string, double>{{querys[query_id], 1.0}});
                auto limit_executor = std::make_shared<LimitExecutor> (db, 10);

                // TODO: 考虑执行 DAG，比如多个 score_executor 作为一个 limit_executor 的输入.
                ExecutePipeline pipeline;
                pipeline.addExecutor(terms_executor).addExecutor(score_executor).addExecutor(limit_executor);

                transformScoresToResult(pipeline, querys[query_id]);
            }
        }
        else
        {
            auto [type, ast] = parseQuery(query);
            if (type == QueryErrorType::Non)
            {
                auto query_ast = ast->as<ASTQuery>();
                ExecutePipeline pipeline = query_ast->toExecutorPipeline(db);
                transformScoresToResult(pipeline, query_ast->getTerm().value_or(""));
            }
        }

        // 收集查询本身的统计信息
        if (search_timer.elapsedMilliseconds() > 0 || !res.empty())
            db.addQueryStatistics(search_timer.getStartTime(),
                                  std::make_shared<QueryStatistics>(query, search_timer.elapsedMilliseconds(), res.size()));

        // 收集最经常被查询到的文档的编号
        std::vector<size_t> doc_ids;
        for (const auto& search_result : res)
        {
            doc_ids.push_back(search_result.doc_id);
        }
        db.addDocumentQueryFreq(doc_ids);

        return res;
    }

private:
    Database &db;
};