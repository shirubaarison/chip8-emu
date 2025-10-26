#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <raudio.h>

#include "chip8.h"

#define SCALE 10
#define WINDOW_WIDTH (WIDTH * SCALE)
#define WINDOW_HEIGHT (HEIGHT * SCALE)
#define WINDOW_TITLE "Chip8"

static uint8_t keymap[GLFW_KEY_LAST + 1];

static void initKeymap(void) {
  for (int i = 0; i <= GLFW_KEY_LAST; i++)
    keymap[i] = 0xFF;

  keymap[GLFW_KEY_X] = 0x0;
  keymap[GLFW_KEY_1] = 0x1;
  keymap[GLFW_KEY_2] = 0x2;
  keymap[GLFW_KEY_3] = 0x3;
  keymap[GLFW_KEY_4] = 0xC;
  keymap[GLFW_KEY_Q] = 0x4;
  keymap[GLFW_KEY_W] = 0x5;
  keymap[GLFW_KEY_E] = 0x6;
  keymap[GLFW_KEY_R] = 0xD;
  keymap[GLFW_KEY_A] = 0x7;
  keymap[GLFW_KEY_S] = 0x8;
  keymap[GLFW_KEY_D] = 0x9;
  keymap[GLFW_KEY_F] = 0xE;
  keymap[GLFW_KEY_Z] = 0xA;
  keymap[GLFW_KEY_C] = 0xB;
  keymap[GLFW_KEY_V] = 0xF;

  keymap[GLFW_KEY_UP] = 0x2;
  keymap[GLFW_KEY_DOWN] = 0x8;
  keymap[GLFW_KEY_LEFT] = 0x4;
  keymap[GLFW_KEY_RIGHT] = 0x6;

}

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

static void windowSizeCallback(GLFWwindow* window, int w, int h) {
  // to supress warnings
  (void)window;

  float aspect = (float)WIDTH / HEIGHT;
  int viewWidth = w;
  int viewHeight = (int)(w / aspect);

  if (viewHeight > h) {
    viewHeight = h;
    viewWidth = (int)(h * aspect);
  }

  int offsetX = (w - viewWidth) / 2;
  int offsetY = (h - viewHeight) / 2;

  glViewport(offsetX, offsetY, viewWidth, viewHeight);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

static GLFWwindow* setupWindow(void) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to create GLFW Window.\n");
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
 
  GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "GLAD initialization failed.\n");
    glfwTerminate();
    exit(1);
  }

  const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwSetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);

  glfwSetWindowPos(window, (mode->width - WINDOW_WIDTH) / 2,
                   (mode->height - WINDOW_HEIGHT) / 2);

  glfwSetWindowSizeCallback(window, windowSizeCallback);
  glfwFocusWindow(window);

  return window;
}

static void drawPixel(int x, int y) {
  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x + 1, y);
  glVertex2f(x + 1, y + 1);
  glVertex2f(x, y + 1);
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

  initKeymap();
  glfwSetWindowUserPointer(window, chip);
  glfwSetKeyCallback(window, keyCallback);

  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  windowSizeCallback(window, fbWidth, fbHeight);

  return window;
}
