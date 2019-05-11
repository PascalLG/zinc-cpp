#=========================================================================

#---------------------------------------------------------------
# GoogleTest
#---------------------------------------------------------------

set(GTEST_DIR ${CMAKE_SOURCE_DIR}/lib/gtest)
if(NOT EXISTS ${GTEST_DIR})
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/googletest ${GTEST_DIR}
    )
endif()
add_subdirectory(gtest)

#---------------------------------------------------------------
# zLib
#---------------------------------------------------------------

if(${ZINC_COMPRESSION_GZIP} OR ${ZINC_COMPRESSION_DEFLATE})
    set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/lib/zlib)
    if(NOT EXISTS ${ZLIB_DIR})
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/madler/zlib ${ZLIB_DIR}
        )
    endif()
    add_subdirectory(zlib)
    if(WIN32)
        target_include_directories(zlibstatic PUBLIC ${ZLIB_DIR})
        target_include_directories(zlibstatic PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/zlib)
    endif()
endif()

#---------------------------------------------------------------
# Brotli
#---------------------------------------------------------------

if(${ZINC_COMPRESSION_BROTLI})
    set(BROTLI_DIR ${CMAKE_SOURCE_DIR}/lib/brotli)
    if(NOT EXISTS ${BROTLI_DIR})
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/brotli.git ${BROTLI_DIR}
        )
    endif()
    set(BROTLI_BUNDLED_MODE ON)
    add_subdirectory(brotli)
endif()

#=========================================================================
