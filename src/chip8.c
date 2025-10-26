#include "chip8.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define FONT_SET 80

uint8_t chip8_fontset[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,      // 0
    0x20, 0x60, 0x20, 0x20, 0x70,      // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_initialize(chip8* chip) {
  // initialize registers and memory once
  chip->pc = 0x200;
  chip->I = 0;
  chip->sp = 0;
  chip->opcode = 0;

  memset(chip->gfx, 0x0, sizeof(chip->gfx));
  memset(chip->stack, 0x0, sizeof(chip->stack));
  memset(chip->V, 0x0, sizeof(chip->V));
  memset(chip->memory, 0x0, sizeof(chip->memory));
  memset(chip->key, 0x0, sizeof(chip->key));

  // load fontset
  for (int i = 0; i < FONT_SET; i++) {
    chip->memory[i] = chip8_fontset[i];
  }

  chip->delay_timer = 0;
  chip->sound_timer = 0;
  chip->drawFlag = true;

  srand(time(NULL));
}

void chip8_load(chip8* chip, const char* path) {
#define PROGRAM_START 0x200

  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(1);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  if (fileSize <= 0 || fileSize > MAX_MEMORY - PROGRAM_START) {
    fprintf(stderr, "Invalid file size for \"%s\".\n", path);
    exit(1);
  }

  char* buffer = (char*)malloc(fileSize);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    fclose(file);
    exit(1);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\". \n", path);
    exit(1);
  }

  for (size_t i = 0; i < fileSize; i++) {
    chip->memory[i + PROGRAM_START] = buffer[i];
  }

  free(buffer);
  fclose(file);

#undef PROGRAM_START
}

Instruction chip8_table[];
Instruction chip8_arithmetic[];

void chip8_emulateCycle(chip8* chip) {
#define CHIP8_FETCH(chip) \
  ((uint16_t)(((chip)->memory[(chip)->pc] << 8) | ((chip)->memory[(chip)->pc + 1])))

  chip->opcode = CHIP8_FETCH(chip);
  chip->pc += 2;

  Instruction instruction = chip8_table[(chip->opcode & 0xF000) >> 12];
  instruction(chip);

  // update timers
  if (chip->delay_timer > 0)
    --chip->delay_timer;

  if (chip->sound_timer > 0) {
    if (chip->sound_timer == 1)
      fprintf(stdout, "BEEP!\n");
    --chip->sound_timer;
  }

#undef CHIP8_FETCH
}

static void chip8_NULL(chip8* chip) {
  fprintf(stderr, "Invalid opcode: 0x%04X.\n", chip->opcode);
}

// 00E0: clears the screen
static void chip8_00E0(chip8* chip) {
  memset(chip->gfx, 0x0, sizeof(chip->gfx));
  chip->drawFlag = true;
}

#define GET_X(opcode) (((opcode) & 0x0F00) >> 8)
#define GET_Y(opcode) (((opcode) & 0x00F0) >> 4)
#define GET_N(opcode) ((opcode) & 0x000F)
#define GET_NN(opcode) ((opcode) & 0x00FF)
#define GET_NNN(opcode) ((opcode) & 0x0FFF)

// 00EE: returns from a subroutine
static void chip8_00EE(chip8* chip) {
  chip->sp--;
  chip->pc = chip->stack[chip->sp];
}

// 1NNN: jumps to address NNN
static void chip8_1NNN(chip8* chip) {
  uint16_t NNN = GET_NNN(chip->opcode);
  chip->pc = NNN;
}

// 2NNN: calls subroutine at NNN
static void chip8_2NNN(chip8* chip) {
  uint16_t NNN = GET_NNN(chip->opcode);

  chip->stack[chip->sp] = chip->pc;
  chip->sp++;
  chip->pc = NNN;
}

// 3XNN: skips next instruction if VX equals NN
static void chip8_3XNN(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t NN = GET_NN(chip->opcode);

  if (chip->V[X] == NN)
    chip->pc += 2;
}

// 4XNN: skips the next instruction if VX doesn't equal NN
static void chip8_4XNN(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t NN = GET_NN(chip->opcode);

  if (chip->V[X] != NN)
    chip->pc += 2;
}

// 5XY0: skips the next instruction if VX equals VY
static void chip8_5XY0(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  if (chip->V[X] == chip->V[Y])
    chip->pc += 2;
}

// 6XNN: sets VX to NM
static void chip8_6XNN(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t NN = GET_NN(chip->opcode);

  chip->V[X] = NN;
}

// 7XNN: adds NN to VX (carry flag is not changed)
static void chip8_7XNN(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t NN = GET_NN(chip->opcode);

  chip->V[X] += NN;
}

// 8XY0: sets VX to the value of VY
static void chip8_8XY0(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  chip->V[X] = chip->V[Y];
}

// 8XY1: sets VX to VX or VY (bitwise OR op)
static void chip8_8XY1(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  chip->V[X] |= chip->V[Y];
}

// 8XY2: sets VX to VX and VY (bitwise AND op)
static void chip8_8XY2(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  chip->V[X] &= chip->V[Y];
}

// 8XY3: sets VX to VX xor VY
static void chip8_8XY3(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  chip->V[X] ^= chip->V[Y];
}

// 8XY4: adds VY to VX, VF is set to 1 when there's a overflow
static void chip8_8XY4(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  uint16_t sum = chip->V[X] + chip->V[Y];
  chip->V[0xF] = (sum > 0xFF) ? 1 : 0;
  chip->V[X] = sum & 0xFF;
}

// 8XY5: VY is subtracted from VX, VF is set to 0 when there's a underflow
static void chip8_8XY5(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  chip->V[0xF] = (chip->V[X] >= chip->V[Y]) ? 1 : 0;
  chip->V[X] = chip->V[X] - chip->V[Y];
}

// 8XY6: shifts VX to the right by 1, store LSB in VF
static void chip8_8XY6(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
 
  chip->V[0xF] = chip->V[X] & 0x1; // LSB (bit 0)

  chip->V[X] >>= 1;
}

// 8XY7: sets VX to VY minus VX, VF is set to 0 if there's a underflow
static void chip8_8XY7(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  chip->V[0xF] = (chip->V[Y] >= chip->V[X]) ? 1 : 0;
  chip->V[X] = chip->V[Y] - chip->V[X];
}

// 8XYE: shifts VX to the left by 1, store MSB in VF
static void chip8_8XYE(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
 
  chip->V[0xF] = (chip->V[X] >> 7) & 0x1; // MSB (bit 7)

  chip->V[X] <<= 1;
}

// uses the second table
static void chip8_8XXX(chip8* chip) {
  chip8_arithmetic[chip->opcode & 0x00F](chip);
}

// 9XY0: skips the next instruction if VX doesn't equal VY
static void chip8_9XY0(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t Y = GET_Y(chip->opcode);

  if (chip->V[X] != chip->V[Y])
    chip->pc += 2;
}

// ANNN: sets I to the address NNN
static void chip8_ANNN(chip8* chip) { chip->I = chip->opcode & 0x0FFF; }

// BNNN: jumps to the address NNN plus V0
static void chip8_BNNN(chip8* chip) {
  uint16_t NNN = GET_NNN(chip->opcode);

  chip->pc = chip->V[0] + NNN;
}

// CXNN: sets VX to the result of an operation on a random number and NN
static void chip8_CXNN(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  uint8_t NN = GET_NN(chip->opcode);

  chip->V[X] = (rand() & 0xFF) & NN;
}

// DXYN: draws a sprite at coordinate (VX, VY) with a height of N
// VF is set to 1 if any screen pixels are flipped from set to unset
static void chip8_DXYN(chip8* chip) {
  uint8_t X = chip->V[GET_X(chip->opcode)];
  uint8_t Y = chip->V[GET_Y(chip->opcode)];
  uint8_t height = chip->opcode & 0x000F;

  uint8_t pixel;

  chip->V[0xF] = 0;
  for (int yline = 0; yline < height; yline++) {
    pixel = chip->memory[chip->I + yline];

    for (int xline = 0; xline < 8; xline++) {
      if ((pixel & (0x80 >> xline)) != 0) {
        uint16_t x = (X + xline) % WIDTH;
        uint16_t y = (Y + yline) % HEIGHT;
        uint16_t index = x + (y * WIDTH);

        // check for collision (pixel being turned off)
        if (chip->gfx[index] == 1) {
          chip->V[0xF] = 1;
        }

        chip->gfx[index] ^= 1;
      }
    }
  }
  chip->drawFlag = true;
}

// EX9E: skips the next instruction if the key stored in VX is pressed
static void chip8_EX9E(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  if (chip->key[chip->V[X]] != 0)
    chip->pc += 2;
}

// EXA1: skips the next instruction if the key stored in VX is not pressed
static void chip8_EXA1(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);
  if (chip->key[chip->V[X]] == 0)
    chip->pc += 2;
}

// FX0A: a key pressed is awaited, and then stored in VX
static void chip8_FX0A(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);

  bool key_pressed = false;

  for (int i = 0; i < KEY_SIZE; i++) {
    if (chip->key[i] != 0) {
      chip->V[X] = i;
      key_pressed = true;
      break;
    }
  }

  if (!key_pressed)
    chip->pc -= 2;
}

// FX1E: adds VX to I. VF is not affected
static void chip8_FX1E(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);

  chip->I += chip->V[X];
}

// FX07: sets VX to the value of the delay timer
static void chip8_FX07(chip8 *chip) {
  uint8_t X = GET_X(chip->opcode);

  chip->V[X] = chip->delay_timer;
}

// FX15: sets the delay timer to VX
static void chip8_FX15(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);

  chip->delay_timer = chip->V[X];
}

// FX18: sets the sound timer to VX
static void chip8_FX18(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);

  chip->sound_timer = chip->V[X];
}

// FX29: sets I to the location of the sprite for the character in VX
// characters 0-F (in hexadecimal) are represented by a 4x5 font
static void chip8_FX29(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);

  chip->I = chip->V[X] * 0x5;
}

// FX33: stores the binary-coded decimal representation of VX
static void chip8_FX33(chip8* chip) {
  uint8_t X = GET_X(chip->opcode);

  chip->memory[chip->I]     = chip->V[X] / 100;
  chip->memory[chip->I + 1] = (chip->V[X] / 10) % 10;
  chip->memory[chip->I + 2] = chip->V[X] % 10;
}

// FX55: stores from V0 to VX (including VX) in memory, starting at address I
static void chip8_FX55(chip8 *chip) {
  uint8_t X = GET_X(chip->opcode);

  for (int i = 0; i <= X; i++)
    chip->memory[chip->I + i] = chip->V[i];

  // chip->I += X + 1;
}

// FX65: fills from V0 to VX (including VX) in memory, starting at address
static void chip8_FX65(chip8 *chip) {
  uint8_t X = GET_X(chip->opcode);

  for (int i = 0; i <= X; i++)
    chip->V[i] = chip->memory[chip->I + i];

  // chip->I += X + 1;
}

static void chip8_0XXX(chip8* chip) {
  switch (chip->opcode & 0x00FF) {
    case 0x00E0:
      chip8_00E0(chip);
      break;
    case 0x00EE:
      chip8_00EE(chip);
      break;
    default: 
      chip8_NULL(chip);
      break;
  }
}

static void chip8_EXXX(chip8* chip) {
  switch (chip->opcode & 0x00FF) {
    case 0x009E:
      chip8_EX9E(chip);
      break;
    case 0x00A1:
      chip8_EXA1(chip);
      break;

    default:
      chip8_NULL(chip);
      break;
  }
}

static void chip8_FXXX(chip8* chip) {
  switch (chip->opcode & 0x00FF) {
    case 0x000A:
      chip8_FX0A(chip);
      break;
    case 0x001E:
      chip8_FX1E(chip);
      break;
    case 0x0007:
      chip8_FX07(chip);
      break;
    case 0x0015:
      chip8_FX15(chip);
      break;
    case 0x0018:
      chip8_FX18(chip);
      break;
    case 0x0029:
      chip8_FX29(chip);
      break;
    case 0x0033:
      chip8_FX33(chip);
      break;
    case 0x0055:
      chip8_FX55(chip);
      break;
    case 0x0065:
      chip8_FX65(chip);
      break;

    default:
      chip8_NULL(chip);
      break;
  }
}

Instruction chip8_table[] = {
  chip8_0XXX,       // 0x0XXX - Group 0 instructions
  chip8_1NNN,       // 0x1XXX - JP addr
  chip8_2NNN,       // 0x2XXX - CALL addr  
  chip8_3XNN,       // 0x3XXX - SE Vx, byte
  chip8_4XNN,       // 0x4XXX - SNE Vx, byte
  chip8_5XY0,       // 0x5XXX - SE Vx, Vy
  chip8_6XNN,       // 0x6XXX - LD Vx, byte
  chip8_7XNN,       // 0x7XXX - ADD Vx, byte
  chip8_8XXX,       // 0x8XXX - Arithmetic operations (uses second table!)
  chip8_9XY0,       // 0x9XXX - SNE Vx, Vy
  chip8_ANNN,      // 0xAXXX - LD I, addr
  chip8_BNNN,      // 0xBXXX - JP V0, addr
  chip8_CXNN,      // 0xCXXX - RND Vx, byte
  chip8_DXYN,      // 0xDXXX - DRW Vx, Vy, nibble
  chip8_EXXX,      // 0xEXXX - SKP Vx / SKNP Vx
  chip8_FXXX,      // 0xFXXX - LD Vx, DT / LD Vx, K / etc.
  chip8_NULL
};

Instruction chip8_arithmetic[] = {
  chip8_8XY0,       // 0x8XY0 - LD Vx, Vy
  chip8_8XY1,       // 0x8XY1 - OR Vx, Vy
  chip8_8XY2,       // 0x8XY2 - AND Vx, Vy
  chip8_8XY3,       // 0x8XY3 - XOR Vx, Vy
  chip8_8XY4,       // 0x8XY4 - ADD Vx, Vy
  chip8_8XY5,       // 0x8XY5 - SUB Vx, Vy
  chip8_8XY6,       // 0x8XY6 - SHR Vx {, Vy}
  chip8_8XY7,       // 0x8XY7 - SUBN Vx, Vy
  chip8_8XYE,       // 0x8XYE - SHL Vx {, Vy}
  chip8_NULL,       // 0x8XY9
  chip8_NULL,      // 0x8XYA
  chip8_NULL,      // 0x8XYB
  chip8_NULL,      // 0x8XYC
  chip8_NULL,      // 0x8XYD
  chip8_NULL,      // 0x8XYE
  chip8_NULL       // 0x8XYF
};

#undef GET_X
#undef GET_Y
#undef GET_N
#undef GET_NN
#undef GET_NNN
