CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(server_project)
SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
find_package (Threads)
add_executable(clientMini ./client.cpp)
target_link_libraries (clientMini ${CMAKE_THREAD_LIBS_INIT})
