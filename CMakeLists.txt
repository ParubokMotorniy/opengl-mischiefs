cmake_minimum_required(VERSION 3.10)
project(engine)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES src/*)

add_executable(engine ${SRC_FILES})

target_link_libraries(engine
    glfw
    GL
    X11
    pthread
    Xrandr
    Xi
    dl
)
