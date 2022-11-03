cmake_minimum_required(VERSION 3.14.1)
project(cuke)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

include(cmake/gtest.cmake)
include(cmake/asio.cmake)
include(cmake/nlohmann_json.cmake)

set(CMAKE_CXX_STANDARD 17)

add_library(cuke INTERFACE)
target_link_libraries(cuke INTERFACE asio::asio nlohmann_json::nlohmann_json GTest::gtest GTest::gmock)
target_include_directories(cuke INTERFACE .)
add_library(cuke::cuke ALIAS cuke)

enable_testing()
add_subdirectory(test)
add_subdirectory(example/calculator)
