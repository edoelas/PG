cmake_minimum_required(VERSION 3.5)

project(p10)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag" "*.tese" "*.tesc")

add_executable(p10 ${SOURCES})

target_link_libraries(p10 PGUPV)

config_exercise(p10)
