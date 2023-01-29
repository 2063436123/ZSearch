#pragma once

#include "typedefs.h"
#include "utils/SerializeUtils.h"


struct TermStatisticsWithInDoc
{
    std::set<size_t> offsets_in_file;
};

using PostingList = std::vector<size_t>;
using StatisticsList = std::vector<TermStatisticsWithInDoc>;

class Term;
using TermPtr = std::shared_ptr<Term>;
using TermMap = std::unordered_map<std::string, TermPtr>;

struct Term {
    std::string word;
    // 两个 List 都是有序的，插入时需要二分搜索
    PostingList posting_list; // doc ids
    StatisticsList statistics_list; // statistics in correlated doc

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeString(word);
        helper.writeLinearContainer(posting_list);
        helper.writeNumber(statistics_list.size());
        for (const auto& stat : statistics_list)
            helper.writeSetContainer(stat.offsets_in_file);
    }

    static TermPtr deserialize(ReadBufferHelper &helper)
    {
        TermPtr term = std::make_shared<Term>();
        term->word = helper.readString();
        term->posting_list = helper.readLinearContainer<std::vector, size_t>();
        auto size = helper.readNumber<size_t>();
        for (size_t i = 0; i < size; i++)
            term->statistics_list.push_back(TermStatisticsWithInDoc{.offsets_in_file = helper.readSetContainer<std::set, size_t>()});
        return term;
    }
};
