cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(KanjiTest)

set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:catchorg/Catch2@3.11.0")

# ---- Create binary ----

file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Catch2::Catch2WithMain
    KanjiReviewLib
)

# ---- Enable testing ----

enable_testing()

# Automatically discover Catch2 tests
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)

catch_discover_tests(${PROJECT_NAME}
    DISCOVERY_MODE PRE_TEST
)