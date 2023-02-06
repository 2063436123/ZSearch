#pragma once

#include "typedefs.h"
#include "executor/TermsExecutor.h"
#include "executor/HavingExecutor.h"
#include "executor/ScoreExecutor.h"
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

    SearchResultSet search(const std::string &query)
    {
        ExecutePipeline pipeline;
        SearchResultSet res;

        // TODO: 引入 queryparser，支持更多语法的解析
        if (std::all_of(query.begin(), query.end(), [](char c) { return isalnum(c); }))
        {
            LeafNode<std::string> leaf_node(query);
            TermsExecutor terms_executor(db, &leaf_node);
            ScoreExecutor score_executor(db, {{query, 1.0}});
            pipeline.addExecutor(&terms_executor).addExecutor(&score_executor);

            // TODO: 添加 highlight_text
            auto scores = std::any_cast<std::map<size_t, size_t, std::greater<>>>(pipeline.execute());
            for (const auto &iter : scores)
            {
                auto document_ptr = db.findDocument(iter.second);
                res.push_back(SearchResult{
                        .doc_id = iter.second,
                        .doc_path = document_ptr ? (document_ptr->getPath().string()) : "deleted",
                        .highlight_texts = {"TODO1", "TODO2", "TODOmore"},
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
            THROW(Poco::NotImplementedException());
        }
        return res;
    }

private:
    Database &db;
};