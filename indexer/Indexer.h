#pragma once

#include "core/Database.h"
#include "core/Document.h"
#include "utils/FileSystemUtils.h"

// 一个 Indexer 绑定到一个 database 上，并且在其中创建新的 document
// Indexer 应该被单线程使用，not thread-safe
class Indexer {
public:
    explicit Indexer(Database &db_)
            : db(db_) {}

    // path point at a document or a directory.
    void index(const std::filesystem::path &path)
    {
        for (const auto& file : gatherExistedFiles(path))
            indexFile(path);
    }

    size_t indexFile(const std::filesystem::path &file_path)
    {
        if (!is_regular_file(file_path))
            return 0;

        if (IGNORED_FILE_EXTENSIONS.contains(file_path.extension()))
        {
            // ignore
            return 0;
        }

        if (file_path.extension() == ".json")
        {
            std::unique_ptr<Reader> reader = std::make_unique<TxtLineReader>(file_path);
            std::unique_ptr<Extractor> extractor = std::make_unique<JsonExtractor>(std::move(reader));

            ExtractResult words_and_kvs = extractor->extract();

            if (!words_and_kvs.is_valid)
                return 0;

            size_t doc_id = db.newDocId();
            for (const auto &word_in_file : words_and_kvs.words)
            {
                db.addTerm(word_in_file.str, doc_id, word_in_file.offset_in_file);
            }

            db.addDocument(doc_id, file_path, words_and_kvs.words.size(), words_and_kvs.kvs);
            return doc_id;
        }
        else if (ALLOWED_FILE_EXTENSIONS.contains(file_path.extension())) // 白名单中的文本类型都视为 .txt
        {
            std::unique_ptr<Reader> reader = std::make_unique<TxtLineReader>(file_path);
            std::unique_ptr<Extractor> extractor = std::make_unique<WordExtractor>(std::move(reader));

            auto word_in_files = extractor->extract();
            if (!word_in_files.is_valid)
                return 0;

            size_t doc_id = db.newDocId();
            for (const auto &word_in_file : word_in_files.words)
            {
                db.addTerm(word_in_file.str, doc_id, word_in_file.offset_in_file);
            }

            assert(word_in_files.kvs.empty());
            db.addDocument(doc_id, file_path, word_in_files.words.size(), word_in_files.kvs);
            return doc_id;
        }
        return 0;
    }

private:
    Database &db;
};
