cmake_minimum_required(VERSION 3.15)
project(json_bench CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimised build
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Add include directories for all cloned repos
include_directories(
    ${CMAKE_SOURCE_DIR}/json/include          # nlohmann/json
    ${CMAKE_SOURCE_DIR}/rapidjson/include     # RapidJSON
    ${CMAKE_SOURCE_DIR}/simdjson/include      # simdjson
	${CMAKE_SOURCE_DIR}/json/include/nlohmann
    ${CMAKE_SOURCE_DIR}/Json          # your parser (correct path)
)


# simdjson needs its library built
add_subdirectory(simdjson)

# Build the benchmark
add_executable(json_bench json_bench.cpp Json/Json.cpp)

# Link simdjson
target_link_libraries(json_bench PRIVATE simdjson)
