#include "HTTPHandlerFactory.h"
#include <iostream>

using namespace Poco::Net;
void run()
{
    Database db(ROOT_PATH + "/database1", true);
    FileSystemDaemon daemon(db);

    HTTPServer server(new HTTPHandlerFactory({{"admin", {&db, &daemon}}}), ServerSocket(8080), new HTTPServerParams);
    server.start();

    Poco::Timer timer(0, DAEMON_INTERVAL_SECONDS * 1000);
    Poco::TimerCallback<FileSystemDaemon> callback(daemon, &FileSystemDaemon::run);
    timer.start(callback); // TODO: 启动 Daemon

    sleep(1000000);
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

