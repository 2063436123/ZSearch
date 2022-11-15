#include "Database.h"
#include <fcntl.h>

TEST(database, CreateDatabase)
{
    ASSERT_THROW(Database::createDatabase(root_path + "/database1"), Poco::CreateFileException);
}

TEST(database, NewDocId)
{
    Database db(root_path + "/database1", true);

    // multi thread
    bool doc_ids[100001]{false, };
    auto f = [&db, &doc_ids]() {
        for (int i = 0; i < 10000; i++)
        {
            doc_ids[db.newDocId()] = true;
        }
    };

    for (int i = 0; i < 10; i++)
    {
        std::thread thread(f);
        thread.detach();
    }

    sleep(1);
    for (int i = 1; i <= 100000; i++)
    {
        ASSERT_EQ(doc_ids[i], true) << i;
    }

    // single thread
    for (int i = 100001; i < 200000; i++)
        ASSERT_EQ(i, db.newDocId());
}

TEST(database, AddFind)
{
    Database db(root_path + "/database1", true);

    db.addDocument(1, root_path + "/articles/ABC.txt");
    ASSERT_THROW(db.addDocument(2, root_path + "/articles/NotFound.txt"), FileTypeUnmatchException);
    ASSERT_THROW(db.addDocument(3, root_path + "/articles"), FileTypeUnmatchException);

    auto doc1 = db.findDocument(1);
    ASSERT_EQ(doc1.getId(), 1);
    ASSERT_EQ(doc1.getPath(), root_path + "/articles/ABC.txt");

    // "hello" occur twice in doc1, occur once in doc3
    db.addTerm("hello", 3, 100);
    db.addTerm("hello", 1, 1);
    db.addTerm("hello", 1, 30);

    auto term1 = db.findTerm("hello");
    ASSERT_EQ(term1.word, "hello");

    // 注意 term 一定按照 doc_id 升序排序存储: posting_list[0] < posting_list[1]
    ASSERT_EQ(term1.posting_list.size(), 2);
    ASSERT_EQ(term1.posting_list[0], 1);
    ASSERT_EQ(term1.posting_list[1], 3);

    ASSERT_EQ(term1.statistics_list.size(), 2);
    ASSERT_EQ(term1.statistics_list[0].offsets_in_file.size(), 2);
    ASSERT_EQ(term1.statistics_list[1].offsets_in_file.size(), 1);

    ASSERT_EQ(std::accumulate(term1.statistics_list[0].offsets_in_file.begin(), term1.statistics_list[0].offsets_in_file.end(), 0), 31);
    ASSERT_EQ(*term1.statistics_list[1].offsets_in_file.begin(), 100);

    // not found
    auto term2 = db.findTerm("world");
    ASSERT_EQ(term2.isEmpty(), true);
}

TEST(document, GetString)
{
    Document document(1, root_path + "/articles/WhenYouAreOld.txt");
    ASSERT_EQ(document.getString(0, 10, 20), "When you are old and");
    ASSERT_EQ(document.getString(180, 10, 2), ";\n\nHow man");
    ASSERT_EQ(document.getString(510, 10, 20), "d a crowd of stars.\n");
    ASSERT_EQ(document.getString(520, 10, 20), "d a crowd of stars.\n");

    Document document2(2, root_path + "/articles/Little.txt");
    ASSERT_EQ(document2.getString(0, 100, 100), "This is a little text.");
    ASSERT_EQ(document2.getString(0, 0, 100), "This is a little text.");
    ASSERT_EQ(document2.getString(0, 100, 0), "This is a little text.");
}

TEST(database, SerializeAndDeserialize)
{
    {
        // store
        Database db(root_path + "/database1", true);
        db.addDocument(1, root_path + "/articles/ABC.txt");
        db.addTerm("hello", 3, 100);
        db.addTerm("hello", 1, 1);
        db.addTerm("hello", 1, 30);
    }

    // restore
    Database db(root_path + "/database1", false);
    auto doc1 = db.findDocument(1);
    ASSERT_EQ(doc1.getId(), 1);
    ASSERT_EQ(doc1.getPath(), root_path + "/articles/ABC.txt");

    auto term1 = db.findTerm("hello");
    ASSERT_EQ(term1.word, "hello");

    ASSERT_EQ(term1.posting_list.size(), 2);
    ASSERT_EQ(term1.posting_list[0], 1);
    ASSERT_EQ(term1.posting_list[1], 3);

    ASSERT_EQ(term1.statistics_list.size(), 2);
    ASSERT_EQ(term1.statistics_list[0].offsets_in_file.size(), 2);
    ASSERT_EQ(term1.statistics_list[1].offsets_in_file.size(), 1);

    ASSERT_EQ(std::accumulate(term1.statistics_list[0].offsets_in_file.begin(), term1.statistics_list[0].offsets_in_file.end(), 0), 31);
    ASSERT_EQ(*term1.statistics_list[1].offsets_in_file.begin(), 100);
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}