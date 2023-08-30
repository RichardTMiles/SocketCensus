cmake_minimum_required(VERSION 3.5)
project(major2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    cli.c
    README.md
    svr.c)

add_executable(major2 ${SOURCE_FILES})