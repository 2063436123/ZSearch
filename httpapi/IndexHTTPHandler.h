#pragma once
#include "../typedefs.h"
#include "HTTPHandler.h"

class AddIndexHandler : public HTTPRequestHandler
{
public:
    AddIndexHandler(FileSystemDaemon& daemon_) : daemon(daemon_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/html;charset=UTF-8");

        auto& out = response.send();

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);
        auto iter = form.find("path");
        if (iter == form.end())
        {
            out << InvalidParameterMessage;
            return;
        }

        httpLog("addPath - " + iter->second);
        daemon.addPath(iter->second);

        out << SuccessMessage;
    }

private:
    FileSystemDaemon& daemon;
};

class RemoveIndexHandler : public HTTPRequestHandler
{
public:
    RemoveIndexHandler(FileSystemDaemon& daemon_) : daemon(daemon_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/html;charset=UTF-8");

        auto& out = response.send();

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);
        auto iter = form.find("path");
        if (iter == form.end())
        {
            out << InvalidParameterMessage;
            return;
        }

        httpLog("removePath - " + iter->second);
        daemon.removePath(iter->second);

        out << SuccessMessage;
    }

private:
    FileSystemDaemon& daemon;
};
