#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chip8.h"
#include "init.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <ROM file>.\n", argv[0]);
    return EXIT_FAILURE;
  }
 
  chip8 chip;
  chip8_initialize(&chip);

  GLFWwindow* window = setup(&chip);

  chip8_load(&chip, argv[1]);

  while (!glfwWindowShouldClose(window)) {
    chip8_emulateCycle(&chip);

    if (chip.drawFlag) {
      render(&chip);
      glfwSwapBuffers(window);
    }

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
