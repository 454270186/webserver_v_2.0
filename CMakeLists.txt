cmake_minimum_required(VERSION 3.0)
project(WebServer)

# Add excutable file
add_executable(http_server main.cpp hello/hello.cpp)

# Add inclue path
target_include_directories(http_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})