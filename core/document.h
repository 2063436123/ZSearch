#pragma once

#include "term.h"
#include "storage/reader.h"
#include "extractor/extractor.h"

class Document {
public:
    explicit Document(size_t doc_id, std::filesystem::path origin_path_)
            : id(doc_id), origin_path(std::move(origin_path_)) {}

    size_t getId() const
    {
        return id;
    }

    std::filesystem::path getPath() const
    {
        return origin_path;
    }

    std::string getString(size_t offset, size_t len, size_t res_len) const
    {
        int fd = ::open(origin_path.c_str(), O_RDONLY);
        if (fd < 0)
            throw Poco::FileNotFoundException();

        // if output like: xxx matched yyy
        // left_len is len(xxx)
        // right_len is len(matched yyy)
        size_t left_len = 0;
        size_t right_len = len;
        if (res_len > len)
        {
            // 尝试均匀输出左右侧上下文
            if ((res_len - len) / 2 <= offset)
            {
                left_len = (res_len - len) / 2;
                right_len += (res_len - len + 1) / 2;
            }
            else // 在最左侧，所以我们不均匀输出而是从左开始全部输出
            {
                left_len = offset;
                right_len = res_len - offset;
            }
        }
        assert(left_len >= 0);
        lseek(fd, offset - left_len, SEEK_SET);

        char *buf = new char[std::max(res_len, len)];

        ssize_t read_number = ::read(fd, buf, std::max(res_len, len));

        if (read_number < 0)
            throw Poco::ReadFileException();

        auto res = std::string(buf, read_number);
        delete[] buf;
        return res;
    }

private:
    size_t id;
    std::filesystem::path origin_path;
};

using DocumentMap = std::unordered_map<size_t, Document>;