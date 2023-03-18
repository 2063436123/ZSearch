#pragma once

class QueryStatistics;

using QueryStatisticsPtr = std::shared_ptr<QueryStatistics>;
using QueryStatisticsMap = std::multimap<DateTime, QueryStatisticsPtr>;

struct QueryStatistics
{
    QueryStatistics(const std::string &query_, double elapsed_time_, uint64_t result_num_)
            : query(query_),
              elapsed_time(elapsed_time_),
              result_num(result_num_) {}

    std::string query;
    double elapsed_time; // Milliseconds
    uint64_t result_num;
};
