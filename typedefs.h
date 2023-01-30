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

const std::string ROOT_PATH = "/Users/peter/Code/GraduationDesignSrc/master";
const std::unordered_set<std::string> IGNORED_FILE_EXTENSIONS = {".DS_Store", "", ".csv"};

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
