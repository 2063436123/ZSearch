#pragma once

#include "../typedefs.h"
#include <random>
#include "HTTPHandler.h"

void addRandPiece(std::string& id, int piece_length = 5)
{
    auto generate_piece = [=]() {
        const std::string allowed_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, allowed_chars.size() - 1);

        std::string random_string(piece_length, ' ');
        for (int i = 0; i < random_string.length(); i++) {
            random_string[i] = allowed_chars[dis(gen)];
        }
        return random_string;
    };

    std::string res;
    for (char ch : id)
    {
        res += ch;
        res += generate_piece();
    }
    id = res;
}

void removePiece(std::string& id, int piece_length = 5)
{
    std::string res;
    for (size_t i = 0; i < id.size(); i++)
    {
        res += id[i];
        i += piece_length;
    }
    id = res;
}

std::string encrypt(std::string id)
{
    addRandPiece(id);
    std::reverse(id.begin(), id.end());
    return id;
}

std::string decrypt(std::string id)
{
    std::reverse(id.begin(), id.end());
    removePiece(id);
    return id;
}

bool validateUser(const std::string& username, const std::string& password)
{
    auto iter = USERNAME_PASSWORDS.find(UserAttribute(username, "", ""));
    if (iter != USERNAME_PASSWORDS.end() && iter->password == password)
        return true;
    return false;
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
        if (!validateUser(username, password))
        {
            httpLog("login - " + username + " " + password + " failed.");
            response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
            response.send();
            return;
        }

        httpLog("login - " + username + " " + password + " success.");
        auto& out = makeResponseOK(response);
        out << "http://localhost:8080/app.html?id=" + encrypt(username);
    }
};


class IllegalAccessHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        std::string content_type = "text/html;charset=UTF-8";

        httpLog("illegalAccess -- no such a user");
        auto& out = makeResponseOK(response, content_type);

        out << makeStandardResponse(-1, IllegalAccessMessage, nlohmann::json::object());
    }
};