cmake_minimum_required(VERSION 3.19)
project(ZSearch)
set(CMAKE_CXX_STANDARD 20)

include_directories(.)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)

find_package(Poco REQUIRED Util)
find_package(Poco REQUIRED Net)
link_libraries(Poco::Util)
link_libraries(Poco::Net)
link_libraries(gtest)

add_executable(ZSearchHTTP httpapi/main.cpp)

add_executable(gtest_storage storage/testStorage.cpp)
add_executable(gtest_extractor extractor/testExtractor.cpp)
add_executable(gtest_core core/testCore.cpp)
add_executable(gtest_utils utils/testUtils.cpp)

add_executable(testPoco third-party-tests/testPoco.cpp)
add_executable(testJson third-party-tests/testJson.cpp)
add_executable(testDaemon daemon/testDaemon.cpp)
add_executable(testmyDaemon daemon/testDaemonLightly.cpp)
add_executable(testSearcher searcher/testSearcher.cpp)
add_executable(testParser queryparser/testParser.cpp)

add_executable(testExecutor executor/testExecutor.cpp)