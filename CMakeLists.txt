cmake_minimum_required(VERSION 4.0)
project(itch_parser)

set(CMAKE_CXX_STANDARD 20)

add_executable(itch_parser src/main.cpp
        include/itch_messages.h
        include/data_structs.h
        src/orderbook.cpp
        src/message_parsing.cpp
        include/parsing_helpers.h
        include/itch_parser.h)
