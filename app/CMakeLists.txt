cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(KanjiReview)

set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options(/utf-8)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library("${PROJECT_NAME}Lib" STATIC ${SOURCES} ${HEADERS})
target_include_directories("${PROJECT_NAME}Lib" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(cmake/CPM.cmake)
CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG "asio-1-36-0")

CPMAddPackage(
    NAME Crow
    GITHUB_REPOSITORY CrowCpp/Crow
    VERSION 1.3.1
    EXCLUDE_FROM_ALL YES
    OPTIONS "CROW_USE_BOOST OFF" "CROW_INSTALL OFF"
)
CPMAddPackage("gh:gabime/spdlog@1.16.0")
CPMAddPackage("gh:nlohmann/json@3.12.0")
CPMAddPackage(
    NAME sqlite3
    VERSION 3.51.1
    URL https://sqlite.org/2025/sqlite-amalgamation-3510100.zip
)

find_package(CURL QUIET)
if(NOT CURL_FOUND)
    CPMAddPackage(
      NAME curl
      GITHUB_REPOSITORY curl/curl
      GIT_TAG curl-8_11_1
      OPTIONS
        "BUILD_CURL_EXE OFF"
        "BUILD_SHARED_LIBS OFF"
        "CURL_USE_SCHANNEL ON"
        "CURL_USE_OPENSSL OFF"
        "BUILD_TESTING OFF"
        "CURL_DISABLE_INSTALL ON"
    )
    set(CURL_LINK_TARGET libcurl_static)
    set(CURL_STATIC TRUE)
else()
    message(STATUS "Using system curl")
    set(CURL_LINK_TARGET CURL::libcurl)
endif()

CPMAddPackage(
    NAME jwt-cpp
    GITHUB_REPOSITORY Thalhammer/jwt-cpp
    VERSION 0.7.2
    OPTIONS "JWT_BUILD_EXAMPLES OFF" "JWT_DISABLE_PICOJSON ON"
)
find_package(OpenSSL REQUIRED)

add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})

target_link_libraries("${PROJECT_NAME}Lib" PUBLIC
    sqlite3 nlohmann_json::nlohmann_json spdlog ${CURL_LINK_TARGET}
    jwt-cpp::jwt-cpp OpenSSL::SSL OpenSSL::Crypto Crow::Crow asio)
    
if(CURL_STATIC)
    target_compile_definitions("${PROJECT_NAME}Lib" PRIVATE
      CURL_STATICLIB
      _CRT_SECURE_NO_WARNINGS
    )
endif()

if(asio_ADDED)
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
    target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
    add_library(asio::asio ALIAS asio)
endif()

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_NAME}Lib" Crow::Crow asio)


option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
add_dependencies(${PROJECT_NAME} BuildUI)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${UI_DIST_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying UI dist to server assets folder..."
)
