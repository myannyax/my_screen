cmake_minimum_required(VERSION 3.12)
project("my_screen" LANGUAGES C CXX VERSION "0.1")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_executable(my_screen_server
        src/main.cpp
        src/Server.cpp
        src/ServerLogic.cpp
)

add_executable(my_screen_client
        client/src/main.cpp
        client/src/Client.cpp
        client/src/ClientLogic.cpp
)

target_compile_options(my_screen_server PRIVATE -Wall -Wextra -pedantic)
target_compile_features(my_screen_server PRIVATE cxx_std_20)

target_compile_features(my_screen_client PRIVATE cxx_std_20)
