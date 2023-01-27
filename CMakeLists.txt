cmake_minimum_required(VERSION 3.19)
project(ZSearch)
set(CMAKE_CXX_STANDARD 20)

include_directories(.)
include_directories(/usr/local/include/antlr4-runtime)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)

link_libraries(antlr4-runtime)
link_libraries(xapian)

find_package(Poco REQUIRED Util)
link_libraries(Poco::Util)
link_libraries(gtest)

add_executable(ZSearch cmdapi/main.cpp)

add_executable(gtest_storage storage/testStorage.cpp)
add_executable(gtest_extractor extractor/testExtractor.cpp)
add_executable(gtest_core core/testCore.cpp)
add_executable(gtest_utils utils/testUtils.cpp)
#add_executable(gtest_relational relational/testRelational.cpp)

#add_executable(testXapianIndexer third-party-tests/testXapianIndexer.cpp)
#add_executable(testXapianSearcher third-party-tests/testXapianSearcher.cpp)
add_executable(testPoco third-party-tests/testPoco.cpp)

add_executable(testAntlr4 queryparser/grammars/testAntlr4.cpp
        queryparser/grammars/QueryListener.cpp
        queryparser/grammars/QueryParser.cpp
        queryparser/grammars/QueryLexer.cpp
        queryparser/grammars/QueryBaseListener.cpp
        queryparser/grammars/QueryVisitor.cpp
        queryparser/grammars/QueryBaseVisitor.cpp
        )