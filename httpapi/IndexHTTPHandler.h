#pragma once

#include "../typedefs.h"
#include "HTTPHandler.h"

class AddIndexHandler : public HTTPRequestHandler
{
public:
    AddIndexHandler(FileSystemDaemon &daemon_) : daemon(daemon_) {}

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

        auto iter = form.find("path");
        if (iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());;
            return;
        }

        // 索引条目必须已经存在
        if (!exists(std::filesystem::path(iter->second)))
        {
            httpLog("addIndexPath - " + iter->second + " not existed.");
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        if (std::filesystem::path(iter->second).is_relative())
        {
            httpLog("addIndexPath - " + iter->second + " relative position not allowed.");
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }
        httpLog("addIndexPath - " + iter->second + " success.");

        daemon.addPath(iter->second);

        out << makeStandardResponse(0, SuccessMessage, nlohmann::json::object());
    }

private:
    FileSystemDaemon &daemon;
};

class RemoveIndexHandler : public HTTPRequestHandler
{
public:
    RemoveIndexHandler(FileSystemDaemon &daemon_) : daemon(daemon_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);
        auto iter = form.find("path");
        if (iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        httpLog("removeIndexPath - " + iter->second);
        daemon.removePath(iter->second);

        out << makeStandardResponse(0, SuccessMessage, nlohmann::json::object());
    }

private:
    FileSystemDaemon &daemon;
};

class GetAllIndexHandler : public HTTPRequestHandler
{
public:
    GetAllIndexHandler(FileSystemDaemon &daemon_) : daemon(daemon_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        httpLog("getAllIndex");

        nlohmann::json::array_t path_names;
        for (const auto &path : daemon.getPaths())
        {
            auto path_in_disk = std::filesystem::path(path.first);
            if (!exists(path_in_disk)) // 忽略已失效的索引条目
                continue;
            path_names.push_back(std::unordered_map<std::string, nlohmann::json>{{"path",            path.first},
                                                                                 {"document_number", path.second.size()},
                                                                                 {"mtime",           getModifiedLastDateTime(
                                                                                         path_in_disk).string(true)}});
        }

        nlohmann::json data(
                {{"paths", path_names}}
        );
        out << makeStandardResponse(0, SuccessMessage, data);
    }

private:
    FileSystemDaemon &daemon;
};

class RebuildAllIndexHandler : public HTTPRequestHandler
{
public:
    RebuildAllIndexHandler(FileSystemDaemon &daemon_) : daemon(daemon_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        httpLog("rebuildAllIndex");

        daemon.rebuildAllPaths();
        out << makeStandardResponse(0, SuccessMessage, nlohmann::json::object());
    }

private:
    FileSystemDaemon &daemon;
};

class GetIndexInfoHandler : public HTTPRequestHandler
{
public:
    GetIndexInfoHandler(FileSystemDaemon &daemon_) : daemon(daemon_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);
        auto iter = form.find("path");
        if (iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        httpLog("getIndexInfoPath - " + iter->second);
        std::unordered_map<std::string, size_t> path_infos = daemon.getPaths()[iter->second];

        // order by id
        std::map<size_t, std::string> ordered_paths_infos;
        for (const auto &path_info : path_infos) {
            ordered_paths_infos.emplace(path_info.second, path_info.first);
        }

        std::vector<nlohmann::json> data;
        for (const auto &path_info : ordered_paths_infos)
        {
            data.emplace_back(std::unordered_map<std::string, nlohmann::json>{{"id",   path_info.first},
                                                                              {"path", path_info.second}});
        }

        out << makeStandardResponse(0, SuccessMessage, std::unordered_map<std::string, nlohmann::json>{{"infos", data}});
    }

private:
    FileSystemDaemon &daemon;
};