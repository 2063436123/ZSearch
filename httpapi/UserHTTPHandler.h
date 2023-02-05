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
        auto &out = makeResponseOK(response);

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);

        // 获取 POST 方法的参数
        auto len = request.getContentLength();
        if (len > 0)
        {
            char *buf = new char[len];
            request.stream().read(buf, len);
            nlohmann::json data = nlohmann::json::parse(std::string(buf, len));
            if (data.is_object())
            {
                for (auto iter = data.begin(); iter != data.end(); ++iter)
                {
                    form.add(iter.key(), iter.value());
                }
            }
        }

        std::string username, password;

        auto iter = form.find("username");
        if (iter == form.end())
        {
            httpLog("login failed.");
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());;
            return;
        }
        username = iter->second;

        iter = form.find("password");
        if (iter == form.end())
        {
            httpLog("login - " + username + " failed.");
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());;
            return;
        }
        password = iter->second;

        // TODO: validation
        if (username != "admin" || password != "admin")
        {
            httpLog("login - " + username + " " + password + " failed.");
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }
        httpLog("login - " + username + " " + password + " success.");

        nlohmann::json::object_t data;
        data["id"] = encrypt(username);
        out << makeStandardResponse(0, SuccessMessage, data);
    }

};
