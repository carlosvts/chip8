#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#define SIZE_4KB 4096
#define DISPLAY_SIZE 64 * 32
#define REGISTER_COUNT 16 
#define STACK_SIZE 16 
#define INTERPRETER_RESERVED_MEMORY 0x200

typedef struct CHIP8 
{
    uint8_t memory[SIZE_4KB];
    uint8_t display[DISPLAY_SIZE];
    uint8_t v[REGISTER_COUNT];     // registers 
    uint16_t i;                    // index register  
    uint16_t pc;                   // program counter 
    uint16_t stack[STACK_SIZE];    // used to call subroutines and functions
    uint8_t stack_pointer;         // normal stack pointer 
    uint8_t delay;                 // delay at 60hz 
    uint8_t sound_timer;            
} CHIP8;


// initialize all values and points the stackpointer before the first 512 bits
void chip8_init(CHIP8* cpu);
// load game
void load_rom(CHIP8* cpu, char* path);
#endif 
