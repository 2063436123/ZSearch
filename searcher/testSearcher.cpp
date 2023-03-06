#include "../typedefs.h"
#include "Searcher.h"
#include "indexer/Indexer.h"

TEST(Trie, you)
{
    Database db(ROOT_PATH + "/database1", true);
    Indexer indexer(db);
    indexer.index(ROOT_PATH + "/articles");
    Searcher searcher(db);
    ASSERT_NO_THROW(searcher.search("yo"));
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}