#include "chip8.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void chip8_init(CHIP8* cpu)
{
    memset(cpu->memory, 0, SIZE_4KB);
    memset(cpu->v, 0, REGISTER_COUNT);
    cpu->pc = INTERPRETER_RESERVED_MEMORY; // sets program counter to 512byte since 0 and 511 are reserved
}

void load_rom(CHIP8* cpu, char *path)
{
    FILE* file_pointer = fopen(path, (const char*) "rb");
    if (file_pointer == NULL)
    {
        perror("Error while opening file: ");
        return;  
    }
    fseek(file_pointer, 0, SEEK_END);
    // since .ch8 stores raw bytes data, we can check the size of the file using ftell
    long size = ftell(file_pointer);
    if (size >= SIZE_4KB - INTERPRETER_RESERVED_MEMORY)
    {
        printf("Your file is too big for a chip 8 engine! terminating.\n");
        return; 
    }
    // move filepointer back to the begining 
    rewind(file_pointer);

    fread(cpu->memory + cpu->pc, sizeof(uint8_t), size, file_pointer);
}

