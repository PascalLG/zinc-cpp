#=========================================================================

cmake_minimum_required(VERSION 3.9)
project(zinc)

set(CMAKE_CXX_STANDARD 14)

set(APP_VERSION_MAJOR 0)
set(APP_VERSION_MINOR 99)
string(TIMESTAMP APP_BUILD_TIMESTAMP "%s" UTC)
 
#---------------------------------------------------------------
# Application
#---------------------------------------------------------------

set(SOURCES_APP
    src/main/http_status.cpp
    src/main/http_status.h
    src/main/http_header.cpp
    src/main/http_header.h
    src/main/http_verb.cpp
    src/main/http_verb.h
    src/main/http_request.cpp
    src/main/http_request.h
    src/main/http_response.cpp
    src/main/http_response.h
    src/main/http_server.cpp
    src/main/http_server.h
    src/main/stream.cpp
    src/main/stream.h
    src/main/stream_socket.cpp
    src/main/stream_socket.h
    src/main/stream_null.cpp
    src/main/stream_null.h
    src/main/stream_chunked.cpp
    src/main/stream_chunked.h
    src/main/stream_compress.cpp
    src/main/stream_compress.h
    src/main/resource.cpp
    src/main/resource.h
    src/main/resource_builtin.cpp
    src/main/resource_builtin.h
    src/main/resource_static_file.cpp
    src/main/resource_static_file.h
    src/main/resource_script.cpp
    src/main/resource_script.h
    src/main/resource_directory.cpp
    src/main/resource_directory.h
    src/main/resource_error_page.cpp
    src/main/resource_error_page.h
    src/main/resource_redirection.cpp
    src/main/resource_redirection.h
    src/main/misc.cpp
    src/main/misc.h
    src/main/config.cpp
    src/main/config.h
    src/main/compression.cpp
    src/main/compression.h
    src/main/filesys.cpp
    src/main/filesys.h
    src/main/mimetype.cpp
    src/main/mimetype.h
    src/main/logger.cpp
    src/main/logger.h
    src/main/uri.cpp
    src/main/uri.h
    src/main/version.cpp
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
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/lib/brotli/c/include)
find_package(Threads REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} src/main/main.cpp ${SOURCES_APP})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC zlibstatic)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC brotlienc-static)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC Threads::Threads)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -Wall)

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
    src/test/filesys.cpp
    src/test/misc.cpp
    src/test/config.cpp
    src/test/http_header.cpp
    src/test/http_request.cpp
    src/test/http_status.cpp
    src/test/http_verb.cpp
    src/test/compression.cpp
    src/test/mimetype.cpp
    src/test/stream_chunked.cpp
    src/test/stream_compress.cpp
    src/test/resource_redirection.cpp
    src/test/resource_script.cpp
    src/test/uri.cpp
)

set(UT_PROJECT_NAME ut)

add_executable(${UT_PROJECT_NAME} ${SOURCES_TEST} ${SOURCES_APP})
target_include_directories(${UT_PROJECT_NAME} PUBLIC src/main)
set_target_properties(${UT_PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS UNIT_TESTING=1)
target_link_libraries(${UT_PROJECT_NAME} PUBLIC gtest gtest_main)
target_link_libraries(${UT_PROJECT_NAME} PUBLIC zlibstatic)
target_link_libraries(${UT_PROJECT_NAME} PUBLIC brotlienc-static)

#---------------------------------------------------------------
# Librairies
#---------------------------------------------------------------

add_subdirectory(lib)

#=========================================================================
