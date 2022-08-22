cmake_minimum_required(VERSION 3.2)
project(leave_package)

set(CMAKE_CXX_STANDARD 20)

add_executable(leave_package main.cpp user.cpp user.h communication.cpp communication.h RequestHandler.cpp RequestHandler.h fileManager.cpp fileManager.h)
target_link_libraries(leave_package pthread)
target_link_libraries(leave_package -static)