cmake_minimum_required(VERSION 3.5)

project(p5)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag")

add_executable(p5 ${SOURCES})

target_link_libraries(p5 PGUPV)

config_exercise(p5)

