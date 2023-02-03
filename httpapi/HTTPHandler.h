#pragma once
#include "../typedefs.h"
#include "daemon/Daemon.h"

using namespace Poco::Net;

const char * InvalidParameterMessage = R"(请求失败-参数错误)";
const char * NotFoundMessage = R"(你来到了没有知识的荒原)";
const char * SuccessMessage = R"(请求成功)";

template <typename T>
void httpLog(const T& msg)
{
    static std::mutex log_lock;
    std::lock_guard lg(log_lock);
    std::cout << '[' << DateTime().string(true) << "] " << msg << std::endl;
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
            char *buf = new char[request.getContentLength()];
            in.read(buf, request.getContentLength());
            std::cout << buf << std::endl;
        }

        // 可以获取 request header 内容
        // std::cout << request.get("Accept") << std::endl;

        // 可以获取 Method 和 URI
//        std::cout << request.getMethod() + ' ' + request.getURI() << std::endl;

        // 可以获取 GET 方法的参数
//        Poco::Net::HTMLForm form1(request);
//        std::cout << "get test " << form1.find("test")->second << std::endl;

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/html;charset=UTF-8");
        auto& out = response.send();
        out << R"(hello world)";
    }
};

class NotFoundHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        // NOTE: setStatus、setContentType 应该发生在 send 之前
        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/html;charset=UTF-8");

        auto& out = response.send();
        out << NotFoundMessage;
    }
};