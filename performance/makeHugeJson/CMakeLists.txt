cmake_minimum_required(VERSION 3.10)
project(json)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(makeHugeJson makeHugeJson.cpp)

target_compile_features(makeHugeJson PRIVATE cxx_std_20)

target_compile_options(makeHugeJson PRIVATE -Wall -Wextra -Wpedantic)

set(CMAKE_BUILD_TYPE Debug)
