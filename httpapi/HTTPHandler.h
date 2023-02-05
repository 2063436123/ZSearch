#pragma once
#include "../typedefs.h"
#include "daemon/Daemon.h"

using namespace Poco::Net;

const char * InvalidParameterMessage = R"(请求失败-参数错误)";
const char * NotFoundMessage = R"(你来到了没有知识的荒原)";
const char * IllegalAccessMessage = R"(非法访问，请登录)";
const char * SuccessMessage = R"(请求成功)";

std::string makeStandardResponse(int status, const std::string& msg, const nlohmann::json& data)
{
    assert(data.is_object());
    std::ostringstream oss;
    oss << "{"
        << R"("status": )"
        << status
        << ", "
        << R"("msg": ")"
        << msg
        << "\", "
        << R"("data": )"
        << to_string(data)
        << "}";
    return oss.str();
}

std::ostream& makeResponseOK(HTTPServerResponse &response, const std::string& content_type = "text/html;charset=UTF-8")
{
    // NOTE: setStatus、setContentType 应该发生在 send 之前
    response.setStatus(HTTPResponse::HTTP_OK);
    response.setContentType(content_type);
    response.set("Access-Control-Allow-Methods", "PUT, GET, HEAD, POST, DELETE, OPTIONS");
    response.set("Access-Control-Allow-Origin", "*");
    response.set("Access-Control-Allow-Headers", "*");
    response.set("Access-Control-Allow-Credentials", "true");

    return response.send();
}

class HelloHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto& in = request.stream();

        // 可以获取 body 内容
        auto len = request.getContentLength();
        if (len > 0)
        {
            char *buf = new char[len];
            in.read(buf, len);
            std::cout << std::string(buf, len) << std::endl;
        }

        // 可以获取 request header 内容
        // std::cout << request.get("Accept") << std::endl;

        // 可以获取 Method 和 URI
//        std::cout << request.getMethod() + ' ' + request.getURI() << std::endl;

        // 可以获取 GET 方法的参数
//        Poco::Net::HTMLForm form1(request);
//        std::cout << "get test " << form1.find("test")->second << std::endl;

        auto& out = makeResponseOK(response);
        out << R"(hello world)";
    }
};

class GetFileHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        std::string content_type = "text/html;charset=UTF-8";
        auto uri_path = Poco::URI(request.getURI()).getPath();
        if (uri_path.ends_with(".css"))
            content_type = "text/css";

        httpLog("getFile " + uri_path + " content-type=" + content_type);
        auto& out = makeResponseOK(response, content_type);

        std::ifstream page1(RESOURCE_PATH + uri_path);
        if (!page1.is_open())
            out << makeStandardResponse(-1, NotFoundMessage, nlohmann::json::object());
        else
            Poco::StreamCopier::copyStream(page1, out);
    }
};