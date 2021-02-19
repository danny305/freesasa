cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(freesasa LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)



set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
