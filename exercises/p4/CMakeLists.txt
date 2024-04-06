cmake_minimum_required(VERSION 3.5)

project(p4)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag")

add_executable(p4 ${SOURCES})

target_link_libraries(p4 PGUPV)

config_exercise(p4)

