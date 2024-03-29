#pragma once
#include "../typedefs.h"
#include "HTTPHandler.h"
#include "IndexHTTPHandler.h"
#include "SearchHTTPHandler.h"
#include "UserHTTPHandler.h"
#include "StatisticsHTTPHandler.h"
#include "DocumentHTTPHandler.h"

using DatabasePtr = std::shared_ptr<Database>;
using FileSystemDaemonPtr = std::shared_ptr<FileSystemDaemon>;
class HTTPHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    HTTPHandlerFactory(std::unordered_map<std::string, std::pair<DatabasePtr, FileSystemDaemonPtr>> user_data_) : user_data(std::move(user_data_)) {}

    Poco::Net::HTTPRequestHandler * createRequestHandler(const Poco::Net::HTTPServerRequest &request) override
    {
        // some useful: https://stackoverflow.com/questions/13386837/get-url-params-with-poco-library
        if (request.getMethod() != "GET" && request.getMethod() != "POST" && request.getMethod() != "DOWNLOAD")
        {
            httpLog("unsupported method: " + request.getMethod());
            THROW(Poco::NotImplementedException());
        }

        // get /path
        std::string uri_path = Poco::URI(request.getURI()).getPath();

        if (uri_path == "/login")
        {
            return new LoginHandler();
        }

        // get user id
        Poco::Net::HTMLForm form(request);
        auto id_iter = form.find("id");
        if (id_iter == form.end())
            return new GetFileHandler();
        std::string id = decrypt(id_iter->second);

        if (!user_data.contains(id))
        {
            return new IllegalAccessHandler();
        }

        Database &db = *user_data[id].first;
        FileSystemDaemon& daemon = *user_data[id].second;


        if (uri_path == "/")
        {
            return new HelloHandler();
        }
        if (uri_path == "/add-index")
        {
            return new AddIndexHandler(daemon);
        }
        if (uri_path == "/remove-index")
        {
            return new RemoveIndexHandler(daemon);
        }
        if (uri_path == "/get-all-index")
        {
            return new GetAllIndexHandler(daemon);
        }
        if (uri_path == "/rebuild-all-index")
        {
            return new RebuildAllIndexHandler(daemon);
        }
        if (uri_path == "/get-index-info")
        {
            return new GetIndexInfoHandler(daemon);
        }
        if (uri_path == "/start-query")
        {
            return new StartQueryHandler(db);
        }
        if (uri_path == "/download-document")
        {
            return new DownloadDocumentHandler(db);
        }
        if (uri_path == "/get-type-statistics")
        {
            return new GetTypeStatisticsHandler(daemon);
        }
        if (uri_path == "/get-query-statistics")
        {
            return new GetQueryStatisticsHandler(db);
        }
        if (uri_path == "/get-document-freq-statistics")
        {
            return new GetDocumentFreqStatisticsHandler(db);
        }
        if (uri_path == "/get-document-comment-rating")
        {
            return new GetDocumentCommentRatingHandler(db);
        }
        if (uri_path == "/score-document")
        {
            return new ScoreDocumentHandler(db, id);
        }
        if (uri_path == "/comment-document")
        {
            return new CommentDocumentHandler(db, id);
        }
        if (uri_path == "/get-document-property")
        {
            return new GetDocumentPropertyHandler(db);
        }
        return new GetFileHandler();
    }

private:
    std::unordered_map<std::string, std::pair<DatabasePtr, FileSystemDaemonPtr>> user_data;
};