#pragma once

#include "../typedefs.h"
#include "HTTPHandler.h"

class GetTypeStatisticsHandler : public HTTPRequestHandler
{
public:
    GetTypeStatisticsHandler(FileSystemDaemon &daemon_) : daemon(daemon_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        httpLog("getTypeStatistics");

        std::unordered_map<std::string, uint64_t> stats = daemon.getTypeStatistics();
        nlohmann::json::array_t data;
        for (const auto& [name, value] : stats)
        {
            data.push_back(std::unordered_map<std::string, nlohmann::json>{{"name", name}, {"value", value}});
        }
        out << makeStandardResponse(0, SuccessMessage, {{"results", data}});
    }

private:
    FileSystemDaemon &daemon;
};

class GetQueryStatisticsHandler : public HTTPRequestHandler
{
public:
    GetQueryStatisticsHandler(Database &db_) : db(db_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        httpLog("getQueryStatistics");

        QueryStatisticsMap stat_map = db.getAllQueryStatistics();

        nlohmann::json::array_t x_data, query_array, elapsed_time_array, result_num_array;
        for (const auto& [time, stat] : stat_map)
        {
            x_data.push_back(time.string(true));
            query_array.push_back(stat->query);
            elapsed_time_array.push_back(stat->elapsed_time);
            result_num_array.push_back(stat->result_num);
        }
        out << makeStandardResponse(0, SuccessMessage,
                                    {{"x_strings", x_data}, {"query_vals", query_array}, {"elapsed_time_vals", elapsed_time_array}, {"result_num_vals", result_num_array}});
    }

private:
    Database &db;
};