cmake_minimum_required(VERSION 3.28)
project(finalproject C)

set(CMAKE_C_STANDARD 11)

find_package(CURL REQUIRED)

# Specify the include and library directories for cJSON
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(finalproject main.c)

target_link_libraries(finalproject CURL::libcurl cjson)
