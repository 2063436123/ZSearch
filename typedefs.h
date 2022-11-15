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

#include <Poco/Exception.h>
#include <Poco/String.h>
#include <Poco/Ascii.h>
#include <Poco/StringTokenizer.h>
#include <Poco/DateTime.h>

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

const std::string root_path = "/Users/peter/Code/GraduationDesignSrc/master";