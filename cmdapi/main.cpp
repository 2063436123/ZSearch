#include "core/indexer.h"
#include "searcher/searcher.h"
#include <iostream>

void run()
{
    std::string cmd;
    // TODO: 实现序列化反序列化方法
    Database db(root_path + "/database1");
    while (std::cin >> cmd)
    {
        if (cmd.starts_with("index"))
        {
            std::string path;
            std::cin >> path;
//            Database db(path);
            Indexer indexer(db);

            std::cin >> path;
            indexer.index(path);
        }
        else if (cmd.starts_with("search"))
        {
            std::string path;
            std::cin >> path;
//            Database db(path);

            Searcher searcher(db);

            std::string query;
            std::cin >> query;
            SearchResultSet res = searcher.search(query);
            size_t no = 0;
            for (const auto &result: res)
            {
                std::cout << "[No." << no++ << " doc id: " << result.document.getId() << " path: "
                          << result.document.getPath() << " score: " << result.score << "]" << std::endl;
                auto matched_text = result.document.getString(result.offset_in_file, result.related_text_len, 20);
                std::cout << output_smooth(matched_text) << std::endl;
            }
        }
        else if (cmd.starts_with("q"))
        {
            std::cout << "bye" << std::endl;
            exit(0);
        }
        else
        {
            std::cout << R"(usage:
                index <database_path> <file_path>
                search <database_path> <query>
                q)" << std::endl;
        }
    }
}

int main()
{
    try
    {
        run();
    } catch (Poco::Exception &e)
    {
        std::cout << "exception " << e.what() << " " << e.message() << std::endl;
    }
}

