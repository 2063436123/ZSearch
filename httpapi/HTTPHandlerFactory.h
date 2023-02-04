#pragma once
#include "../typedefs.h"
#include "HTTPHandler.h"
#include "IndexHTTPHandler.h"

class HTTPHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    HTTPHandlerFactory(Database &db_, FileSystemDaemon& daemon_) : db(db_), daemon(daemon_) {}

    Poco::Net::HTTPRequestHandler * createRequestHandler(const Poco::Net::HTTPServerRequest &request) override
    {
        // some useful: https://stackoverflow.com/questions/13386837/get-url-params-with-poco-library

        if (request.getMethod() != "GET" && request.getMethod() != "POST")
        {
            httpLog("unsupported method: " + request.getMethod());
            // 获取 POST 方法的参数
            // 待验证 Poco::Net::HTMLForm form2(request, request.stream());
            THROW(Poco::NotImplementedException());
        }
        std::string uri_path = Poco::URI(request.getURI()).getPath();

        if (uri_path == "/")
        {
            return new HelloHandler();
        }
        if (uri_path == "/add-index")
        {
            return new AddIndexHandler(daemon);
        }
        if (uri_path == "/remove-index")
        {
            return new RemoveIndexHandler(daemon);
        }
        if (uri_path == "/get-all-index")
        {
            return new GetAllIndexHandler(daemon);
        }
        if (uri_path == "/rebuild-all-index")
        {
            return new RebuildAllIndexHandler(daemon);
        }
        if (uri_path == "/get-index-info")
        {
            return new GetIndexInfoHandler(daemon);
        }
        return new GetFileHandler();
    }

private:
    Database &db;
    FileSystemDaemon& daemon;
};