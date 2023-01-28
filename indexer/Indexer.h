#pragma once

#include "core/Database.h"
#include "core/Document.h"

// 一个 Indexer 绑定到一个 database 上，并且在其中创建新的 document
class Indexer {
public:
    explicit Indexer(Database &db_)
            : db(db_) {}

    // file_path point at a document or a directory.
    void index(const std::filesystem::path &file_path)
    {
        if (is_directory(file_path))
        {
            for (const auto& file : std::filesystem::recursive_directory_iterator(file_path))
            {
                index(file.path());
            }
            return;
        }

        size_t doc_id = db.newDocId();
        db.addDocument(doc_id, file_path);

        if (file_path.extension() == "txt")
        {
            std::unique_ptr<Reader> reader = std::make_unique<TxtLineReader>(file_path);
            std::unique_ptr<Extractor> extractor = std::make_unique<WordExtractor>(std::move(reader));

            auto word_in_files = extractor->extract();
            if (!word_in_files.is_valid)
                return;
            for (const auto &word_in_file : word_in_files.words)
            {
                db.addTerm(word_in_file.str, doc_id, word_in_file.offset_in_file);
            }
        }
        else if (file_path.extension() == "json")
        {
            std::unique_ptr<Reader> reader = std::make_unique<TxtLineReader>(file_path);
            std::unique_ptr<Extractor> extractor = std::make_unique<JsonExtractor>(std::move(reader));

            auto words_and_kvs = extractor->extract();
            if (!words_and_kvs.is_valid)
                return;
            for (const auto &word_in_file : words_and_kvs.words)
            {
                db.addTerm(word_in_file.str, doc_id, word_in_file.offset_in_file);
            }

            auto document_ptr = db.findDocument(doc_id);
            for (const auto& [key, value] : words_and_kvs.kvs)
            {
                document_ptr->addKV(key, value);
            }
        }
        else if (file_path.extension() == "csv")
        {
            THROW(Poco::NotImplementedException());
//            std::unique_ptr<Reader> reader = std::make_unique<TxtLineReader>(file_path);
//            std::unique_ptr<Extractor> extractor = std::make_unique<CSVExtractor>(file_path.stem(), std::move(reader));
//
//            auto table_in_file = extractor->extract();
//            if (!table_in_file.is_valid)
//                return;
//            table_in_file.table.setDocId(doc_id);
//            db.addTable(table_in_file.table.name(), std::make_shared<Table>(table_in_file.table));
        }
        else
        {
            THROW(Poco::NotImplementedException());
        }
    }

private:
    Database &db;
};
