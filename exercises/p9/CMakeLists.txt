cmake_minimum_required(VERSION 3.5)

project(p9)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag" "*.tese" "*.tesc")

add_executable(p9 ${SOURCES})

target_link_libraries(p9 PGUPV)

config_exercise(p9)
