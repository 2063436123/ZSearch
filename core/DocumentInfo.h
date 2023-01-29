#pragma once

#include <utility>
#include "../typedefs.h"

struct DocumentInfo
{
    DateTime changed_time;

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeDateTime(changed_time);
    }

    static DocumentInfo deserialize(ReadBufferHelper &helper)
    {
        DocumentInfo info{.changed_time = helper.readDateTime()};
        return info;
    }
};