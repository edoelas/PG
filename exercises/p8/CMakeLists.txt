cmake_minimum_required(VERSION 3.5)

project(p8)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag" "*.tese" "*.tesc")

add_executable(p8 ${SOURCES})

target_link_libraries(p8 PGUPV)

config_exercise(p8)

