cmake_minimum_required(VERSION 3.16)
project(engine VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_GENERATOR Ninja)

option(ENGINE_DISABLE_BINDLESS_TEXTURES "Determines whether the bindless textures are used. Note: calls to the corresponding API are merely disabled in OFF mode." OFF)

option(ENGINE_ENABLE_HW_PCF "Whether to use hardware PCF for shadow mapping" ON)

include(FetchContent)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenGL REQUIRED)

message(STATUS "Fetching GLFW...")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

message(STATUS "Fetching GLAD...")
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
FetchContent_MakeAvailable(glad)

message(STATUS "Fetching KHR headers...")
FetchContent_Declare(
    khr
    GIT_REPOSITORY https://github.com/KhronosGroup/EGL-Registry.git
    GIT_TAG main
    SOURCE_SUBDIR ""
)
FetchContent_MakeAvailable(khr)

message(STATUS "Fetching GLM...")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

message(STATUS "Fetching stb...")
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

message(STATUS "Fetching imgui")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

message(STATUS "Fetching Assimp...")
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.3.1
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

file(GLOB SRC_FILES src/*)
file(GLOB imgui_FILES 
    ${imgui_SOURCE_DIR}/*.cpp 
    ${imgui_SOURCE_DIR}/*.h 
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3*
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw*
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.*
)

list(APPEND SRC_FILES 
    ${imgui_FILES}
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${khr_SOURCE_DIR}/api
    ${imgui_SOURCE_DIR}/
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    OpenGL::GL
    glfw
    glm::glm
    assimp
    glad
)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        pthread
        Xrandr
        Xi
        dl
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WINDOWS)
    message(STATUS "Compiling for Windows!")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_definitions(${PROJECT_NAME} PUBLIC LINUX)
    message(STATUS "Compiling for Linux!")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_ROOT="${CMAKE_CURRENT_LIST_DIR}"
    ENGINE_SHADERS="${CMAKE_CURRENT_LIST_DIR}/shaders"
    ENGINE_TEXTURES="${CMAKE_CURRENT_LIST_DIR}/textures"
    ENGINE_MODELS="${CMAKE_CURRENT_LIST_DIR}/models"
)
if(ENGINE_DISABLE_BINDLESS_TEXTURES)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_DISABLE_BINDLESS_TEXTURES)
endif()

if(ENGINE_ENABLE_HW_PCF)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_ENABLE_HW_PCF)
endif()

# add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
#     COMMAND git lfs pull || true #to make sure the models and textures are intact
# )

