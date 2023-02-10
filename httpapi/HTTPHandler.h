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
            content_type = "text/css; charset=utf-8";
        if (uri_path.ends_with(".js"))
            content_type = "text/javascript; charset=utf-8";

        httpLog("getFile " + uri_path + " content-type=" + content_type);

        if (std::filesystem::path(uri_path).is_relative())
        {
            response.redirect("http://114.116.103.123:9090/notfound.html");
            return;
        }

        std::ifstream file(RESOURCE_PATH + uri_path);
        if (!file.is_open())
        {
            response.redirect("http://114.116.103.123:9090/notfound.html");
        }
        else
        {
            auto& out = makeResponseOK(response, content_type);
            Poco::StreamCopier::copyStream(file, out);
        }
    }
};

class DownloadDocumentHandler : public HTTPRequestHandler
{
public:
    DownloadDocumentHandler(Database& db_) : db(db_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        std::string content_type = "text/html;charset=UTF-8";

        Poco::Net::HTMLForm form(request);
        auto iter = form.find("doc_id");
        if (iter == form.end())
        {
            auto& out = makeResponseOK(response, content_type);
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        size_t doc_id;
        try
        {
            doc_id = restrictStoi<size_t>(iter->second);
        }
        catch (Poco::InvalidArgumentException& e)
        {
            auto& out = makeResponseOK(response, content_type);
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }
        httpLog("downloadFile " + std::to_string(doc_id));

        auto document_ptr = db.findDocument(doc_id);
        if (!document_ptr)
        {
            auto& out = makeResponseOK(response, content_type);
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        std::ifstream file(document_ptr->getPath().string());
        if (!file.is_open())
        {
            auto& out = makeResponseOK(response, content_type);
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
        }
        else
        {
            response.set("Content-Disposition", "attachment; filename=\"" + document_ptr->getPath().filename().string() + "\"");
            response.setStatus(HTTPResponse::HTTP_OK);
            auto& out = response.send();
            Poco::StreamCopier::copyStream(file, out);
        }
    }

private:
    Database &db;
};