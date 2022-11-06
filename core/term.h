#pragma once

#include "typedefs.h"


struct TermStatisticsWithInDoc
{
    std::set<size_t> offsets_in_file;
};

using PostingList = std::vector<size_t>;
using StatisticsList = std::vector<TermStatisticsWithInDoc>;

struct Term {
    std::string word;
    PostingList posting_list; // doc ids
    StatisticsList statistics_list; // statistics in correlated doc

    bool isEmpty() const
    {
        return word.empty();
    }
};

using TermMap = std::unordered_map<std::string, Term>;