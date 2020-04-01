cmake_minimum_required(VERSION 3.10)

project(hashlife)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FILE(GLOB SOURCE_FILES src/*.cpp)
FILE(GLOB TEST_FILES tests/*.cpp)

include_directories(include)
include_directories(external)

add_executable(conway conway.cpp ${SOURCE_FILES})
add_executable(execute_test ${TEST_FILES} ${SOURCE_FILES})