#include "Database.h"
#include "Value.h"
#include "indexer/Indexer.h"
#include <fcntl.h>

TEST(database, CreateDatabase)
{
    ASSERT_THROW(Database::createDatabase(ROOT_PATH + "/database1"), Poco::CreateFileException);

    try
    {
        Database::createDatabase(ROOT_PATH + "/database1");
    } catch (const Poco::Exception &e)
    {
        std::stringstream oss;
        oss << e.what() << " " << e.message();
        ASSERT_EQ(oss.str().substr(0, 193),
                  "Cannot create file <message - can't create database directory in /Users/peter/Code/GraduationDesignSrc/master/database1> "
                  "<location - /Users/peter/Code/GraduationDesignSrc/master/core/Database.h");
    }

    Database::destroyDatabase(ROOT_PATH + "/database1");
}

TEST(database, NewDocId)
{
    Database db(ROOT_PATH + "/database1", true);

    // multi thread
    bool doc_ids[100001]{false,};
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
    Database db(ROOT_PATH + "/database1", true);

    db.addDocument(1, ROOT_PATH + "/articles/ABC.txt", 0, {});
    ASSERT_THROW(db.addDocument(2, ROOT_PATH + "/articles/NotFound.txt", 0, {}), FileTypeUnmatchException);
    ASSERT_THROW(db.addDocument(3, ROOT_PATH + "/articles", 0, {}), FileTypeUnmatchException);

    auto doc1 = db.findDocument(1);
    ASSERT_EQ(doc1->getId(), 1);
    ASSERT_EQ(doc1->getPath(), ROOT_PATH + "/articles/ABC.txt");

    // "hello" occur twice in doc1, occur once in doc3
    db.addTerm("hello", 3, 100);
    db.addTerm("hello", 1, 1);
    db.addTerm("hello", 1, 30);

    auto term1 = db.findTerm("hello");
    assert(term1);
    ASSERT_EQ(term1->word, "hello");

    // 注意 term 一定按照 doc_id 升序排序存储: posting_list[0] < posting_list[1]
    ASSERT_EQ(term1->posting_list.size(), 2);
    ASSERT_EQ(term1->posting_list[0], 1);
    ASSERT_EQ(term1->posting_list[1], 3);

    ASSERT_EQ(term1->statistics_list.size(), 2);
    ASSERT_EQ(term1->statistics_list[0].offsets_in_file.size(), 2);
    ASSERT_EQ(term1->statistics_list[1].offsets_in_file.size(), 1);

    ASSERT_EQ(std::accumulate(term1->statistics_list[0].offsets_in_file.begin(),
                              term1->statistics_list[0].offsets_in_file.end(), 0), 31);
    ASSERT_EQ(*term1->statistics_list[1].offsets_in_file.begin(), 100);

    // not found
    auto term2 = db.findTerm("world");
    ASSERT_EQ(term2 == nullptr, true);
}

TEST(document, GetString)
{
    Document document(1, ROOT_PATH + "/articles/WhenYouAreOld.txt", 0, {});
    ASSERT_EQ(document.getString(0, 10, 20), "When you are old and");
    ASSERT_EQ(document.getString(180, 10, 2), ";\n\nHow man");
    ASSERT_EQ(document.getString(510, 10, 20), "d a crowd of stars.\n");
    ASSERT_EQ(document.getString(520, 10, 20), "d a crowd of stars.\n");

    Document document2(2, ROOT_PATH + "/articles/Little.txt", 0, {});
    ASSERT_EQ(document2.getString(0, 100, 100), "This is a little text.");
    ASSERT_EQ(document2.getString(0, 0, 100), "This is a little text.");
    ASSERT_EQ(document2.getString(0, 100, 0), "This is a little text.");
}

TEST(database, SerializeAndDeserialize)
{
    {
        // store
        Database db(ROOT_PATH + "/database1", true);
        db.addDocument(db.newDocId(), ROOT_PATH + "/articles/ABC.txt", 0, {});
        db.addTerm("hello", 3, 100);
        db.addTerm("hello", 1, 1);
        db.addTerm("hello", 1, 30);

        Indexer indexer(db);
        indexer.index(ROOT_PATH + "/articles/single-jsons/webapp.json");

        auto document_ptr = db.findDocument(2);
        auto kvs = document_ptr->getKvs();
    }

    // restore
    Database db(ROOT_PATH + "/database1", false);
    auto doc1 = db.findDocument(1);
    ASSERT_EQ(doc1->getId(), 1);
    ASSERT_EQ(doc1->getPath(), ROOT_PATH + "/articles/ABC.txt");

    auto term1 = db.findTerm("hello");
    ASSERT_EQ(term1->word, "hello");

    ASSERT_EQ(term1->posting_list.size(), 2);
    ASSERT_EQ(term1->posting_list[0], 1);
    ASSERT_EQ(term1->posting_list[1], 3);

    ASSERT_EQ(term1->statistics_list.size(), 2);
    ASSERT_EQ(term1->statistics_list[0].offsets_in_file.size(), 2);
    ASSERT_EQ(term1->statistics_list[1].offsets_in_file.size(), 1);

    ASSERT_EQ(std::accumulate(term1->statistics_list[0].offsets_in_file.begin(),
                              term1->statistics_list[0].offsets_in_file.end(), 0), 31);
    ASSERT_EQ(*term1->statistics_list[1].offsets_in_file.begin(), 100);

    auto document_ptr = db.findDocument(2);
    auto kvs = document_ptr->getKvs();
    EXPECT_EQ(kvs.size(), 76);

    EXPECT_EQ(kvs["web-app.servlet.0.init-param.useJSP"].as<Bool>(), false);
    EXPECT_EQ(kvs["web-app.servlet.0.init-param.jspListTemplate"].as<String>(), "listTemplate.jsp");
    EXPECT_EQ(kvs["web-app.servlet.2.servlet-name"].as<String>(), "cofaxAdmin");
    EXPECT_EQ(kvs["web-app.servlet.3.servlet-name"].as<String>(), "fileServlet");
    EXPECT_EQ(kvs["web-app.servlet.4.servlet-name"].as<String>(), "cofaxTools");
    EXPECT_EQ(kvs["web-app.taglib.taglib-uri"].as<String>(), "cofax.tld");

    auto barr = kvs["web-app.b-arr"];
    barr.doArrayHandler<Bool>([](std::vector<Bool>* vec) { ASSERT_EQ(vec->size(), 2); });
    auto iarr = kvs["web-app.i-arr"];
    iarr.doArrayHandler<Number>([](std::vector<Number>* vec) { ASSERT_EQ(vec->size(), 3); });

    EXPECT_EQ(barr.isArray(), true);
    EXPECT_EQ(barr.as<Bool>(0), true);
    EXPECT_EQ(barr.as<Bool>(1), false);
    EXPECT_EQ(iarr.isArray(), true);
    EXPECT_EQ(iarr.as<Number>(0), 100);
    EXPECT_EQ(iarr.as<Number>(1), 200);
    EXPECT_EQ(iarr.as<Number>(2), 300);
}

TEST(Value, base)
{
    Value v1;
    ASSERT_EQ(v1.isNull(), true);
    ASSERT_EQ(v1.getValueType(), ValueType::Null);

    Value v2("hello");
    ASSERT_EQ(v2.isArray(), false);
    ASSERT_EQ(v2.getValueType(), ValueType::String);

    Value v3(123);
    ASSERT_EQ(v3.getValueType(), ValueType::Number);
    ASSERT_EQ(v3.as<Number>(), 123);

    Value v4(34.6f);
    ASSERT_EQ(v4.getValueType(), ValueType::Number);

    Value v5(false);
    ASSERT_EQ(v5.getValueType(), ValueType::Bool);

    Value v6(DateTime("1930-12-22 05:12:33"));
    ASSERT_EQ(v6.getValueType(), ValueType::DateTime);

    Value v7(ArrayLabel{}, ValueType::String);
    ASSERT_EQ(v7.getValueType(), ValueType::String);
    ASSERT_EQ(v7.isArray(), true);

    v7.doArrayHandler(PanicValueArrayHandler<Bool>,
                      PanicValueArrayHandler<Number>,
                      [](std::vector<String> *vec) {
                          vec->push_back("hello");
                          vec->push_back("world");
                      },
                      PanicValueArrayHandler<DateTime>);

    size_t size = 0;
    v7.doArrayHandler(PanicValueArrayHandler<Bool>,
                      PanicValueArrayHandler<Number>,
                      [&size](std::vector<String> *vec) {
                          size = vec->size();
                      },
                      PanicValueArrayHandler<DateTime>);
    ASSERT_EQ(size, 2);

    EXPECT_NO_THROW(v7.doArrayHandler(EmptyValueArrayHandler<Bool>,
                                      EmptyValueArrayHandler<Number>,
                                      EmptyValueArrayHandler<String>,
                                      EmptyValueArrayHandler<DateTime>));

    EXPECT_THROW(v7.doArrayHandler(EmptyValueArrayHandler<Bool>,
                      EmptyValueArrayHandler<Number>,
                      PanicValueArrayHandler<String>,
                      EmptyValueArrayHandler<DateTime>),
                 UnreachableException);

    ASSERT_EQ(v7.as<String>(1), "world");
}

TEST(Key, base)
{
    {
        EXPECT_THROW(Key key1("hello..b"), Poco::LogicException);
        EXPECT_NO_THROW(Key key2(""));
        EXPECT_THROW(Key key3(".c"), Poco::LogicException);
    }
    Key key("a.b.c");
    EXPECT_EQ(key.string(), "a.b.c");
    EXPECT_EQ(*key.begin(), "a");
    EXPECT_EQ(*++key.begin(), "b");
    EXPECT_EQ(*--key.end(), "c");

    std::unordered_map<Key, int> map;
    map.emplace("a.c", 100);
    map.emplace("a.c", 200);
    map.emplace("a.b", 100);
    map.emplace("t.c", 300);

    EXPECT_EQ(map[Key("a.c")], 100);
    EXPECT_EQ(map[Key("a.b")], 100);
    EXPECT_EQ(map[Key("t.c")], 300);
}

TEST(keyValue, SerializeAndDeserialize)
{
    WriteBuffer wbuf;
    WriteBufferHelper whelper(wbuf);

    std::stringstream io;

    ReadBuffer rbuf;
    ReadBufferHelper rhelper(rbuf);
    auto handleKey = [&whelper, &wbuf, &rbuf, &rhelper, &io](const std::string& str) {
        Key key1(str);

        key1.serialize(whelper);
        wbuf.dumpAllToStream(io);

        rbuf.readAllFromStream(io);
        io.clear();
        Key key2 = Key::deserialize(rhelper);

        ASSERT_EQ(key1, key2);
    };

    handleKey("");
    handleKey("a");
    handleKey("a.b");
    handleKey("a.b.c");
    handleKey("falweijfioesa.af091823mrf120.sjfoicxfa6566");

    auto handleSingleValue = [&whelper, &wbuf, &rbuf, &rhelper, &io]<typename T>(const T& value) {
        Value value1(value);

        value1.serialize(whelper);
        wbuf.dumpAllToStream(io);

        rbuf.readAllFromStream(io);
        io.clear();
        Value value2 = Value::deserialize(rhelper);

        ASSERT_EQ(value1, value2);
    };

    handleSingleValue(true);
    handleSingleValue("hello");
    handleSingleValue(std::string("fu\0 cko"));
    handleSingleValue(123);
    handleSingleValue(523.452);
    handleSingleValue(DateTime("1921-12-12 23:45:59"));

    auto handleArrayValue = [&whelper, &wbuf, &rbuf, &rhelper, &io](const DynamicArray& da) {
        Value value1(ArrayLabel{}, da.getType(), da);

        value1.serialize(whelper);
        wbuf.dumpAllToStream(io);

        rbuf.readAllFromStream(io);
        io.clear();
        Value value2 = Value::deserialize(rhelper);

        ASSERT_EQ(value1, value2);
    };

    DynamicArray arr1(ValueType::String);
    arr1.applyHandler(PanicValueArrayHandler<Bool>, PanicValueArrayHandler<Number>,
            [](std::vector<String>* vec){
            vec->insert(vec->end(), {"hello", "word", "fkla", "xianxian"});
        }, PanicValueArrayHandler<DateTime>);
    handleArrayValue(arr1);

    DynamicArray arr2(ValueType::Number);
    arr2.applyHandler(PanicValueArrayHandler<Bool>,
            [](std::vector<Number>* vec){
        vec->insert(vec->end(), {10293.532, 123213, 3425945, 32421});
    }, PanicValueArrayHandler<String>, PanicValueArrayHandler<DateTime>);
    handleArrayValue(arr2);

    DynamicArray arr3(ValueType::DateTime);
    arr3.applyHandler(PanicValueArrayHandler<Bool>, PanicValueArrayHandler<Number>, PanicValueArrayHandler<String>,
                      [](std::vector<DateTime>* vec){
                          vec->insert(vec->end(), {DateTime("1253-12-12 12:43:12")});
                      });
    handleArrayValue(arr3);

    EXPECT_EQ(arr1.get<String>(3), "xianxian");
    EXPECT_EQ(arr2.get<Number>(2), 3425945);
    EXPECT_EQ(arr3.get<DateTime>(0).string(), "1253-12-12 12:43:12");
}

TEST(database, TidyTerm)
{
    Database db(ROOT_PATH + "/database1", true);

    db.addDocument(1, ROOT_PATH + "/articles/ABC.txt", 0, {});

    auto doc1 = db.findDocument(1);

    // "hello" occur twice in doc1, occur once in doc3
    db.addTerm("hello", 1, 1);
    db.addTerm("hello", 1, 30);

    auto term1 = db.findTerm("hello");
    EXPECT_EQ(term1.operator bool(), true);

    db.deleteDocument(1);
    auto term2 = db.findTerm("hello");
    EXPECT_EQ(term2->posting_list.size(), 1);
    EXPECT_EQ(term2->statistics_list.size(), 1);

    db.tidyTerm("hello");
    auto term3 = db.findTerm("hello");
    EXPECT_EQ(term3->posting_list.size(), 0);
    EXPECT_EQ(term3->statistics_list.size(), 0);
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}