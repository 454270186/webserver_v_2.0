cmake_minimum_required(VERSION 3.0)
project(WebServer)

# Add inclue path
include_directories(
    ./buffer
    ./http
    ./threadpool
    ./webserver
    ./timer
)

# Add source file
file(GLOB SOURCES 
    "./buffer/*.cpp"
    "./http/*.cpp"
    "./threadpool/*.cpp"
    "./webserver/*.cpp"
    "./timer/*.cpp"
)

# Add excutable file
add_executable(http_server main.cpp ${SOURCES})