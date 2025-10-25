#include "chip8.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SCALE 10
#define WINDOW_WIDTH (WIDTH * SCALE)
#define WINDOW_HEIGHT (HEIGHT * SCALE)
#define WINDOW_TITLE "Chip8"

static GLFWwindow* setup() {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to create GLFW Window.\n");
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

  GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "GLAD initialization failed.\n");
    glfwTerminate();
    exit(1);
  }

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  return window;
}

static void drawPixel(int x, int y) {
  glBegin(GL_QUADS);
    glVertex2f(x * SCALE, y * SCALE);
    glVertex2f((x + 1) * SCALE, y * SCALE);
    glVertex2f((x + 1) * SCALE, (y + 1) * SCALE);
    glVertex2f(x * SCALE, (y + 1) * SCALE);
    glEnd();
}

static void render(chip8* chip) {
  glClear(GL_COLOR_BUFFER_BIT);
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (chip->gfx[x + (y * WIDTH)] == 0)
        glColor3f(0.0f, 0.0f, 0.0f);
      else
        glColor3f(1.0f, 1.0f, 1.0f);

      drawPixel(x, y);
    }
  }
  chip->drawFlag = false;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  chip8* chip = (chip8*)glfwGetWindowUserPointer(window);

  int keymap[KEY_SIZE] = {
    GLFW_KEY_X,     // 0
    GLFW_KEY_1,     // 1
    GLFW_KEY_2,     // 2
    GLFW_KEY_3,     // 3
    GLFW_KEY_Q,     // 4
    GLFW_KEY_W,     // 5
    GLFW_KEY_E,     // 6
    GLFW_KEY_A,     // 7
    GLFW_KEY_S,     // 8
    GLFW_KEY_D,     // 9
    GLFW_KEY_Z,    // A
    GLFW_KEY_C,    // B
    GLFW_KEY_4,    // C
    GLFW_KEY_R,    // D
    GLFW_KEY_F,    // E
    GLFW_KEY_V     // F
  };

  if (action == GLFW_PRESS) {
    for (int i = 0; i < KEY_SIZE; i++) {
      if (key == keymap[i]) {
        chip->key[i] = 1;
      }
    }

    if (key == GLFW_KEY_ESCAPE) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  } else if (action == GLFW_RELEASE) {
    for (int i = 0; i < KEY_SIZE; i++) {
      if (key == keymap[i]) {
        chip->key[i] = 0;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <ROM file>.\n", argv[0]);
    return EXIT_FAILURE;
  }

  GLFWwindow* window = setup();
 
  chip8 chip;
  chip8_initialize(&chip);

  glfwSetWindowUserPointer(window, &chip);
  glfwSetKeyCallback(window, keyCallback);

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
