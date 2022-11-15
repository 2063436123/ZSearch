#pragma once

#include "typedefs.h"
#include "utils/SerializerUtils.h"


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

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeString(word);
        helper.writeLinearContainer(posting_list);
        helper.writeInteger(statistics_list.size());
        for (const auto& stat : statistics_list)
            helper.writeSetContainer(stat.offsets_in_file);
    }

    static Term deserialize(ReadBufferHelper &helper)
    {
        Term term;
        term.word = helper.readString();
        term.posting_list = helper.readLinearContainer<std::vector, size_t>();
        auto size = helper.readInteger<size_t>();
        for (size_t i = 0; i < size; i++)
            term.statistics_list.push_back(TermStatisticsWithInDoc{.offsets_in_file = helper.readSetContainer<std::set, size_t>()});
        return term;
    }

    bool isEmpty() const
    {
        return word.empty();
    }
};

using TermMap = std::unordered_map<std::string, Term>;