cmake_minimum_required(VERSION 3.10)
project(rdma_demo C)

set(CMAKE_C_STANDARD 99)

# Ensure POSIX functions are available
add_definitions(-D_POSIX_C_SOURCE=200809L)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(IBVERBS REQUIRED libibverbs)

# Include directories
include_directories(
    ${IBVERBS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link directories
link_directories(
    ${IBVERBS_LIBRARY_DIRS}
)

# Source files
set(SOURCES
    src/rdma_common.c
)

# Server executable
add_executable(rdma_server
    src/rdma_server.c
    ${SOURCES}
)

target_link_libraries(rdma_server
    PRIVATE
    ${IBVERBS_LIBRARIES}
)

# Client executable
add_executable(rdma_client
    src/rdma_client.c
    ${SOURCES}
)

target_link_libraries(rdma_client
    PRIVATE
    ${IBVERBS_LIBRARIES}
)