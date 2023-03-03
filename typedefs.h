#pragma once

#include <fcntl.h>

#include <vector>
#include <string>
#include <filesystem>
#include <utility>
#include <list>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <unordered_set>

#include <Poco/Exception.h>
#include <Poco/String.h>
#include <Poco/Ascii.h>
#include <Poco/StringTokenizer.h>
#include <Poco/DateTime.h>
#include <Poco/Format.h>
#include <Poco/StreamCopier.h>
#include <Poco/Hash.h>
#include <Poco/Timer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/URI.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HttpServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>

#include <gtest/gtest.h>

using Names = std::vector<std::string>;

POCO_DECLARE_EXCEPTION(Foundation_API, FileTypeUnmatchException, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(FileTypeUnmatchException, Poco::LogicException, "file type unmatch with reader")

POCO_DECLARE_EXCEPTION(Foundation_API, DatabaseTypeException, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(DatabaseTypeException, Poco::LogicException, "database path must be a directory")

POCO_DECLARE_EXCEPTION(Foundation_API, DateTimeFormatException, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(DateTimeFormatException, Poco::LogicException, "date time format error")

POCO_DECLARE_EXCEPTION(Foundation_API, UnreachableException, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(UnreachableException, Poco::LogicException, "unreachable!")

POCO_DECLARE_EXCEPTION(Foundation_API, UnexpectedChar, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(UnexpectedChar, Poco::LogicException, "Unexpected Char!")

POCO_DECLARE_EXCEPTION(Foundation_API, UnexpectedToken, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(UnexpectedToken, Poco::LogicException, "Unexpected Token!")

POCO_DECLARE_EXCEPTION(Foundation_API, UnmatchedToken, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(UnmatchedToken, Poco::LogicException, "Unmatched Token!")

POCO_DECLARE_EXCEPTION(Foundation_API, ParseException, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(ParseException, Poco::LogicException, "Parse Error!")

std::string ROOT_PATH = "/Users/peter/Code/GraduationDesignSrc/master";
std::string RESOURCE_PATH = "/Users/peter/Code/GraduationDesignSrc/master/html";

const std::unordered_set<std::string> IGNORED_FILE_EXTENSIONS = {".DS_Store", "", ".csv", ".test"};
const std::unordered_set<std::string> ALLOWED_FILE_EXTENSIONS = {".txt", ".h", ".cpp", ".sh", ".xml", ".json", ".story"};

const int SCORE_GRANULARITY = 1000;
const int DAEMON_INTERVAL_SECONDS = 3;
const int MAX_FILE_NUMBER_EVERY_INDEX = 5000;

const std::unordered_map<std::string, std::string> USERNAME_PASSWORDS = {{"admin", "admin"}, {"peter", "123456"}};

template<typename T>
void THROW_HELPER(const char* file, int line, const char* func, const T& e) __attribute__ ((noreturn));

#define THROW(exception) THROW_HELPER(__FILE__, __LINE__, __func__, exception)

template<typename T>
void THROW_HELPER(const char* file, int line, const char* func, const T& e)
{
    static_assert(std::is_base_of_v<Poco::Exception, T>);
    std::string location_msg = std::string(file) + ':' + std::to_string(line) + "," + func;
    throw T("<message - " + e.message() + "> <location - " + location_msg + ">");
}
