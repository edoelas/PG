
function(config_exercise NAME)

target_link_libraries(${NAME} PGUPV)
set_target_properties(${NAME} PROPERTIES 
	DEBUG_POSTFIX "d"
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	FOLDER "exercises")
endfunction()

file(GLOB dirs LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "p*")

foreach (proj ${dirs})
add_subdirectory(${proj})
endforeach()
