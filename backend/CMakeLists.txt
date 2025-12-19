cmake_minimum_required(VERSION 3.10)
project(PVZ_Maze_Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows MSVC UTF-8 support (fix Chinese character encoding issues)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Executable
add_executable(pvz_game ${SOURCES} main.cpp)

# Link libraries (websocket, threading, etc.)
# find_package(Threads REQUIRED)
# target_link_libraries(pvz_game Threads::Threads)

# Tests
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
