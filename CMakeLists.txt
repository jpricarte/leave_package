cmake_minimum_required(VERSION 3.2)
project(leave_package)

set(CMAKE_CXX_STANDARD 20)

add_executable(leave_package src/main.cpp src/user.cpp include/user.h src/communication.cpp include/communication.h include/monitor.h src/commandHandler.cpp include/commandHandler.h src/client.cpp include/client.h)
target_link_libraries(leave_package pthread)