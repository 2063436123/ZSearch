#pragma once

#include "Term.h"
#include "storage/Reader.h"
#include "extractor/Extractor.h"
#include "DocumentInfo.h"
#include "core/Key.h"
#include "core/Value.h"

class Document;
using DocumentPtr = std::shared_ptr<Document>;
using DocumentMap = std::unordered_map<size_t, DocumentPtr>;

class Document {
public:
    Document(size_t doc_id, std::filesystem::path origin_path_)
            : id(doc_id), origin_path(std::move(origin_path_)) {
        if (!is_regular_file(origin_path))
            THROW(FileTypeUnmatchException());
        info.changed_time = getModifiedLastDateTime(origin_path);
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

    size_t getWordCount() const
    {
        return info.word_count;
    }

    void setWordCount(size_t word_count)
    {
        info.word_count = word_count;
    }

    std::unordered_map<Key, Value> getKvs() const
    {
        std::lock_guard<std::mutex> guard(kvs_lock);
        return kvs;
    }

    void setKvs(const std::unordered_map<Key, Value> &kvs_)
    {
        std::lock_guard<std::mutex> guard(kvs_lock);
        kvs = kvs_;
    }

    void addKV(const Key& key, Value value)
    {
        std::lock_guard<std::mutex> guard(kvs_lock);
        kvs.emplace(key, value);
    }

    Value findKV(const Key &key) const
    {
        std::lock_guard<std::mutex> guard(kvs_lock);
        auto iter = kvs.find(key);
        if (iter == kvs.end())
            return {};
        return iter->second;
    }

    // 获取文档中的字符串，直接匹配的部分位于 [offset, offset + len) —— 一定会被完全输出，res_len 指明我们期望的总输出宽度.
    std::string getString(size_t offset, size_t len, size_t res_len) const
    {
        auto file_len = file_size(origin_path);
        int fd = ::open(origin_path.c_str(), O_RDONLY);
        if (fd < 0)
            THROW(Poco::FileNotFoundException());

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
            THROW(Poco::ReadFileException());

        auto res = std::string(buf, read_number);
        delete[] buf;
        return res;
    }

    void serialize(WriteBufferHelper &helper) const
    {
        helper.writeNumber(id);
        helper.writeString(origin_path.string());
        {
            std::lock_guard lg(kvs_lock);
            helper.writeNumber(kvs.size());
            for (const auto& pair : kvs)
            {
                pair.first.serialize(helper);
                pair.second.serialize(helper);
            }
        }
        info.serialize(helper);
    }

    static DocumentPtr deserialize(ReadBufferHelper &helper)
    {
        auto id = helper.readNumber<size_t>();
        std::filesystem::path path(helper.readString());
        auto document_ptr = std::make_shared<Document>(id, path);

        auto size = helper.readNumber<size_t>();
        std::unordered_map<Key, Value> kvs;
        for (size_t i = 0; i < size; i++)
        {
            kvs.emplace(Key::deserialize(helper), Value::deserialize(helper));
        }
        document_ptr->setKvs(kvs);

        document_ptr->setInfo(DocumentInfo::deserialize(helper));
        return document_ptr;
    }

private:
    size_t id;
    std::filesystem::path origin_path;

    mutable std::mutex kvs_lock;
    std::unordered_map<Key, Value> kvs;

    DocumentInfo info;
};