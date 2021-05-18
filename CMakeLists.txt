cmake_minimum_required(VERSION 3.12)
project("my_screen" LANGUAGES C CXX VERSION "0.1")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_executable(my_screen
        client/src/main.cpp
        client/src/Client.cpp
        server/src/Server.cpp
        server/src/ServerLogic.cpp
        server/src/Session.cpp
        server/src/SessionLogic.cpp
        common/src/Common.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_include_directories(my_screen PRIVATE common/include client/include server/include)
target_compile_options(my_screen PRIVATE -Wall -Wextra -pedantic)
target_compile_features(my_screen PRIVATE cxx_std_20)
target_link_libraries(my_screen rt Threads::Threads util)
