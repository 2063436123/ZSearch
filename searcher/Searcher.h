#pragma once

#include "typedefs.h"
#include "core/Database.h"

struct SearchResult {
    DocumentPtr document;
    size_t offset_in_file;
    size_t related_text_len;

    // TODO: add score
    double score = 0.0;

    bool operator<(const SearchResult& rhs) const
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

using SearchResultSet = std::set<SearchResult>;

class Searcher {
public:
    explicit Searcher(Database &db_)
            : db(db_) {}

    SearchResultSet search(const std::string &query)
    {
        if (std::all_of(query.begin(), query.end(), [](char c) { return isalnum(c); }))
        {
            SearchResultSet res;

            TermPtr term = db.findTerm(query);
            if (!term)
                return res;
            assert(term->posting_list.size() == term->statistics_list.size());

            for (size_t i = 0; i < term->posting_list.size(); i++)
            {
                for (auto offset_in_file : term->statistics_list[i].offsets_in_file)
                    res.insert(SearchResult{.document = db.findDocument(term->posting_list[i]), .offset_in_file = offset_in_file, .related_text_len = query.size()});
            }
            return res;
        }
        else
        {
            THROW(Poco::NotImplementedException());
        }
    }

private:
    Database &db;
};