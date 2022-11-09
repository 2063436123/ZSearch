#pragma once
#include <utility>

#include "typedefs.h"
#include "document.h"

class Database {
public:
    explicit Database(std::filesystem::path path_, bool create = false) : database_path(std::move(path_))
    {
        if (!exists(database_path))
        {
            if (!create)
                return;
            else
                std::filesystem::create_directory(database_path);
        }
        if (!is_directory(database_path))
            throw DatabaseTypeException();
        if (!create)
            deserialize();
    }

    size_t newDocId()
    {
        return next_doc_id++;
    }

    void addDocument(size_t doc_id, const std::string& doc_path)
    {
        std::lock_guard<std::mutex> guard(document_map_lock);
        document_map.emplace(doc_id, Document(doc_id, doc_path));
    }

    void addTerm(const std::string& word, size_t doc_id, size_t offset_in_file)
    {
        std::lock_guard<std::mutex> guard(term_map_lock);
        if (!term_map.contains(word))
            term_map[word].word = word;
        auto &posting_list = term_map[word].posting_list;
        auto &statistics_list = term_map[word].statistics_list;

        auto doc_iter = std::lower_bound(posting_list.begin(), posting_list.end(), doc_id);
        auto stat_offset = doc_iter - posting_list.begin();

        if (doc_iter == posting_list.end() || *doc_iter != doc_id) // can't find doc_id and related statistics, insert them.
        {
            posting_list.insert(doc_iter, doc_id);
            // 如果想要插入一个空的 TermStatisticsWithInDoc，use {{}}/TermStatisticsWithInDoc{} instead of {}, 因为 insert 支持插入 std::initializer_list，{} 会被当做一个空的初始化列表从而不插入任何东西
            statistics_list.insert(statistics_list.begin() + stat_offset, TermStatisticsWithInDoc{});
        }

        auto &offset_set = statistics_list[stat_offset].offsets_in_file;
        assert(!offset_set.contains(offset_in_file));
        offset_set.emplace(offset_in_file);
    }

    Document findDocument(size_t doc_id) const
    {
        std::lock_guard<std::mutex> guard(document_map_lock);
        assert(document_map.contains(doc_id));
        return document_map.find(doc_id)->second;
    }

    Term findTerm(const std::string &word) const
    {
        std::lock_guard<std::mutex> guard(term_map_lock);
        auto iter = term_map.find(word);
        if (iter == term_map.end())
            return {};
        return iter->second;
    }

    static void createDatabase(const std::filesystem::path& path)
    {
        if (!create_directory(path))
            throw Poco::CreateFileException("can't create directory in " + path.string());
    }

    ~Database() {
        serialize();
    }
private:

    std::filesystem::path database_path;

    TermMap term_map;
    mutable std::mutex term_map_lock;

    DocumentMap document_map;
    mutable std::mutex document_map_lock;

    // 参考 Clickhouse 中的 writeVarBuf 等方法?
    void serialize() {
        std::scoped_lock<std::mutex, std::mutex> sl(term_map_lock, document_map_lock);

        std::ofstream fout(database_path.string() + "/meta");
        WriteBuffer buf;
        WriteBufferHelper helper(buf);
        term_map_lock.unlock();
        auto test_term = this->findTerm("my");
        term_map_lock.lock();

        helper.writeInteger(next_doc_id.load());

        helper.writeInteger(term_map.size());
        for (const auto& pair : term_map)
        {
            helper.writeString(pair.first);
            pair.second.serialize(helper);
        }

        helper.writeInteger(document_map.size());
        for (const auto& pair : document_map)
        {
            helper.writeInteger(pair.first);
            pair.second.serialize(helper);
        }
        buf.dumpAllToStream(fout);
    }

    void deserialize() {
        std::scoped_lock<std::mutex, std::mutex> sl(term_map_lock, document_map_lock);

        std::ifstream fin(database_path.string() + "/meta");
        ReadBuffer buf;
        buf.readAllFromStream(fin);
        ReadBufferHelper helper(buf);

        next_doc_id = helper.readInteger<size_t>();

        auto size = helper.readInteger<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            term_map.emplace(helper.readString(), Term::deserialize(helper));
        }

        size = helper.readInteger<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            document_map.emplace(helper.readInteger<size_t>(), Document::deserialize(helper));
        }
    }

    std::atomic_size_t next_doc_id = 1;
};
