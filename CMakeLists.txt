cmake_minimum_required(VERSION 3.5)
project(lserver)

set(CMAKE_CXX_STANDARD 11)

set(CXX_FLAGS "-Wall -std=c++11 -pthread -O3 -g")

link_libraries(pthread)

set(CMAKE_BUILD_TYPE "Release")

# if linux
# set(CMAKE_CXX_COMPILER "g++")
# if mac user
set(CMAKE_CXX_COMPILER "g++-5")
set(CMAKE_CXX_FLAGS_DEBUG "-O3")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

include_directories(server/include)

file(GLOB_RECURSE SOURCE_FILES server/src/*.cpp)

message("Current SOURCE_FILES" ${SOURCE_FILES})

add_executable(lserver ${SOURCE_FILES})