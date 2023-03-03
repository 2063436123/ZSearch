#pragma once
#include <utility>

#include "typedefs.h"
#include "Document.h"
#include "utils/ContainerUtils.h"

class Database {
public:
    // TODO: 明确 new_database 的定位
    // 如果 new_database == false, 那么需要提前手动在 path 处创建文件夹
    // 如果 cover old，那么需要
    explicit Database(std::filesystem::path path, bool new_database = false) : database_path(std::move(path)), is_new_database(new_database)
    {
        if (!exists(database_path))
        {
            if (!new_database)
                THROW(DatabaseTypeException("not found database"));
            else
                std::filesystem::create_directory(database_path);
        }
        if (!is_directory(database_path))
            THROW(DatabaseTypeException());
        if (!new_database)
            deserialize();
    }

    bool is_a_new_database() const
    {
        return is_new_database;
    }

    std::filesystem::path getPath() const
    {
        return database_path;
    }

    size_t newDocId()
    {
        return next_doc_id++;
    }

    size_t maxAllocatedDocId() const
    {
        return next_doc_id - 1;
    }

    double getAvgWordCount() const
    {
        // TODO: 优化性能 保存 total + size 来计算 avg，注意文档被添加/删除时都要修改 total + size.
        std::lock_guard<std::mutex> guard(document_map_lock);
        size_t sum = 0, count = document_map.size();
        for (const auto& iter : document_map)
        {
            sum += iter.second->getWordCount();
        }

        return sum * 1.0 / count;
    }

    void addDocument(size_t doc_id, const std::string& doc_path, size_t word_count, const std::unordered_map<Key, Value>& kvs)
    {
        std::lock_guard<std::mutex> guard(document_map_lock);
        document_map.emplace(doc_id, std::make_shared<Document>(doc_id, doc_path, word_count, kvs));
    }

    DocumentPtr findDocument(size_t doc_id) const
    {
        std::lock_guard<std::mutex> guard(document_map_lock);
        auto document_ptr = document_map.find(doc_id);
        if (document_ptr == document_map.end())
            return nullptr;
        return document_ptr->second;
    }

    void deleteDocument(size_t doc_id)
    {
        std::lock_guard<std::mutex> guard(document_map_lock);
        document_map.erase(doc_id);
    }

    void addTerm(const std::string& word, size_t doc_id, size_t offset_in_file)
    {
        std::lock_guard<std::mutex> guard(term_map_lock);
        auto& term_ptr = term_map[word]; // use ref, for quick insertion if term_ptr is nullptr.
        if (term_ptr == nullptr)
        {
            term_ptr = std::make_shared<Term>(word);
        }
        auto &posting_list = term_ptr->posting_list;
        auto &statistics_list = term_ptr->statistics_list;

        auto doc_iter = std::lower_bound(posting_list.begin(), posting_list.end(), doc_id);
        auto stat_offset = doc_iter - posting_list.begin();

        if (doc_iter == posting_list.end() || *doc_iter != doc_id) // can't find doc_id (and related statistics), insert them.
        {
            posting_list.insert(doc_iter, doc_id);
            // 如果想要插入一个空的 TermStatisticsWithInDoc，use {{}} or TermStatisticsWithInDoc{} instead of {}, 因为 insert 支持插入 std::initializer_list，{} 会被当做一个空的初始化列表从而不插入任何东西
            statistics_list.insert(statistics_list.begin() + stat_offset, TermStatisticsWithInDoc{});
        }

        auto &offset_set = statistics_list[stat_offset].offsets_in_file;
        assert(!offset_set.contains(offset_in_file));
        offset_set.emplace(offset_in_file);
    }

    TermPtr findTerm(const std::string &word) const
    {
        std::lock_guard<std::mutex> guard(term_map_lock);
        auto iter = term_map.find(word);
        if (iter == term_map.end())
            return nullptr;
        return iter->second;
    }

    // 删除 posting_list, statistics_list 中过时元素（产生自已被删除的 document）
    void tidyTerm(const std::string& word)
    {
        std::lock_guard<std::mutex> guard(term_map_lock);
        auto iter = term_map.find(word);
        if (iter == term_map.end())
            return;
        const auto& posting_list = iter->second->posting_list;
        std::unordered_set<size_t> index_to_delete;
        for (size_t i = 0; i < posting_list.size(); i++)
        {
            if (findDocument(posting_list[i]) == nullptr)
                index_to_delete.emplace(i);
        }
        removeElements(iter->second->posting_list, index_to_delete);
        removeElements(iter->second->statistics_list, index_to_delete);
    }

    static void createDatabase(const std::filesystem::path& path)
    {
        if (!create_directory(path))
            THROW(Poco::CreateFileException("can't create database directory in " + path.string()));
    }

    static void destroyDatabase(const std::filesystem::path& path)
    {
        if (!exists(path))
            THROW(Poco::CreateFileException("no such database directory in " + path.string()));
        std::filesystem::remove_all(path);
    }

    void clear()
    {
        std::scoped_lock sl(term_map_lock, document_map_lock);
        term_map.clear();
        document_map.clear();
        next_doc_id = 1;
    }

    ~Database() {
        serialize();
    }

private:
    // 这两个变量不需要持久化
    bool is_new_database;
    std::filesystem::path database_path;

    TermMap term_map;
    mutable std::mutex term_map_lock;

    DocumentMap document_map;
    mutable std::mutex document_map_lock;


    void serialize() {
        std::scoped_lock sl(term_map_lock, document_map_lock);

        std::ofstream fout(database_path.string() + "/meta");
        WriteBuffer buf;
        WriteBufferHelper helper(buf);

        helper.writeNumber(next_doc_id.load());

        helper.writeNumber(term_map.size());
        for (const auto& pair : term_map)
        {
            helper.writeString(pair.first);
            pair.second->serialize(helper);
        }

        helper.writeNumber(document_map.size());
        for (const auto& pair : document_map)
        {
            helper.writeNumber(pair.first);
            pair.second->serialize(helper);
        }
        buf.dumpAllToStream(fout);
    }

    void deserialize() {
        std::scoped_lock sl(term_map_lock, document_map_lock);

        std::ifstream fin(database_path.string() + "/meta");
        if (!fin.is_open())
            return;

        ReadBuffer buf;
        buf.readAllFromStream(fin);
        ReadBufferHelper helper(buf);

        next_doc_id = helper.readNumber<size_t>();

        auto size = helper.readNumber<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            auto term_name = helper.readString();
            term_map.emplace(term_name, Term::deserialize(helper));
        }

        size = helper.readNumber<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            auto doc_id = helper.readNumber<size_t>();
            document_map.emplace(doc_id, Document::deserialize(helper));
        }
    }

    std::atomic_size_t next_doc_id = 1;
};
