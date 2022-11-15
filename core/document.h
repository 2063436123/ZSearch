#pragma once

#include "Term.h"
#include "storage/Reader.h"
#include "extractor/Extractor.h"
#include "DocumentInfo.h"

class Document {
public:
    Document(size_t doc_id, std::filesystem::path origin_path_)
            : id(doc_id), origin_path(std::move(origin_path_)) {
        if (!is_regular_file(origin_path))
            throw FileTypeUnmatchException();
        struct stat file_stat;
        stat(origin_path.c_str(), &file_stat);
        info.type = decideFileType(origin_path);
        info.changed_time = file_stat.st_mtimespec;
    }

    size_t getId() const
    {
        return id;
    }

    std::filesystem::path getPath() const
    {
        return origin_path;
    }

    void setPath(std::filesystem::path path) {
        origin_path = path;
    }

    DocumentInfo getInfo() const {
        return info;
    }

    void setInfo(const DocumentInfo& info_)
    {
        info = info_;
    }

    // 获取文档中的字符串，直接匹配的部分位于 [offset, offset + len) —— 一定会被完全输出，res_len 指明我们期望的总输出宽度.
    std::string getString(size_t offset, size_t len, size_t res_len) const
    {
        auto file_len = file_size(origin_path);
        int fd = ::open(origin_path.c_str(), O_RDONLY);
        if (fd < 0)
            throw Poco::FileNotFoundException();

        // if output like: xxx matched yyy
        // offset points to 'm' of "matched"
        // left_len is len(xxx)
        // right_len is len(matched yyy)
        // left_len + right_len == max(res_len, len)
        size_t left_len = 0;
        if (res_len > len)
        {
            bool is_left_enough_chars = (res_len - len) / 2 <= offset;
            bool is_right_enough_chars = (res_len - len + 1) / 2 + len <= file_len - offset;

            // 尝试均匀输出左右侧上下文
            if (is_left_enough_chars && is_right_enough_chars)
            {
                left_len = (res_len - len) / 2;
            }
            else if (is_right_enough_chars)
                // 整个文档长度小于 res_len，直接输出整个文档
                // or
                // 在最左侧，所以我们从左开始尽可能输出
            {
                left_len = offset;
            }
            else // 在最右侧，所以我们从右开始尽可能输出
            {
                left_len = res_len - (file_len - offset);
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

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeInteger(id);
        helper.writeString(origin_path.string());
    }

    static Document deserialize(ReadBufferHelper &helper)
    {
        auto id = helper.readInteger<size_t>();
        std::filesystem::path path(helper.readString());
        return {id, path};
    }

private:
    DocumentType decideFileType(const std::filesystem::path& path) {
        auto extension = path.extension().string();
        if (extension.empty())
            return DocumentType::UNKNOWN;
        if (extension == "txt")
            return DocumentType::NORMAL;
        if (extension == "csv")
            return DocumentType::CSV;
        if (extension == "json")
            return DocumentType::JSON;
        return DocumentType::NORMAL;
    }

    size_t id;
    std::filesystem::path origin_path;
    DocumentInfo info;
};

using DocumentMap = std::unordered_map<size_t, Document>;