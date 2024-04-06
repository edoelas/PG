add_library(PGUPV STATIC)

file(GLOB SRCS "src/*.cpp" "include/*.h")

target_sources(PGUPV PRIVATE ${SRCS})

target_include_directories(PGUPV PUBLIC include/)
target_link_libraries(PGUPV PRIVATE PG-deps)
target_link_libraries(PGUPV PUBLIC PG-public-deps)
