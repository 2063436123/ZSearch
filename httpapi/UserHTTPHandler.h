#pragma once

#include "../typedefs.h"
#include "HTTPHandler.h"

std::string encrypt(std::string id)
{
    std::reverse(id.begin(), id.end());
    return id;
}

std::string decrypt(std::string id)
{
    std::reverse(id.begin(), id.end());
    return id;
}

class LoginHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        // 获取 GET/POST 方法的参数
        Poco::Net::HTMLForm form(request, request.stream());

        if (!form.has("username") || !form.has("password"))
        {
            httpLog("login format failed.");
            response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
            response.send();
            return;
        }

        std::string username = form.get("username"), password = form.get("password");
        // TODO: validation
        if (username != "admin" || password != "admin")
        {
            httpLog("login - " + username + " " + password + " failed.");
            response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
            response.send();
            return;
        }
        httpLog("login - " + username + " " + password + " success.");
//        response.redirect("http://114.116.103.123:9090/app.html?id=" + encrypt(username));
        auto& out = makeResponseOK(response);
        out << "http://114.116.103.123:9090/app.html?id=" + encrypt(username);
    }
};


class IllegalAccessHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        std::string content_type = "text/html;charset=UTF-8";

        httpLog("illegalAccess");
        auto& out = makeResponseOK(response, content_type);

        out << makeStandardResponse(-1, IllegalAccessMessage, nlohmann::json::object());
    }
};