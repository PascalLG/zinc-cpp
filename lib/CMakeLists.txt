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

set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/lib/zlib)
if(NOT EXISTS ${ZLIB_DIR})
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/madler/zlib ${ZLIB_DIR}
    )
endif()

add_subdirectory(zlib)

#---------------------------------------------------------------
# Brotli
#---------------------------------------------------------------

set(BROTLI_DIR ${CMAKE_SOURCE_DIR}/lib/brotli)
if(NOT EXISTS ${BROTLI_DIR})
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/brotli.git ${BROTLI_DIR}
    )
endif()

set(BROTLI_BUNDLED_MODE ON)
add_subdirectory(brotli)

#=========================================================================
