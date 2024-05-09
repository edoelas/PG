cmake_minimum_required(VERSION 3.5)

project(p7)

file (GLOB SOURCES "*.cpp" "*.h" "*.hpp" "*.vert" "*.frag" "*.tese" "*.tesc")

add_executable(p7 ${SOURCES})

target_link_libraries(p7 PGUPV)

config_exercise(p7)

