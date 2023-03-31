#pragma once

#include "../typedefs.h"
#include "HTTPHandler.h"
#include <iomanip>
#include <sstream>

class GetDocumentPropertyHandler : public HTTPRequestHandler
{
public:
    GetDocumentPropertyHandler(Database& db_) : db(db_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);

        auto iter = form.find("doc_id");
        if (iter == form.end())
        {
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
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        auto document_ptr = db.findDocument(doc_id);
        if (!document_ptr)
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        std::string word_count_str = std::to_string(document_ptr->getWordCount());
        std::string kvs_count_str = std::to_string(document_ptr->getKvs().size());
        std::string size_str = prettyByteSize(document_ptr->getFileByteSize(), 2);
        std::string mtime_str = document_ptr->getModifyTime().string(true);

        out << makeStandardResponse(0, SuccessMessage, {{"word_count", word_count_str}, {"kvs_count", kvs_count_str}, {"size", size_str}, {"mtime", mtime_str}});
    }

private:
    static std::string prettyByteSize(size_t byte_size, int float_number_after_point)
    {
        auto limitPrecision = [float_number_after_point](double number) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(float_number_after_point) << number;
            return ss.str();
        };

        if (byte_size >= 1024 * 1024)
            return limitPrecision(1.0 * byte_size / 1024 / 1024) + "MB";
        if (byte_size >= 1024)
            return limitPrecision(1.0 * byte_size / 1024) + "KB";
        return std::to_string(byte_size) + "B";
    }

    Database &db;
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

        db.addDocumentDownloadFreq(doc_id);
        auto file_path = document_ptr->getPath();
        std::ifstream file(file_path.string());
        if (!file.is_open())
        {
            auto& out = makeResponseOK(response, content_type);
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
        }
        else
        {
            response.set("Content-Disposition", "attachment; filename=\"" + document_ptr->getPath().filename().string() + "\"");
            response.setContentType("application/json; charset=UTF-8");
            response.setStatus(HTTPResponse::HTTP_OK);
            response.setContentLength(file_size(file_path));
            auto& out = response.send();

            Poco::StreamCopier::copyStream(file, out);
        }
    }

private:
    Database &db;
};

class GetDocumentCommentRatingHandler : public HTTPRequestHandler
{
public:
    GetDocumentCommentRatingHandler(Database& db_) : db(db_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);

        auto iter = form.find("doc_id");
        if (iter == form.end())
        {
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
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        httpLog("getDocumentCommentRating - " + iter->second);

        auto document_ptr = db.findDocument(doc_id);
        if (!document_ptr)
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        nlohmann::json::array_t comments_data;
        for (const auto& [user_name, comment_pair] : document_ptr->getComments())
        {
            comments_data.push_back(nlohmann::json::object_t{});
            comments_data.back()["user_name"] = user_name;
            comments_data.back()["comment_time"] = comment_pair.first.string(true);
            comments_data.back()["comment"] = comment_pair.second;
        }

        auto [people_number, avg_rating] = document_ptr->getRatingStat();

        out << makeStandardResponse(0, SuccessMessage, {{"comments", comments_data}, {"people_number", people_number}, {"avg_rating", avg_rating}});
    }

private:
    Database &db;
};

class ScoreDocumentHandler : public HTTPRequestHandler
{
public:
    ScoreDocumentHandler(Database& db_, const std::string& user_id_) : db(db_), user_id(user_id_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);
        auto doc_id_iter = form.find("doc_id");
        if (doc_id_iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }
        auto rating_iter = form.find("rating");
        if (rating_iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        size_t doc_id;
        double rating;
        try
        {
            doc_id = restrictStoi<size_t>(doc_id_iter->second);
            rating = restrictStod(rating_iter->second);
        }
        catch (Poco::InvalidArgumentException& e)
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        auto document_ptr = db.findDocument(doc_id);
        if (!document_ptr)
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        document_ptr->setRating(user_id, rating);
        out << makeStandardResponse(0, SuccessMessage, nlohmann::json::object());
    }

private:
    Database &db;
    std::string user_id;
};

class CommentDocumentHandler : public HTTPRequestHandler
{
public:
    CommentDocumentHandler(Database& db_, const std::string& user_id_) : db(db_), user_id(user_id_) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        auto &out = makeResponseOK(response);

        // 获取 GET 方法的参数
        Poco::Net::HTMLForm form(request);

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

        auto doc_id_iter = form.find("doc_id");
        if (doc_id_iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }
        auto comment_iter = form.find("comment");
        if (comment_iter == form.end())
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        std::string comment = comment_iter->second;
        size_t doc_id;
        try
        {
            doc_id = restrictStoi<size_t>(doc_id_iter->second);
        }
        catch (Poco::InvalidArgumentException& e)
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        httpLog("addComment -- " + comment);

        auto document_ptr = db.findDocument(doc_id);
        if (!document_ptr)
        {
            out << makeStandardResponse(-1, InvalidParameterMessage, nlohmann::json::object());
            return;
        }

        document_ptr->addComment(user_id, DateTime(), comment);
        out << makeStandardResponse(0, SuccessMessage, nlohmann::json::object());
    }

private:
    Database &db;
    std::string user_id;
};
