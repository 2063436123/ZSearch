#pragma once
#include <utility>

#include "typedefs.h"
#include "Document.h"

class Database {
public:
    explicit Database(std::filesystem::path path, bool cover_old = false) : database_path(std::move(path))
    {
        if (!exists(database_path))
        {
            if (!cover_old)
                return;
            else
                std::filesystem::create_directory(database_path);
        }
        if (!is_directory(database_path))
            THROW(DatabaseTypeException());
        if (!cover_old)
            deserialize();
    }

    size_t newDocId()
    {
        return next_doc_id++;
    }

    size_t maxAllocatedDocId() const
    {
        return next_doc_id - 1;
    }

    void addDocument(size_t doc_id, const std::string& doc_path)
    {
        std::lock_guard<std::mutex> guard(document_map_lock);
        document_map.emplace(doc_id, std::make_shared<Document>(doc_id, doc_path));
    }

    void addTerm(const std::string& word, size_t doc_id, size_t offset_in_file)
    {
        std::lock_guard<std::mutex> guard(term_map_lock);
        if (!term_map.contains(word))
        {
            auto insert_res = term_map.emplace(word, std::make_shared<Term>());
            assert(insert_res.second);
            auto iter = insert_res.first;
            iter->second->word = word;
        }
        auto &posting_list = term_map[word]->posting_list;
        auto &statistics_list = term_map[word]->statistics_list;

        auto doc_iter = std::lower_bound(posting_list.begin(), posting_list.end(), doc_id);
        auto stat_offset = doc_iter - posting_list.begin();

        if (doc_iter == posting_list.end() || *doc_iter != doc_id) // can't find doc_id and related statistics, insert them.
        {
            posting_list.insert(doc_iter, doc_id);
            // 如果想要插入一个空的 TermStatisticsWithInDoc，use {{}} or TermStatisticsWithInDoc{} instead of {}, 因为 insert 支持插入 std::initializer_list，{} 会被当做一个空的初始化列表从而不插入任何东西
            statistics_list.insert(statistics_list.begin() + stat_offset, TermStatisticsWithInDoc{});
        }

        auto &offset_set = statistics_list[stat_offset].offsets_in_file;
        assert(!offset_set.contains(offset_in_file));
        offset_set.emplace(offset_in_file);
    }

//    void addTable(const std::string& table_name, const TablePtr& table)
//    {
//        std::lock_guard<std::mutex> guard(table_map_lock);
//        table_map[table_name] = table;
//    }

    DocumentPtr findDocument(size_t doc_id) const
    {
        std::lock_guard<std::mutex> guard(document_map_lock);
        assert(document_map.contains(doc_id));
        return document_map.find(doc_id)->second;
    }

    TermPtr findTerm(const std::string &word) const
    {
        std::lock_guard<std::mutex> guard(term_map_lock);
        auto iter = term_map.find(word);
        if (iter == term_map.end())
            return nullptr;
        return iter->second;
    }

//    TablePtr findTable(const std::string &word) const
//    {
//        std::lock_guard<std::mutex> guard(table_map_lock);
//        auto iter = table_map.find(word);
//        if (iter == table_map.end())
//            return nullptr;
//        return iter->second;
//    }

    static void createDatabase(const std::filesystem::path& path)
    {
        if (!create_directory(path))
            THROW(Poco::CreateFileException("can't create directory in " + path.string()));
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

//    TableMap table_map;
//    mutable std::mutex table_map_lock;


    // 参考 Clickhouse 中的 writeVarBuf 等方法?
    void serialize() {
//        std::scoped_lock<std::mutex, std::mutex, std::mutex> sl(term_map_lock, document_map_lock, table_map_lock);
        std::scoped_lock sl(term_map_lock, document_map_lock);

        std::ofstream fout(database_path.string() + "/meta");
        WriteBuffer buf;
        WriteBufferHelper helper(buf);
        term_map_lock.unlock();
        auto test_term = this->findTerm("my");
        term_map_lock.lock();

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

//        helper.writeNumber(table_map.size());
//        for (const auto& pair : table_map)
//        {
//            helper.writeString(pair.first);
//            pair.second->serialize(helper);
//        }

        buf.dumpAllToStream(fout);
    }

    void deserialize() {
//        std::scoped_lock<std::mutex, std::mutex, std::mutex> sl(term_map_lock, document_map_lock, table_map_lock);
        std::scoped_lock sl(term_map_lock, document_map_lock);

        std::ifstream fin(database_path.string() + "/meta");
        ReadBuffer buf;
        buf.readAllFromStream(fin);
        ReadBufferHelper helper(buf);

        next_doc_id = helper.readNumber<size_t>();

        auto size = helper.readNumber<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            term_map.emplace(helper.readString(), Term::deserialize(helper));
        }

        size = helper.readNumber<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            document_map.emplace(helper.readNumber<size_t>(), Document::deserialize(helper));
        }

//        size = helper.readNumber<size_t>();
//        for (size_t i = 0; i < size; i++)
//        {
//            table_map.emplace(helper.readString(), Table::deserialize(helper));
//        }
    }

    std::atomic_size_t next_doc_id = 1;
};
