cmake_minimum_required(VERSION 3.25)
project(chip8-emu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(SOURCES
  src/main.c
  src/chip8.c
)

# GLAD
add_library(glad vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# GLFW
add_subdirectory(vendor/glfw)

# Executable
add_executable(chip8-emu ${SOURCES})
target_include_directories(chip8-emu PRIVATE include)

target_compile_options(chip8-emu PRIVATE
    -Wall 
    -Wextra 
    -Wpedantic 
    -Wshadow 
    -g
)

target_link_libraries(chip8-emu PRIVATE glad glfw)
# I didn't test this, I only use Linux
if (APPLE)
    target_link_libraries(chip8-emu PRIVATE "-framework OpenGL")
elseif (WIN32)
    target_link_libraries(chip8-emu PRIVATE opengl32 gdi32)
else()
    target_link_libraries(chip8-emu PRIVATE GL m pthread dl)
endif()
