#include "indexer/indexer.h"
#include "searcher/Searcher.h"
#include <iostream>

void run()
{
    std::string cmd;
    while (std::cin >> cmd)
    {
        if (cmd.starts_with("index"))
        {
            std::string path;
            std::cin >> path;
            Database db(path, true);
            Indexer indexer(db);

            std::cin >> path;
            indexer.index(path);
        }
        else if (cmd.starts_with("search"))
        {
            std::string path;
            std::cin >> path;
            Database db(path);

            Searcher searcher(db);

            std::string query;
            std::cin >> query;
            SearchResultSet res = searcher.search(query);
            size_t no = 0;
            for (const auto &result: res)
            {
                std::cout << "[No." << no++ << " doc id:" << result.document.getId() << " path:"
                          << result.document.getPath() << " score: " << result.score << "]" << std::endl;
                auto matched_text = result.document.getString(result.offset_in_file, result.related_text_len, 30);
                std::cout << outputSmooth(matched_text) << std::endl;
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

