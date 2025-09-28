cmake_minimum_required(VERSION 3.12)

project(
  fsmlite
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/tkem/fsmlite"
)

add_library(${PROJECT_NAME} INTERFACE)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

list(APPEND PUBLIC_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fsm.h"
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
