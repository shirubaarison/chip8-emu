# CHIP-8 Emulator

A simple CHIP-8 emulator written in C, using [GLFW](https://www.glfw.org/) for graphics and input.

The "Hello World" of emulators.

## Requirements

- CMake (3.10+)

GLFW and GLAD are included in `/vendor`, so no additional installation is needed.

## Build

```bash
cmake -B build
cd build
make
```

## How to run

```bash
./chip8_emu <path_to_rom>
```

### Example

```bash
./chip8_emu roms/pong.ch8
```

## ROMs

You can find ROMs in

- [CHIP-8 Archive](https://johnearnest.github.io/chip8Archive/)

- [chip8-roms](https://github.com/kripod/chip8-roms)

Be aware that some won't work because the current implementation doesn't support Super CHIP-8.

## Controls
The CHIP-8 keypad is mapped to:
```
1 2 3 C       →    1 2 3 4
4 5 6 D       →    Q W E R
7 8 9 E       →    A S D F
A 0 B F       →    Z X C V
```

## Acknowledgements

- [How to write an emulator (CHIP-8 interpreter)](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)

- [CHIP-8 Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)

- [Interpreting CPU Instructions via an Array of Function Pointers](https://multigesture.net/wp-content/uploads/mirror/zenogais/FunctionPointers.htm)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
