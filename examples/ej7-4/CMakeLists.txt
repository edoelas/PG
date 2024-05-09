cmake_minimum_required(VERSION 3.5)

project(ej7-4)

file(GLOB SHADERS "*.vert" "*.tesc" "*.tese" "*.geom" "*.frag")
add_executable(ej7-4 main.cpp ${SHADERS})
config_example(ej7-4)