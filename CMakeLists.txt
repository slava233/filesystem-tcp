cmake_minimum_required(VERSION 4.3)
project(fs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(fs
        src/main.cpp
)

target_include_directories(fs PRIVATE src)

target_link_libraries(fs
        PRIVATE
        Boost::system
        Threads::Threads
        stdc++fs
)

target_compile_options(fs PRIVATE -Wall -Wextra)