
function(config_example NAME)
target_link_libraries(${NAME} PGUPV)
set_target_properties(${NAME} PROPERTIES 
	DEBUG_POSTFIX "d"
	TESTING_POSTFIX "d"
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	FOLDER "examples")
endfunction()

file(GLOB dirs LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")


foreach (proj ${dirs})
IF(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${proj}")
  IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${proj}/CMakeLists.txt")
    add_subdirectory(${proj})
  ENDIF()
ENDIF()
endforeach()
