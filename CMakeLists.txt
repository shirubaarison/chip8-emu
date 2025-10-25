cmake_minimum_required(VERSION 3.10)
project(chip8_emu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_library(glad vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

add_subdirectory(vendor/glfw)
include_directories(vendor/glfw/include)

add_executable(chip8-emu ${SOURCES})
target_link_libraries(chip8-emu PRIVATE glad glfw)

if (APPLE)
    target_link_libraries(chip8-emu PRIVATE "-framework OpenGL")
elseif (WIN32)
    target_link_libraries(chip8-emu PRIVATE opengl32 gdi32)
else()
    target_link_libraries(chip8-emu PRIVATE GL m pthread)
endif()
