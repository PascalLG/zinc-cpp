#=========================================================================

cmake_minimum_required(VERSION 3.11)
project(zinc)

set(CMAKE_CXX_STANDARD 14)
string(TIMESTAMP ZINC_BUILD_TIMESTAMP "%s" UTC)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#---------------------------------------------------------------
# Project configuration.
#---------------------------------------------------------------

set(ZINC_VERSION_MAJOR           1)
set(ZINC_VERSION_MINOR          00)
set(ZINC_COMPRESSION_GZIP       ON)
set(ZINC_COMPRESSION_DEFLATE    ON)
set(ZINC_COMPRESSION_BROTLI     ON)
 
#---------------------------------------------------------------
# External librairies
#---------------------------------------------------------------

add_subdirectory(lib)

#---------------------------------------------------------------
# Application
#---------------------------------------------------------------

set(SOURCES_APP
    src/misc/date.cpp
    src/misc/date.h
    src/misc/filesys.cpp
    src/misc/filesys.h
    src/misc/logger.cpp
    src/misc/logger.h
    src/misc/portability.cpp
    src/misc/portability.h
    src/misc/string.cpp
    src/misc/string.h
    src/http/iconfig.h
    src/http/compression.cpp
    src/http/compression.h
    src/http/http_header.cpp
    src/http/http_header.h
    src/http/http_request.cpp
    src/http/http_request.h
    src/http/http_response.cpp
    src/http/http_response.h
    src/http/http_server.cpp
    src/http/http_server.h
    src/http/http_status.cpp
    src/http/http_status.h
    src/http/http_verb.cpp
    src/http/http_verb.h
    src/http/mimetype.cpp
    src/http/mimetype.h
    src/http/resource.cpp
    src/http/resource.h
    src/http/stream.cpp
    src/http/stream.h
    src/http/stream_chunked.cpp
    src/http/stream_chunked.h
    src/http/stream_compress.cpp
    src/http/stream_compress.h
    src/http/stream_null.cpp
    src/http/stream_null.h
    src/http/stream_socket.cpp
    src/http/stream_socket.h
    src/http/uri.cpp
    src/http/uri.h
    src/main/configuration.cpp
    src/main/configuration.h
    src/main/resource_builtin.cpp
    src/main/resource_builtin.h
    src/main/resource_directory.cpp
    src/main/resource_directory.h
    src/main/resource_error_page.cpp
    src/main/resource_error_page.h
    src/main/resource_redirection.cpp
    src/main/resource_redirection.h
    src/main/resource_script.cpp
    src/main/resource_script.h
    src/main/resource_static_file.cpp
    src/main/resource_static_file.h
    src/main/zinc.cpp
    src/main/zinc.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_page_error.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_page_redirection.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_page_directory.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_image_back.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_image_folder.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_image_document.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_image_arrow_up.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_image_arrow_down.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_style.h
    ${CMAKE_CURRENT_BINARY_DIR}/resource_titilliumweb.h
)

configure_file (
  src/main/version.in.h
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
find_package(Threads REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} src/main/main.cpp ${SOURCES_APP})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC Threads::Threads)
if(NOT WIN32)
	target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -Wall)
endif()

if (${ZINC_COMPRESSION_GZIP})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ZINC_COMPRESSION_GZIP)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC zlibstatic)
#    include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/lib/zlib)
#    include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/lib/zlib_build)
endif()

if (${ZINC_COMPRESSION_DEFLATE})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ZINC_COMPRESSION_DEFLATE)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC zlibstatic)
endif()

if (${ZINC_COMPRESSION_BROTLI})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ZINC_COMPRESSION_BROTLI)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC brotlienc-static)
endif()

#---------------------------------------------------------------
# Resource files
#---------------------------------------------------------------

set(RESOURCE_FILES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/resources)
add_executable(makeres ${RESOURCE_FILES_DIR}/makeres.c)

macro(MAKERESOURCE SOURCE DESTINATION)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DESTINATION}
        DEPENDS makeres ${RESOURCE_FILES_DIR}/${SOURCE}
        COMMAND makeres ${SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/${DESTINATION}
        WORKING_DIRECTORY ${RESOURCE_FILES_DIR}
    )
endmacro()

MAKERESOURCE(page_error.html            resource_page_error.h)
MAKERESOURCE(page_redirection.html      resource_page_redirection.h)
MAKERESOURCE(page_directory.html        resource_page_directory.h)
MAKERESOURCE(image_back.png             resource_image_back.h)
MAKERESOURCE(image_folder.png           resource_image_folder.h)
MAKERESOURCE(image_document.png         resource_image_document.h)
MAKERESOURCE(image_arrow_up.png         resource_image_arrow_up.h)
MAKERESOURCE(image_arrow_down.png       resource_image_arrow_down.h)
MAKERESOURCE(style.css                  resource_style.h)
MAKERESOURCE(TitilliumWeb-Regular.ttf   resource_titilliumweb.h)

#---------------------------------------------------------------
# Unit Tests
#---------------------------------------------------------------

set(SOURCES_TEST
    test/misc/date.cpp
    test/misc/filesys.cpp
    test/misc/string.cpp
    test/http/compression.cpp
    test/http/http_header.cpp
    test/http/http_request.cpp
    test/http/http_status.cpp
    test/http/http_verb.cpp
    test/http/mimetype.cpp
    test/http/stream_chunked.cpp
    test/http/stream_compress.cpp
    test/http/uri.cpp
    test/main/configuration.cpp
    test/main/resource_redirection.cpp
    test/main/resource_script.cpp
)

set(UT_PROJECT_NAME ut)

add_executable(${UT_PROJECT_NAME} ${SOURCES_TEST} ${SOURCES_APP})
target_include_directories(${UT_PROJECT_NAME} PUBLIC src)
target_link_libraries(${UT_PROJECT_NAME} PUBLIC gtest gtest_main)
target_compile_definitions(${UT_PROJECT_NAME} PUBLIC UNIT_TESTING)

if (${ZINC_COMPRESSION_GZIP})
    target_compile_definitions(${UT_PROJECT_NAME} PUBLIC ZINC_COMPRESSION_GZIP)
    target_link_libraries(${UT_PROJECT_NAME} PUBLIC zlibstatic)
endif()

if (${ZINC_COMPRESSION_DEFLATE})
    target_compile_definitions(${UT_PROJECT_NAME} PUBLIC ZINC_COMPRESSION_DEFLATE)
    target_link_libraries(${UT_PROJECT_NAME} PUBLIC zlibstatic)
endif()

if (${ZINC_COMPRESSION_BROTLI})
    target_compile_definitions(${UT_PROJECT_NAME} PUBLIC ZINC_COMPRESSION_BROTLI)
    target_link_libraries(${UT_PROJECT_NAME} PUBLIC brotlienc-static)
endif()

#=========================================================================
