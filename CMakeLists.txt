cmake_minimum_required(VERSION 3.14.1)
project(cuke)

set(GTEST_GIT_REPOSITORY "https://github.com/google/googletest.git" CACHE STRING "Googletest Git Repository URL")
set(ASIO_GIT_REPOSITORY "https://github.com/chriskohlhoff/asio" CACHE STRING "asio Git Repository URL")
set(NLOHMANN_JSON_GIT_REPOSITORY "https://github.com/nlohmann/json" CACHE STRING "nlohmann_json Git Repository URL")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

if (NOT TARGET GTest::gtest)
    find_package(GTest QUIET)
    if (NOT gtest_FOUND)
        include(FetchContent)
        FetchContent_Declare(gtest
                GIT_REPOSITORY ${GTEST_GIT_REPOSITORY}
                GIT_TAG release-1.11.0)
        FetchContent_MakeAvailable(gtest)
    endif ()
endif ()

if (NOT TARGET asio::asio)
    find_package(asio QUIET)
    if (NOT asio_FOUND)
        include(ExternalProject)
        ExternalProject_Add(asio_external
                GIT_REPOSITORY ${ASIO_GIT_REPOSITORY}
                GIT_TAG asio-1-21-0
                CONFIGURE_COMMAND ""
                BUILD_COMMAND ""
                INSTALL_COMMAND "")
        ExternalProject_Get_Property(asio_external SOURCE_DIR)
        add_library(asio INTERFACE)
        target_include_directories(asio INTERFACE ${SOURCE_DIR}/asio/include)
        add_dependencies(asio asio_external)
        add_library(asio::asio ALIAS asio)
    endif ()
endif ()

if (NOT TARGET nlohmann_json::nlohmann_json)
    find_package(nlohmann_json QUIET)
    if (NOT nlohmann_json_FOUND)
        include(FetchContent)
        FetchContent_Declare(nlohmann_json
                GIT_REPOSITORY ${NLOHMANN_JSON_GIT_REPOSITORY}
                GIT_TAG v3.10.4)
        FetchContent_MakeAvailable(nlohmann_json)
    endif ()
endif ()

set(CMAKE_CXX_STANDARD 17)

add_library(cuke INTERFACE)
target_link_libraries(cuke INTERFACE asio::asio nlohmann_json::nlohmann_json GTest::gtest GTest::gmock)
target_include_directories(cuke INTERFACE .)
add_library(cuke::cuke ALIAS cuke)

add_subdirectory(test)
add_subdirectory(example/calculator)
