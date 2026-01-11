#include "chip8.h"
#include <stdio.h>

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <path-to-game-rom>\n make sure its a .ch8", argv[0]);
    }
    CHIP8* cpu;
    chip8_init(cpu);
    load_rom(cpu, argv[1]);
    chip8_run(cpu);
}
