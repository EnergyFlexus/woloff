cmake_minimum_required(VERSION 3.10)
project(woloff)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/woloff)

add_executable(woloff src/main.cpp)
target_link_libraries(woloff)