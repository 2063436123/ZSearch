cmake_minimum_required(VERSION 3.23)
project(ZSearch)
set(CMAKE_CXX_STANDARD 20)

include_directories(.)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

link_libraries(xapian)

find_package(Poco REQUIRED Util)
link_libraries(Poco::Util)

link_libraries(gtest)

add_executable(ZSearch cmdapi/main.cpp)

add_executable(gtest_storage storage/testStorage.cpp)
add_executable(gtest_extractor extractor/testExtractor.cpp)
add_executable(gtest_core core/testCore.cpp)

add_executable(testXapianIndexer third-party-tests/testXapianIndexer.cpp)
add_executable(testXapianSearcher third-party-tests/testXapianSearcher.cpp)
add_executable(testPoco third-party-tests/testPoco.cpp)