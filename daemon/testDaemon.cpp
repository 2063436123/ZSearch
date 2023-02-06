#include "Daemon.h"

TEST(Daemon, base)
{
    Database db(ROOT_PATH + "/database1", true);
    FileSystemDaemon file_system_daemon(db);

    const int interval_seconds = 1;
    const int valid_files_number = 10;

    Poco::Timer timer(0, interval_seconds * 1000);
    Poco::TimerCallback<FileSystemDaemon> callback(file_system_daemon, &FileSystemDaemon::run);
    timer.start(callback);

    // 测试初始化
    std::filesystem::remove_all(ROOT_PATH + "/articles/tmp"); // NOTE: gtest bug, 会过早多执行一遍 std::filesystem::create_directory(ROOT_PATH + "/articles/tmp")，需要事先 remove all
    file_system_daemon.addPath(ROOT_PATH + "/articles");
    sleep(interval_seconds * 3);
    EXPECT_EQ(db.maxAllocatedDocId(), valid_files_number);

    // 测试新文件
    ASSERT_EQ(std::filesystem::create_directory(ROOT_PATH + "/articles/tmp"), true);
    {
        std::ofstream fout1(ROOT_PATH + "/articles/tmp/hello.txt");
        fout1 << "hello world";
    }

    sleep(interval_seconds * 2);
    EXPECT_EQ(db.maxAllocatedDocId(), valid_files_number + 1);

    // 测试文件修改
    {
        std::ofstream fout2(ROOT_PATH + "/articles/tmp/hello.txt");
        fout2 << "fuck";
    }

    sleep(interval_seconds * 2);
    EXPECT_EQ(db.maxAllocatedDocId(), valid_files_number + 2);

    // 测试文件删除
    std::filesystem::remove(ROOT_PATH + "/articles/tmp/hello.txt");
    sleep(interval_seconds * 2);
    EXPECT_EQ(db.maxAllocatedDocId(), valid_files_number + 2);

    // 测试 removePath
    EXPECT_EQ(file_system_daemon.getPaths().size(), 1);
    file_system_daemon.removePath(ROOT_PATH + "/articles");
    sleep(interval_seconds * 2);
    EXPECT_EQ(file_system_daemon.getPaths().size(), 0);

    {
        std::ofstream fout3(ROOT_PATH + "/articles/tmp/world.txt");
        fout3 << "world hello";
    }

    sleep(interval_seconds * 2);
    std::filesystem::remove_all(ROOT_PATH + "/articles/tmp");

    EXPECT_EQ(db.maxAllocatedDocId(), valid_files_number + 2);
//    timer.stop();
}

TEST(Daemon, reset)
{
    Database db(ROOT_PATH + "/database1", true);
    FileSystemDaemon file_system_daemon(db);

    const int interval_seconds = 1;
    const int valid_files_number = 10;

    Poco::Timer timer(0, interval_seconds * 1000);
    Poco::TimerCallback<FileSystemDaemon> callback(file_system_daemon, &FileSystemDaemon::run);
    timer.start(callback);

    // 测试初始化
    file_system_daemon.addPath(ROOT_PATH + "/articles");
    sleep(interval_seconds * 2);
    EXPECT_EQ(db.maxAllocatedDocId(), valid_files_number);
    auto paths = file_system_daemon.getPaths();

    file_system_daemon.rebuildAllPaths();
    sleep(interval_seconds * 2);
    EXPECT_EQ(db.maxAllocatedDocId(), valid_files_number);
    EXPECT_EQ(paths, file_system_daemon.getPaths());
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}