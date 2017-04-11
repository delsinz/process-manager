cmake_minimum_required(VERSION 3.6)
project(process_manager)

set(CMAKE_C_STANDARD 99)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(SOURCE_FILES swap.c process.h process.c swap.h)
add_executable(process_manager ${SOURCE_FILES})