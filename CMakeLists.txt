cmake_minimum_required(VERSION 3.12)
project("my_screen" LANGUAGES C CXX VERSION "0.1")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include_directories(
        common/include
        server/include
        client/include
)

add_executable(my_screen_server
        server/src/main.cpp
        server/src/Server.cpp server/include/Server.h
        server/src/ServerLogic.cpp server/include/ServerLogic.h
        server/src/Session.cpp server/include/Session.h
        server/src/SessionLogic.cpp server/include/SessionLogic.h
        common/src/Common.cpp common/include/Common.h
)

add_executable(my_screen
        client/src/main.cpp
        client/src/Client.cpp
        common/src/Common.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_compile_options(my_screen_server PRIVATE -Wall -Wextra -pedantic)
target_compile_features(my_screen_server PRIVATE cxx_std_20)
TARGET_LINK_LIBRARIES(my_screen_server ${Boost_LIBRARIES} rt Threads::Threads)

target_compile_options(my_screen PRIVATE -Wall -Wextra -pedantic)
target_compile_features(my_screen PRIVATE cxx_std_20)
TARGET_LINK_LIBRARIES(my_screen ${Boost_LIBRARIES} rt Threads::Threads)
