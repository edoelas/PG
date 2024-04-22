cmake_minimum_required(VERSION 3.5)

project(p6)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag")

add_executable(p6 ${SOURCES})

target_link_libraries(p6 PGUPV)

config_exercise(p6)

