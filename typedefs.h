#pragma once

#include <fcntl.h>

#include <vector>
#include <string>
#include <filesystem>
#include <utility>
#include <list>
#include <unordered_map>
#include <mutex>

#include <Poco/Exception.h>
#include <Poco/String.h>
#include <Poco/Ascii.h>
#include <Poco/StringTokenizer.h>

#include <gtest/gtest.h>

using Names = std::vector<std::string>;

POCO_DECLARE_EXCEPTION(Foundation_API, FileTypeUnmatchException, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(FileTypeUnmatchException, Poco::LogicException, "file type unmatch with reader")
POCO_DECLARE_EXCEPTION(Foundation_API, DatabaseTypeException, Poco::LogicException)
POCO_IMPLEMENT_EXCEPTION(DatabaseTypeException, Poco::LogicException, "database path must be a directory")

const std::string root_path = "/Users/peter/Code/GraduationDesignSrc/master";