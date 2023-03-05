#pragma once

#include "typedefs.h"
#include "executor/TermsExecutor.h"
#include "executor/HavingExecutor.h"
#include "executor/ScoreExecutor.h"
#include "executor/LimitExecutor.h"
#include "core/Database.h"

struct OldSearchResult
{
    DocumentPtr document;
    size_t offset_in_file;
    size_t related_text_len;

    // TODO: add score
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

void to_json(nlohmann::json& j, const SearchResult& result) {
    nlohmann::json::array_t texts;
    for (const auto& text : result.highlight_texts)
    {
        texts.push_back(std::unordered_map<std::string, std::string>{{"text", text}});
    }
    j = nlohmann::json{{"doc_id", result.doc_id}, {"path", result.doc_path}, {"highlight_texts", texts}, {"score", result.score}};
}

using SearchResultSet = std::vector<SearchResult>;

class Searcher
{
public:
    explicit Searcher(Database &db_)
            : db(db_) {}

    SearchResultSet search(const std::string& query)
    {
        ExecutePipeline pipeline;
        SearchResultSet res;

        // TODO: 加入 trie 的使用，在 executor 前找出真正单词, 这个过程获取可以放在 parser 中？

        // TODO: 引入 queryparser，支持更多语法的解析
        if (std::all_of(query.begin(), query.end(), [](char c) { return isalnum(c); }))
        {
            // TODO: 在这里处理后缀匹配吗
            auto querys = db.matchTerm(query);

            LeafNode<std::string> leaf_node(querys[0]);
            TermsExecutor terms_executor(db, &leaf_node);
            ScoreExecutor score_executor(db, {{query, 1.0}});
            LimitExecutor limit_executor(db, 10);

            pipeline.addExecutor(&terms_executor).addExecutor(&score_executor).addExecutor(&limit_executor);

            auto term_ptr = db.findTerm(querys[0]);
            if (!term_ptr)
                return res;

            auto scores = std::any_cast<std::map<size_t, size_t, std::greater<>>>(pipeline.execute());
            for (const auto &iter : scores)
            {
                auto document_ptr = db.findDocument(iter.second);
                if (!document_ptr)
                    continue;

                std::vector<std::string> highlight_texts;
                auto cur_doc_index = std::lower_bound(term_ptr->posting_list.begin(), term_ptr->posting_list.end(), iter.second) - term_ptr->posting_list.begin();
                assert(cur_doc_index < term_ptr->posting_list.size());

                for (auto offset_in_file : term_ptr->statistics_list[cur_doc_index].offsets_in_file)
                {
                    auto highlight_text = outputSmooth(document_ptr->getString(offset_in_file, query.size(), 50));
                    highlight_texts.push_back(highlight_text);
                }

                res.push_back(SearchResult{
                        .doc_id = iter.second,
                        .doc_path = document_ptr->getPath().string(),
                        .highlight_texts = highlight_texts,
                        .score = 1.0 * iter.first / SCORE_GRANULARITY
                });
            }

            // OldResultSet 逻辑
//            SearchResultSet res;
//
//            TermPtr term = db.findTerm(query);
//            if (!term)
//                return res;
//            assert(term->posting_list.size() == term->statistics_list.size());
//
//            for (size_t i = 0; i < term->posting_list.size(); i++)
//            {
//                for (auto offset_in_file : term->statistics_list[i].offsets_in_file)
//                    res.insert(OldSearchResult{.document = db.findDocument(term->posting_list[i]), .offset_in_file = offset_in_file, .related_text_len = query.size()});
//            }
//            return res;
        }
        else
        {
            THROW(Poco::NotImplementedException("unsupported query -- " + query));
        }
        return res;
    }

private:
    Database &db;
};