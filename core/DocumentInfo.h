#pragma once

#include <utility>
#include "../typedefs.h"

enum class DocumentType {
    NORMAL, // natural language paragraphs, such as literature
    CSV,
    JSON,
    BLOB, // pdf, excel et.
    UNKNOWN
};

struct DocumentInfo {
    DocumentType type = DocumentType::NORMAL;
    DateTime changed_time;
};