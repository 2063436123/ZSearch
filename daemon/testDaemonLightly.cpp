#include "Daemon.h"

TEST(Daemon, gatherExistedFiles)
{
    auto files = gatherExistedFiles(ROOT_PATH + "/articles");
    std::unordered_set<std::string> distinct_files;
    for (const auto& file : files)
        distinct_files.insert(file);

    std::unordered_set<std::string> expected_distinct_files{"/Users/peter/Code/GraduationDesignSrc/master/articles/tpch-json/tpch.sh",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/WhatCanIHoldYouWith.txt",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/Little.txt",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/ABC.txt",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/WhenYouAreOld.txt",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/IfIWereToFallInLove.txt",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/single-jsons/webapp.json",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/single-jsons/glossary.json",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/tpch-json/tpch.json",
                                                            "/Users/peter/Code/GraduationDesignSrc/master/articles/single-jsons/css.json"};
    EXPECT_EQ(distinct_files, expected_distinct_files);
}

TEST(Daemon, gatherExistedFiles2)
{
    auto files = gatherExistedFiles(ROOT_PATH + "/articles-cnn");
    EXPECT_EQ(files.size(), 6015);

    std::unordered_set<std::string> distinct_files;
    for (const auto& file : files)
    {
        distinct_files.insert(file);
    }

    EXPECT_EQ(distinct_files.size(), 6015);
}

TEST(Daemon, gatherExistedFiles3)
{
    auto files = gatherExistedFiles(ROOT_PATH + "/articles/IfIWereToFallInLove.txt");
    std::unordered_set<std::string> distinct_files;
    for (const auto& file : files)
        distinct_files.insert(file);

    std::unordered_set<std::string> expected_distinct_files{"/Users/peter/Code/GraduationDesignSrc/master/articles/IfIWereToFallInLove.txt"};
    EXPECT_EQ(distinct_files, expected_distinct_files);
}

TEST(Daemon, gatherExistedFilesEmpty)
{
    auto files = gatherExistedFiles(ROOT_PATH + "/articles/tmp");
    EXPECT_EQ(files.empty(), true);
}

TEST(Daemon, bigFiles)
{
    Database db(ROOT_PATH + "/database1", true);
    FileSystemDaemon file_system_daemon(db);

    const int interval_seconds = 1;

    Poco::Timer timer(0, interval_seconds * 1000);
    Poco::TimerCallback<FileSystemDaemon> callback(file_system_daemon, &FileSystemDaemon::run);
    timer.start(callback);

    // 测试初始化
    file_system_daemon.addPath(ROOT_PATH + "/articles-cnn");
    sleep(interval_seconds * 3);
    EXPECT_EQ(db.maxAllocatedDocId(), 5000);

    EXPECT_EQ(file_system_daemon.getPaths()[ROOT_PATH + "/articles-cnn"].size(), 5000);
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}