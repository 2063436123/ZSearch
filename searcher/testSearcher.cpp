#include "../typedefs.h"
#include "Searcher.h"
#include "indexer/Indexer.h"

TEST(Searcher, base)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");
    Searcher searcher(db);

    auto res = searcher.search("hello");
    ASSERT_EQ(res.size(), 0);
}


TEST(Trie, you)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");
    Searcher searcher(db);
    ASSERT_NO_THROW(searcher.search("yo"));
}

TEST(DocumentFreq, base)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");
    Searcher searcher(db);
    searcher.search("y");
    searcher.search("you");
    searcher.search("yo");
    searcher.search("y");
    searcher.search("yo");
    searcher.search("you");
    db.addDocumentDownloadFreq(8);
    db.addDocumentDownloadFreq(8);
    db.addDocumentDownloadFreq(4);

    auto stat_map = db.getAllDocumentFreq();
    auto compareFreq = [](std::pair<uint64_t, uint64_t> a, std::pair<uint64_t, uint64_t> b) {
        if (a.first != b.first)
            return a.first > b.first;
        return a.second > b.second;
    };
    // (download_freq, query_freq) -> doc_id
    std::multimap<std::pair<uint64_t, uint64_t>, size_t, decltype(compareFreq)> freq_to_documents(compareFreq);
    for (const auto&[doc_id, freq] : stat_map)
        freq_to_documents.emplace(std::make_tuple(freq.first, freq.second), doc_id);

    ASSERT_EQ(freq_to_documents.size(), 5);
    ASSERT_EQ(freq_to_documents.begin()->first, std::make_pair(2ull, 4ull));
    ASSERT_EQ((++freq_to_documents.begin())->first, std::make_pair(1ull, 4ull));
    ASSERT_EQ((++(++freq_to_documents.begin()))->first, std::make_pair(0ull, 16ull));
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}