#pragma once

#include <utility>
#include "../typedefs.h"

struct DocumentInfo
{
    DateTime modify_time;
    size_t word_count;

    // user_name -> comment/rating
    std::unordered_multimap<std::string, std::pair<DateTime, std::string>> comments;
    std::unordered_map<std::string, double> ratings;

    // TODO: 序列化/反序列化 comments, ratings
    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeDateTime(modify_time);
        helper.writeNumber(word_count);
    }

    static DocumentInfo deserialize(ReadBufferHelper &helper)
    {
        DocumentInfo info{.modify_time = helper.readDateTime(), .word_count = helper.readNumber<size_t>()};
        return info;
    }
};