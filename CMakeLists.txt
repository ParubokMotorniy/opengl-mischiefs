cmake_minimum_required(VERSION 3.10)
project(engine)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES src/*)

add_executable(engine ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}
    glfw
    GL
    X11
    pthread
    Xrandr
    Xi
    dl
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_ROOT="${PROJECT_SOURCE_DIR}" 
                                                   ENGINE_SHADERS="${PROJECT_SOURCE_DIR}/shaders" 
                                                   ENGINE_TEXTURES="${PROJECT_SOURCE_DIR}/textures")
