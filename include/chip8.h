#ifndef chip_8_h
#define chip_8_h

#include "raudio.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_MEMORY 4096           // 4k
#define WIDTH 64
#define HEIGHT 32
#define STACK_SIZE 16
#define KEY_SIZE 16
#define REGISTERS_SIZE 16

typedef struct {
  uint16_t opcode;                // 35 opcodes, two bytes long
  uint8_t memory[MAX_MEMORY];
  uint8_t V[REGISTERS_SIZE];      // 15 8-bit general purpose registers
  uint16_t I;                     // index register
  uint16_t pc;                    // program counter
  uint16_t stack[STACK_SIZE];
  uint8_t sp;                     // stack pointer
  uint8_t delay_timer;
  uint8_t sound_timer;

  uint8_t gfx[WIDTH * HEIGHT];    // black and white screen with 2048 pixels
  uint8_t key[KEY_SIZE];

  bool drawFlag;

  Sound beep;
} chip8;

void chip8_initialize(chip8* chip);
void chip8_load(chip8* chip, const char* path);
void chip8_emulateCycle(chip8* chip);

typedef void (*Instruction)(chip8* chip);

#endif // !chip_8_h
