cmake_minimum_required(VERSION 3.14)

include(FetchContent)

find_package(OpenGL REQUIRED)


if (MSVC)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(SDL_TEST OFF)
set(SDL_TEST_LIBRARY OFF)

FetchContent_Declare(SDL2
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
    GIT_TAG "release-2.28.5"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SDL2)

add_library(freetype STATIC IMPORTED)
set (FREETYPE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/freetype-2.13.2/objs/freetype.lib)
set(FREETYPE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/freetype-2.13.2/include)

set_target_properties(freetype PROPERTIES
			 INTERFACE_INCLUDE_DIRECTORIES ${FREETYPE_INCLUDE_DIRS}
			 IMPORTED_IMPLIB ${FREETYPE_LIBRARY}
)

set(SDL2TTF_VENDORED OFF)
set(SDL2TTF_INSTALL OFF)
set(SDL2TTF_SAMPLES OFF)
FetchContent_Declare(SDL_ttf
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL_ttf.git"
    GIT_TAG "release-2.20.2"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SDL_ttf)
endif()


set(GLM_BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(glm
    GIT_REPOSITORY "https://github.com/g-truc/glm"
    GIT_TAG "1.0.0" 
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(glm)


FetchContent_Declare(imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG "v1.90" 
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(imgui)


set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
FetchContent_Declare(assimp
    GIT_REPOSITORY "https://github.com/assimp/assimp.git"
    GIT_TAG "v5.3.1" 
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v4.0.0"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(GSL)


FetchContent_Declare(json 
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG "v3.11.2"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(json)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(GLI_TEST_ENABLE OFF)
FetchContent_Declare(gli
    GIT_REPOSITORY "https://github.com/g-truc/gli.git"
#    GIT_TAG "v3.11.2"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(gli)
add_library(gli_lib INTERFACE)
target_include_directories(gli_lib INTERFACE ${gli_SOURCE_DIR})

FetchContent_Declare(par
    GIT_REPOSITORY "https://github.com/prideout/par.git"
#    GIT_TAG "v3.11.2"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(par)
add_library(par_lib INTERFACE)
target_include_directories(par_lib INTERFACE ${par_SOURCE_DIR})

set(WITH_JPEG8 ON)
add_subdirectory(openpnp-capture)
# set the include dir
target_include_directories(openpnp-capture PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/openpnp-capture/include>)



if (MSVC)
add_library(GLEW STATIC glew-2.2.0/src/glew.c)
target_include_directories(GLEW PUBLIC glew-2.2.0/include)
target_compile_definitions(GLEW PUBLIC GLEW_STATIC)
target_link_libraries(GLEW PUBLIC OpenGL::GL)

set(FreeImageDLL ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/Dist/x64/FreeImage.dll)
add_library(freeimage SHARED IMPORTED)
set_target_properties(freeimage PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/Dist/x64/
        IMPORTED_LOCATION ${FreeImageDLL}
	IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/Dist/x64/FreeImage.lib
)


set(FFmpeg_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-6.1-full_build-shared")
set(FFmpeg_INCLUDE_DIR ${FFmpeg_ROOT}/include)
set(FFmpeg_BIN_DIR ${FFmpeg_ROOT}/bin)

file(GLOB FF_LIBS ${FFmpeg_ROOT}/lib/*.lib)

set(EXTRA_LIBS "")

foreach (LIBFILE ${FF_LIBS})
get_filename_component(module ${LIBFILE} NAME_WE)
set(libname lib${module})

# find the dll
file(GLOB DLL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${module}*.dll)

add_library(${libname} SHARED IMPORTED)
set_target_properties(${libname} PROPERTIES
			 INTERFACE_INCLUDE_DIRECTORIES ${FFmpeg_INCLUDE_DIR}
             IMPORTED_LOCATION ${DLL}
			 IMPORTED_IMPLIB ${LIBFILE}
)
set(EXTRA_LIBS ${EXTRA_LIBS} ${libname})
endforeach()

set (EXTRA_LIBS ${EXTRA_LIBS} SDL2::SDL2)
set (EXTRA_LIBS ${EXTRA_LIBS} SDL2_ttf::SDL2_ttf)

else()
find_package(FREEIMAGE REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(GLEW REQUIRED)
set(EXTRA_LIBS ${FFMPEG_LIBRARIES} SDL2 SDL2_ttf)
endif()

add_library(PG-deps INTERFACE)
target_link_libraries(PG-deps INTERFACE assimp::assimp freeimage nlohmann_json::nlohmann_json gli_lib ${EXTRA_LIBS} par_lib openpnp-capture)

add_library(PG-public-deps INTERFACE)
target_link_libraries(PG-public-deps INTERFACE glm::glm GLEW guipg Microsoft.GSL::GSL)

set(imdir ${imgui_SOURCE_DIR})

add_library(dearimgui STATIC ${imdir}/imgui.cpp ${imdir}/imgui_draw.cpp ${imdir}/imgui_tables.cpp ${imdir}/imgui_widgets.cpp ${imdir}/backends/imgui_impl_sdl2.cpp ${imdir}/backends/imgui_impl_opengl3.cpp)
target_include_directories(dearimgui PUBLIC ${imdir}/ ${imdir}/backends)
target_compile_definitions(dearimgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW GLM_ENABLE_EXPERIMENTAL IMGUI_DISABLE_OBSOLETE_FUNCTIONS IMGUIZMO_IMGUI_FOLDER= IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(dearimgui PRIVATE SDL2::SDL2)

if (MSVC)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(assimp gli_dummy glm dearimgui GLEW sdl_headers_copy SDL2 SDL2_ttf SDL2main SDL2-static uninstall zlibstatic UpdateAssimpLibsDebugSymbolsAndDLLs openpnp-capture openpnp-capture-test PROPERTIES FOLDER "deps")
set_target_properties(PROPERTIES FOLDER "deps")
set_target_properties(PROPERTIES FOLDER "deps")
#set_target_properties(glm_shared PROPERTIES FOLDER "deps")

endif()
