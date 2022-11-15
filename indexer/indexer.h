#include "core/Database.h"
#include "core/Document.h"

// 一个 Indexer 绑定到一个 database 上，并且在其中创建新的 document
class Indexer {
public:
    // TODO: 支持更多文本类型，only txt at present.
    explicit Indexer(Database &db_)
            : db(db_) {}

    // file_path point at a document or a directory.
    void index(const std::filesystem::path &file_path)
    {
        if (is_directory(file_path))
        {
            for (const auto& file : std::filesystem::recursive_directory_iterator( file_path))
            {
                index(file.path());
            }
            return;
        }

        std::unique_ptr<Reader> reader = std::make_unique<TxtLineReader>(file_path);
        std::unique_ptr<Extractor> extractor = std::make_unique<WordExtractor>();

        size_t doc_id = db.newDocId();
        db.addDocument(doc_id, file_path);

        while (true)
        {
            auto line = reader->readUntil();
            if (line.str.empty())
                break;
            auto word_in_files = extractor->extract(line);
            for (const auto &word_in_file: word_in_files)
            {
                db.addTerm(word_in_file.str, doc_id, word_in_file.offset_in_file);
            }
        }
    }

private:
    Database &db;
};
