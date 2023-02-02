#pragma once

#include <utility>
#include "../typedefs.h"

struct DocumentInfo
{
    DateTime changed_time;
    size_t word_count;

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeDateTime(changed_time);
        helper.writeNumber(word_count);
    }

    static DocumentInfo deserialize(ReadBufferHelper &helper)
    {
        DocumentInfo info{.changed_time = helper.readDateTime(), .word_count = helper.readNumber<size_t>()};
        return info;
    }
};