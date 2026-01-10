#include "chip8.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void chip8_init(CHIP8* cpu)
{
    memset(cpu->memory,0, SIZE_4KB);
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

void chip8_cycle(CHIP8* cpu)
{
    Instruction instruction; 

    // big endian 
    uint16_t opcode = (cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];
    
    instruction.type = (opcode & 0XF000) >> 12; 
    instruction.x = (opcode & 0X0F00) >> 8;
    instruction.y = (opcode & 0X00F0) >> 4; 
    instruction.n = (opcode & 0X000F);
    instruction.nn = (opcode & 0X00FF);
    instruction.nnn = (opcode & 0x0FFF);

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
            }
            break;
        
        case OP_SKIP_X_NE_BYTE:
            if (cpu->v[instruction.x] != instruction.nn)
            {
                cpu->pc += 2;
            }
            break;

        case OP_SKIP_X_EQ_Y:
            if (cpu->v[instruction.x] == cpu->v[instruction.y])
            {
                cpu->pc += 2;
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
                    uint8_t least_significant = cpu->v[instruction.x] & 1;
                    if (least_significant == 1)
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
                case SUB_OP_SHL_X:
                    uint8_t most_significant = cpu->v[instruction.x] >> 7; 
                    if (most_significant == 1)
                    {
                        cpu->v[REGISTER_VF] = 1;
                    }
                    else
                    {
                        cpu->v[REGISTER_VF] = 0;
                    }
                    cpu->v[instruction.x] = cpu->v[instruction.x] << 1; 
            }
    }
    // jumps to the next instruction
    cpu->pc += 2;
}



