#pragma once

#include "../typedefs.h"
#include "HTTPHandler.h"
#include "searcher/Searcher.h"

class StartQueryHandler : public HTTPRequestHandler
{
public:
    StartQueryHandler(Database &db_) : db(db_) {}

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

        auto iter = form.find("query");
        if (iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());;
            return;
        }
        std::string query = iter->second;

        httpLog("starting query - " + query);

        // TODO: 执行查询并获取结果
        SearchResultSet result_set = Searcher(db).search(query);
        nlohmann::json::array_t data;
        for (const auto& result : result_set)
            data.push_back(nlohmann::json(result));

        out << makeStandardResponse(0, SuccessMessage, nlohmann::json({{"results", data}}));
    }

private:
    Database &db;
};
