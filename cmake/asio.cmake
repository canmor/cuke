set(ASIO_URL "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-21-0.zip" CACHE STRING "ASIO URL")
set(ASIO_URL_HASH "MD5=8d21cba2f802967c0e414b5e66673e44" CACHE STRING "ASIO URL HASH")
set(ASIO_GIT_REPOSITORY "https://github.com/chriskohlhoff/asio" CACHE STRING "asio Git Repository URL")
set(ASIO_GIT_TAG "asio-1-21-0" CACHE STRING "asio Git Repository tag")

if (NOT TARGET asio::asio)
    find_package(asio QUIET)
    if (NOT asio_FOUND)
        include(ExternalProject)
        if (ASIO_URL STREQUAL "")
            ExternalProject_Add(asio_external
                    GIT_REPOSITORY ${ASIO_GIT_REPOSITORY}
                    GIT_TAG ${ASIO_GIT_TAG}
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
        else ()
            ExternalProject_Add(asio_external
                    URL ${ASIO_URL}
                    URL_HASH ${ASIO_URL_HASH}
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
        endif ()
        ExternalProject_Get_Property(asio_external SOURCE_DIR)
        add_library(asio INTERFACE)
        target_include_directories(asio INTERFACE ${SOURCE_DIR}/asio/include)
        if (WIN32)
            target_link_libraries(asio INTERFACE Ws2_32)
        endif ()
        add_dependencies(asio asio_external)
        add_library(asio::asio ALIAS asio)
    endif ()
endif ()

