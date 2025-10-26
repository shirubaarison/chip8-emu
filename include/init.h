#ifndef init_h
#define init_h

#include "chip8.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLFWwindow* setup(chip8* chip);

void render(chip8* chip);

#endif // !graphics_H
