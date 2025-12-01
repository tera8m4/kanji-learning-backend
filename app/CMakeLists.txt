cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# create project
project(MyProject)

set (CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options(/utf-8)
endif()


# add executable
add_executable(main WIN32 src/main.cpp src/controller.cpp src/controller.h src/kanji.h src/database.h src/database.cpp)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage("gh:webview/webview#0.12.0")
CPMAddPackage("gh:nlohmann/json@3.12.0")
CPMAddPackage(
    NAME sqlite3
    VERSION 3.51.1
    URL https://sqlite.org/2025/sqlite-amalgamation-3510100.zip
)

add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})

# link dependencies
target_link_libraries(main PRIVATE webview::core sqlite3 nlohmann_json::nlohmann_json) 


