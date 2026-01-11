#include "chip8.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <stdio.h>
#include <time.h> 

void chip8_init(CHIP8* cpu)
{
    memset(cpu->memory,0, SIZE_4KB);
    memset(cpu->v, 0, REGISTER_COUNT);
    cpu->pc = INTERPRETER_RESERVED_MEMORY; // sets program counter to 512byte since 0 and 511 are reserved
    // random from current time
    srand(time(NULL));
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

void chip8_cycle(CHIP8* cpu)
{
    Instruction instruction; 

    bool shouldjump = true; 

    // big endian 
    uint16_t opcode = (cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];
    
    // populating instruction struct 
    instruction.type = (opcode & 0XF000) >> 12; 
    instruction.x = (opcode & 0X0F00) >> 8;
    instruction.y = (opcode & 0X00F0) >> 4; 
    instruction.n = (opcode & 0X000F);
    instruction.nn = (opcode & 0X00FF);
    instruction.nnn = (opcode & 0x0FFF);
    
    // buffer for drawing sprite 
    uint8_t sprite_byte;
    uint8_t bit; 

    switch (instruction.type)
    {
        case OP_JUMP:
            cpu->pc = instruction.nnn;
            break;

        case OP_CALL:
            cpu->stack_pointer++; 
            if (cpu->stack_pointer > 15) { return; }
            cpu->stack[cpu->stack_pointer] = cpu->pc;
            cpu->stack_pointer = instruction.nnn;
            break;

        case OP_SKIP_X_EQ_BYTE:
            if (cpu->v[instruction.x] == instruction.nn)
            {
                cpu->pc += 2; 
                shouldjump = false; 
            }
            break;
        
        case OP_SKIP_X_NE_BYTE:
            if (cpu->v[instruction.x] != instruction.nn)
            {
                cpu->pc += 2;
                shouldjump = false;
            }
            break;

        case OP_SKIP_X_EQ_Y:
            if (cpu->v[instruction.x] == cpu->v[instruction.y])
            {
                cpu->pc += 2;
                shouldjump = false; 
            }
            break;

        case OP_LOAD_X_BYTE:
            cpu->v[instruction.x] = instruction.nn; 
            break;

        case OP_ADD_X_BYTE:
            instruction.x += instruction.nn;
            break;

        case OP_ARITHMETIC:
            switch (instruction.n)
            {
                case SUB_OP_LOAD_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.y];
                    break;
                case SUB_OP_OR_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.x] | cpu->v[instruction.y];
                    break;
                case SUB_OP_AND_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.x] & cpu->v[instruction.y];
                    break;
                case SUB_OP_XOR_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.x] ^ cpu->v[instruction.y];
                    break;

                case SUB_OP_ADD_X_Y:
                    uint16_t addition = cpu->v[instruction.x] + cpu->v[instruction.y];
                    if (addition > 255)
                    {
                        // v[15]
                        cpu->v[REGISTER_VF] = 1;
                    }
                    else 
                    {
                        cpu->v[REGISTER_VF] = 0;
                    }
                    cpu->v[instruction.x] = cpu->v[instruction.x] + cpu->v[instruction.y];
                    break;

                case SUB_OP_SUB_X_Y:
                    if (cpu->v[instruction.x] >= cpu->v[instruction.y])
                    {
                        cpu->v[REGISTER_VF] = 1;
                    }
                    else 
                    {
                        cpu->v[REGISTER_VF] = 0;
                    }
                    cpu->v[instruction.x] -= cpu->v[instruction.y];
                    break;

                case SUB_OP_SHR_X:
                    // least_significant = cpu->v[instruction.x] & 1;
                    if ((cpu->v[instruction.x] & 1) == 1)
                    {
                        cpu->v[REGISTER_VF] = 1;
                    }
                    else 
                    {
                        cpu->v[REGISTER_VF] = 0;
                    }
                    cpu->v[instruction.x] = cpu->v[instruction.x] >> 1;
                    break;

                case SUB_OP_SUBN_X_Y:
                    if (cpu->v[instruction.y] >= cpu->v[instruction.x])
                    {
                        cpu->v[REGISTER_VF] = 1;
                    }
                    else 
                    {
                        cpu->v[REGISTER_VF] = 0;
                    }
                    cpu->v[instruction.x] = cpu->v[instruction.y] - cpu->v[instruction.x];
                    break;

                case SUB_OP_SHL_X:
                    // most_significant = cpu->v[instruction.x] >> 7; 
                    if ((cpu->v[instruction.x] >> 7) == 1)
                    {
                        cpu->v[REGISTER_VF] = 1;
                    }
                    else
                    {
                        cpu->v[REGISTER_VF] = 0;
                    }
                        cpu->v[instruction.x] = cpu->v[instruction.x] << 1;
                        break;
                case OP_SKIP_X_NE_Y:
                    if (cpu->v[instruction.x != cpu->v[instruction.y]])
                    {
                        cpu->pc += 2;
                        shouldjump = false; 
                    }
                    break;
                case OP_LOAD_I_ADDR:
                    cpu->i = instruction.nnn; 
                    break;
                case OP_JUMP_V0_ADDR:
                    cpu->pc = instruction.nnn + cpu->v[0];
                    shouldjump = false;
                    break;
                case OP_RANDOM_X_BYTE:
                {
                    // random number between 0 and 255 
                    // just simplified the formula: rand() % (max - min + 1) + min 
                    // since min is zero
                    cpu->v[instruction.x] = (rand() % 256) & instruction.nn; 
                } 
                case OP_DRAW_SPRITE:
                    // loop through bites and check each bit 
                    cpu->v[REGISTER_VF] = 0;
                    for (int i = 0; i < instruction.n; ++i)
                    {
                        sprite_byte = cpu->memory[cpu->i + i];
                        for (int col = 0; col < 8; col++)
                        {
                            // 0x80 is 01000000
                            bit = sprite_byte & (0x80 >> col);
                            if (bit != 0)
                            {
                                // sorry for spaghetiness, but here i apply the formula to converts 2d(grid) into 1d(display in memory)
                                // index = y * width + x
                                // checks for colision
                                if (cpu->display[ ( (( (cpu->v[instruction.y] + i) % 32) * 64)  + (cpu->v[instruction.x] + col) % 64)] == 1)
                                {
                                    cpu->v[REGISTER_VF] = 1;
                                }    
                                // aplly XOR
                                cpu->display[ ( (( (cpu->v[instruction.y] + i) % 32) * 64)  + (cpu->v[instruction.x] + col) % 64)]  ^= 1;
                            }
                        }
                    } 
            }
    }
    // jumps to the next instruction
    if (shouldjump) { cpu->pc += 2; }
}



