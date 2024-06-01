cmake_minimum_required(VERSION 3.5)

project(ej10-1)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag" "*.tese" "*.tesc")

add_executable(ej10-1 ${SOURCES})
config_example(ej10-1)