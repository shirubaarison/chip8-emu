#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <string.h>

#include "chip8.h"

#define SCALE 12
#define WINDOW_WIDTH (WIDTH * SCALE)
#define WINDOW_HEIGHT (HEIGHT * SCALE)
#define WINDOW_TITLE "Chip8"

static uint8_t keymap[GLFW_KEY_LAST + 1] = {
  [GLFW_KEY_X] = 0x0,
  [GLFW_KEY_1] = 0x1,
  [GLFW_KEY_2] = 0x2,
  [GLFW_KEY_3] = 0x3,
  [GLFW_KEY_4] = 0xC,
  [GLFW_KEY_Q] = 0x4,
  [GLFW_KEY_W] = 0x5,
  [GLFW_KEY_E] = 0x6,
  [GLFW_KEY_R] = 0xD,
  [GLFW_KEY_A] = 0x7,
  [GLFW_KEY_S] = 0x8,
  [GLFW_KEY_D] = 0x9,
  [GLFW_KEY_F] = 0xE,
  [GLFW_KEY_Z] = 0xA,
  [GLFW_KEY_C] = 0xB,
  [GLFW_KEY_V] = 0xF
};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  chip8* chip = (chip8*)glfwGetWindowUserPointer(window);

  // to supress warnings
  (void)scancode;
  (void)mods;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  if (key >= 0 && key <= GLFW_KEY_LAST) {
    uint8_t mapped = keymap[key];
    if (mapped != 0xFF)
      chip->key[mapped] = (action == GLFW_PRESS) ? 1 : 0;
  }
}

static GLFWwindow* setupWindow(void) {
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

  glfwSetKeyCallback(window, keyCallback);

  return window;
}

static void setupViewPort(void) {
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

static void drawPixel(int x, int y) {
  glBegin(GL_QUADS);
    glVertex2f(x * SCALE, y * SCALE);
    glVertex2f((x + 1) * SCALE, y * SCALE);
    glVertex2f((x + 1) * SCALE, (y + 1) * SCALE);
    glVertex2f(x * SCALE, (y + 1) * SCALE);
    glEnd();
}

void render(chip8* chip) {
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

GLFWwindow* setup(chip8* chip) {
  GLFWwindow* window = setupWindow();

  glfwSetWindowUserPointer(window, chip);

  setupViewPort();

  return window;
}
