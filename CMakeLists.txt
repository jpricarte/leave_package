cmake_minimum_required(VERSION 3.23)
project(leave_package)

set(CMAKE_CXX_STANDARD 14)

add_executable(leave_package main.cpp)
target_link_libraries(leave_package pthread)