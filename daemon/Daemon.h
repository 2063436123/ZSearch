#pragma once
#include "../typedefs.h"
#include "core/Database.h"
#include "indexer/Indexer.h"

namespace std
{
    template<>
    struct hash<std::filesystem::path>
    {
        std::size_t operator()(const std::filesystem::path& p) const
        {
            return hash_value(p);
        }
    };
}

template <typename T>
void httpLog(const T& msg)
{
    static std::mutex log_lock;
    std::lock_guard lg(log_lock);
    std::cout << '[' << DateTime().string(true) << "] " << msg << std::endl;
}

using FileToDocId = std::unordered_map<std::string, size_t>;

// 监听索引中的文件变化，以单条索引中的每个文件为粒度 ———— 最细粒度.
class FileSystemDaemon {
public:
    FileSystemDaemon(Database& db_) : db(db_), indexer(db_) {}

    void run(Poco::Timer& timer)
    {
        assert(timer.skipped() == 0);
        std::lock_guard lg(paths_lock);

        httpLog("indexed paths checker starting...");
        for (const auto& path : paths)
        {
            smartIndexAndRecord(path.first);
        }
    }

    void rebuildAllPaths()
    {
        std::lock_guard lg(paths_lock);
        db.clear();
        for (auto& path : paths)
        {
            path.second.clear();
            smartIndexAndRecord(path.first);
        }
    }

    // NOTE: 这些方法应该被 http 调用
    // TODO: 添加检查合并路径的机制，移除冗余的 paths 条目

    // NOTE: 即使 path 当前不存在，也可以预添加
    // e.g. '/a' includes '/a/b'
    void addPath(const std::filesystem::path& path)
    {
        std::lock_guard lg(paths_lock);
        paths.emplace(path.string(), FileToDocId{});
        smartIndexAndRecord(path);
    }

    void removePath(const std::filesystem::path& path)
    {
        std::lock_guard lg(paths_lock);
        paths.erase(path);
    }

    auto getPaths() const
    {
        std::lock_guard lg(paths_lock);
        return paths;
    }

    // TODO: 添加序列化/反序列化方法

    static std::unordered_set<std::string> gatherExistedFiles(const std::filesystem::path& path)
    {
        if (!exists(path))
            return {};

        if (!is_directory(path))
            return {path.string()};

        std::unordered_set<std::string> res;
        for (const auto& path_entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (path_entry.is_regular_file() && !IGNORED_FILE_EXTENSIONS.contains(path_entry.path().extension()))
            {
               res.insert(path_entry.path().string());
            }
        }
        return res;
    }

private:
    // 只对发生变化的(新)文件重新索引，并删除过时 doc_id.
    // not thread-safe, caller promise.
    void smartIndexAndRecord(const std::filesystem::path& path)
    {
        if (!exists(path))
            return;
//        std::cout << "[smartIndex " << path.string() << "]" << std::endl;
        std::unordered_map<std::string, size_t>& indexed_documents = paths[path];
        std::unordered_set<std::string> existed_files = gatherExistedFiles(path);

        std::vector<std::string> files_to_index;

        // 1. 查看新文件
        for (const auto& existed_file : existed_files)
        {
            if (!indexed_documents.contains(existed_file))
            {
//                std::cout << "\tnew file: " << existed_file << std::endl;
                files_to_index.push_back(existed_file);
            }
        }

        // 2. 查看旧文件
        for (auto iter = indexed_documents.begin(); iter != indexed_documents.end();)
        {
            auto old_file_path = iter->first;
            auto old_doc_id = iter->second;
            // 2.1. 已删除
            if (!existed_files.contains(old_file_path))
            {
//                std::cout << "\tdeleted file: " << old_file_path << std::endl;
                db.deleteDocument(old_doc_id);
                iter = indexed_documents.erase(iter);
            }
            // 2.2. 已修改
            else if (auto document_ptr = db.findDocument(old_doc_id))
            {
                if (document_ptr->getModifyTime() < getModifiedLastDateTime(old_file_path))
                {
//                    std::cout << "\toutdated file: " << old_file_path << std::endl;
                    // 前半段删除就索引数据，逻辑同 2.1.
                    db.deleteDocument(old_doc_id);
                    iter = indexed_documents.erase(iter);
                    // 后半段重新索引，逻辑同 1.
                    files_to_index.push_back(old_file_path);
                }
                else
                {
                    ++iter;
                }
            }
            else
            {
                ++iter;
            }
        }

        // 3. (重新)索引文件
        for (const auto& file_path : files_to_index)
        {
            size_t doc_id = indexer.indexFile(file_path);
            if (doc_id == 0) // indexer 决定不索引此文档 --> 文档为空或者文档类型被过滤
                continue;
            indexed_documents.emplace(file_path, doc_id);
        }

    }

    Database& db;
    Indexer indexer;

    mutable std::mutex paths_lock;
    // directory(/file)_path -> (file_path -> doc_id)
    std::unordered_map<std::string, FileToDocId> paths;
};