#include "HTTPHandlerFactory.h"
#include <iostream>

using namespace Poco::Net;

void run()
{
    std::unordered_map<std::string, std::pair<DatabasePtr, FileSystemDaemonPtr>> user_database;
    FileSystemDaemons daemons;

    for (const auto& [username, _] : USERNAME_PASSWORDS)
    {
        auto db = std::make_shared<Database>(ROOT_PATH + "/database/" + username, false);

        auto daemon = std::make_shared<FileSystemDaemon>(*db);
        daemons.add(daemon);

        user_database.emplace(username, std::make_pair(db, daemon));
    }

    // 启动 Daemon
    Poco::Timer daemon_timer(0, DAEMON_INTERVAL_SECONDS * 1000);
    Poco::TimerCallback<FileSystemDaemons> callback(daemons, &FileSystemDaemons::run);
    daemon_timer.start(callback);

    // 注册 http 服务
    HTTPServer server(new HTTPHandlerFactory(user_database), ServerSocket(8080), new HTTPServerParams);
    server.start();

    std::string instruction;
    while (std::cin >> instruction)
    {
        if (instruction == "stop")
            break;
    }
    server.stopAll(true);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "usage: ./server <articles_path> <frontend_path>" << std::endl;
        exit(1);
    }

    ROOT_PATH = argv[1];
    RESOURCE_PATH = argv[2];

    try
    {
        run();
    } catch (Poco::Exception &e)
    {
        std::cout << "exception " << e.what() << " " << e.message() << std::endl;
    }
}

