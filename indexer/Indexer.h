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
            for (const auto& file : std::filesystem::directory_iterator(file_path))
            {
                index(file.path());
            }
            return;
        }

        size_t doc_id = db.newDocId();
        db.addDocument(doc_id, file_path);

        if (IGNORED_FILE_EXTENSIONS.contains(file_path.extension()))
        {
            // ignore
        }
        else if (file_path.extension() == ".json")
        {
            std::unique_ptr<Reader> reader = std::make_unique<TxtLineReader>(file_path);
            std::unique_ptr<Extractor> extractor = std::make_unique<JsonExtractor>(std::move(reader));

            ExtractResult words_and_kvs;
            try
            {
                words_and_kvs = extractor->extract();
            }
            catch (const nlohmann::json::exception& j)
            {
                THROW(ParseException(j.what() + std::string(" file_path - ") + file_path.string()));
            }

            if (!words_and_kvs.is_valid)
                return;
            for (const auto &word_in_file : words_and_kvs.words)
            {
                db.addTerm(word_in_file.str, doc_id, word_in_file.offset_in_file);
            }

            auto document_ptr = db.findDocument(doc_id);
            document_ptr->setKvs(words_and_kvs.kvs);
        }
        else if (file_path.extension() == ".csv")
        {
            THROW(Poco::NotImplementedException());
        }
        else if (is_regular_file(file_path)) // 其他的文本类型都视为 .txt
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
    }

private:
    Database &db;
};
