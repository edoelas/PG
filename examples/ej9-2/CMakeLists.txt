cmake_minimum_required(VERSION 3.5)

project(ej9-2)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag" "*.tese" "*.tesc")

add_executable(ej9-2 ${SOURCES})
config_example(ej9-2)