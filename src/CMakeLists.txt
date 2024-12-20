set(SOURCE_DIR "Source")
set(INCLUDE_DIR "Include")
add_executable(${PROJECT_NAME} main.cpp 
                               ${SOURCE_DIR}/Window.cpp
                               ${SOURCE_DIR}/App.cpp
                               ${SOURCE_DIR}/Framebuffer.cpp
                               ${SOURCE_DIR}/Viewer.cpp
                               ${SOURCE_DIR}/Timer.cpp
                               ${SOURCE_DIR}/vecext.cpp
                               ${SOURCE_DIR}/Bezier.cpp
                               ${SOURCE_DIR}/SDF.cpp
                               ${SOURCE_DIR}/Box.cpp
                               ${SOURCE_DIR}/pch.cpp

                               ${INCLUDE_DIR}/Window.h
                               ${INCLUDE_DIR}/App.h
                               ${INCLUDE_DIR}/Framebuffer.h
                               ${INCLUDE_DIR}/Viewer.h
                               ${INCLUDE_DIR}/Timer.h
                               ${INCLUDE_DIR}/Bezier.h
                               ${INCLUDE_DIR}/vecext.h
                               ${INCLUDE_DIR}/Box.h
                               ${INCLUDE_DIR}/SDF.h
                               ${INCLUDE_DIR}/Utils.h
                               ${INCLUDE_DIR}/pch.h
                               )

target_link_libraries(${PROJECT_NAME} PRIVATE gkit
                                              imgui
                                              exprtk
                                              )
                                              
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR}) 
target_precompile_headers(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR/pch.h})

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data" CACHE PATH "Path to the data directory.")
message(STATUS "Data directory set to: ${DATA_DIR}")
set(OBJ_DIR "${DATA_DIR}/obj" CACHE PATH "Path to the obj files directory.")
message(STATUS "Obj files directory set to: ${OBJ_DIR}")
set(SHADER_DIR "${DATA_DIR}/shaders" CACHE PATH "Path to the shader directory.")
message(STATUS "Shader directory set to: ${SHADER_DIR}")
set(MAP_DIR "${DATA_DIR}/map" CACHE PATH "Path to the map directory.")
message(STATUS "Map directory set to: ${MAP_DIR}")

target_compile_definitions(${PROJECT_NAME} PUBLIC DATA_DIR="${DATA_DIR}"
                                           OBJ_DIR="${OBJ_DIR}"
                                           SHADER_DIR="${SHADER_DIR}"
                                           MAP_DIR="${MAP_DIR}"
                                           )           