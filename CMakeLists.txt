cmake_minimum_required(VERSION 3.10)

project(itch_parser)

set(CMAKE_CXX_STANDARD 20)

add_executable(itch_parser
        src/main.cpp
        src/orderbook.cpp
        src/message_parsing.cpp
)

target_include_directories(itch_parser PRIVATE include)
