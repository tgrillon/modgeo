cmake_minimum_required (VERSION 3.26)

project(modgeo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GKIT_DIR "/home/tgrillon/dev/cxx/gkit2light" CACHE PATH "Path to gkit source files.")
message(STATUS "Gkit directory set to: ${GKIT_DIR}")
set(DATA_DIR "${CMAKE_SOURCE_DIR}/data" CACHE PATH "Path to the data directory.")
message(STATUS "Data directory set to: ${DATA_DIR}")
set(OBJ_DIR "${DATA_DIR}/obj" CACHE PATH "Path to the obj files directory.")
message(STATUS "Obj files directory set to: ${OBJ_DIR}")
set(SHADER_DIR "${DATA_DIR}/shaders" CACHE PATH "Path to the shader files.")
message(STATUS "Shader files directory set to: ${SHADER_DIR}")

# Fetching gkit library 
message(STATUS "Fetching GKit...")
file(GLOB gkit_files "${GKIT_DIR}/src/gKit/*.cpp")
add_library(gkit STATIC ${gkit_files})
target_include_directories(gkit PUBLIC ${GKIT_DIR}/src/gKit)
target_link_libraries(gkit PUBLIC GL GLEW SDL2 SDL2_image)

# Fetching imgui library 
message(STATUS "Fetching ImGui...")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui")
add_library(imgui STATIC ${IMGUI_DIR}/imgui.cpp
                         ${IMGUI_DIR}/imgui.h
                         ${IMGUI_DIR}/imgui_impl_sdl_gl3.cpp
                         ${IMGUI_DIR}/imgui_impl_sdl_gl3.h)

target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})
target_link_libraries(imgui PUBLIC SDL2 SDL2_image)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/Source")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/Include")
add_executable(${PROJECT_NAME} ${SOURCE_DIR}/main.cpp 
                               ${SOURCE_DIR}/Bezier.cpp
                               ${SOURCE_DIR}/Implicit.cpp
                               ${SOURCE_DIR}/Application.cpp
                               ${INCLUDE_DIR}/Bezier.h
                               ${INCLUDE_DIR}/Application.h
                               ${INCLUDE_DIR}/Implicit.h)

target_link_libraries(${PROJECT_NAME} gkit
                                      imgui)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PUBLIC DATA_DIR="${DATA_DIR}"
                                                  OBJ_DIR="${OBJ_DIR}"
                                                  SHADER_DIR="${SHADER_DIR}")
