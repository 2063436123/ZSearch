#include "HTTPHandlerFactory.h"
#include <iostream>

using namespace Poco::Net;
void run()
{
    Database db(ROOT_PATH + "/database1", true);
    FileSystemDaemon daemon(db);

    HTTPServer server(new HTTPHandlerFactory(db, daemon), ServerSocket(8080), new HTTPServerParams);
    server.start();

    sleep(10000);
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

