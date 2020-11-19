cmake_minimum_required(VERSION 3.12)

project(
    fsmlite
    VERSION 0.7.3
    DESCRIPTION "fsmlite library"
    HOMEPAGE_URL "https://github.com/tkem/fsmlite"
)

option(ENABLE_TESTS OFF "Enable test")

# Make sure that custom modules are found
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/modules)

# Create an INTERFACE target, ie. header only
add_library(${PROJECT_NAME} INTERFACE)
# Using a library defined in the same CMake tree should look the same as using an external library.
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

list(APPEND PUBLIC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fsm.h"
)

if(ENABLE_TESTS)
  enable_testing()
  include(CTest)
  add_subdirectory(tests)
endif()

# Add install options
include(Install)