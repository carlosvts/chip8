#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define SIZE_4KB 4096
#define DISPLAY_SIZE 64 * 32
#define REGISTER_COUNT 16 
#define REGISTER_VF 0xF
#define STACK_SIZE 16 
#define INTERPRETER_RESERVED_MEMORY 0x200
#define FONTSET_STARTPOINT 0x50

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320
#define SCALE 10 

#define CPU_HERTZ 500
#define TIMER_HERTZ 60
// +1 for rouding up and give the CPU a margin
#define CYCLES_PER_FRAME (CPU_HERTZ / TIMER_HERTZ) + 1 

/*
 *  The follow defines was extracted by Cowgod's Chip-8 Thecnical Reference v1.0
 *  http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0
 */

// Main Instruction Types (First Nibble)
#define OP_SYSTEM          0x0  // Instructions like CLEAR_SCREEN and RETURN
#define OP_JUMP            0x1  // 1nnn - Jump to address NNN
#define OP_CALL            0x2  // 2nnn - Call subroutine at NNN
#define OP_SKIP_X_EQ_BYTE  0x3  // 3xkk - Skip next instruction if Vx == KK
#define OP_SKIP_X_NE_BYTE  0x4  // 4xkk - Skip next instruction if Vx != KK
#define OP_SKIP_X_EQ_Y     0x5  // 5xy0 - Skip next instruction if Vx == Vy
#define OP_LOAD_X_BYTE     0x6  // 6xkk - Set Vx = KK
#define OP_ADD_X_BYTE      0x7  // 7xkk - Set Vx = Vx + KK
#define OP_ARITHMETIC      0x8  // 8xy_ - Math operations family
#define OP_SKIP_X_NE_Y     0x9  // 9xy0 - Skip next instruction if Vx != Vy
#define OP_LOAD_I_ADDR     0xA  // Annn - Set Index Register I = NNN
#define OP_JUMP_V0_ADDR    0xB  // Bnnn - Jump to NNN + V0
#define OP_RANDOM_X_BYTE   0xC  // Cxkk - Vx = random byte AND KK
#define OP_DRAW_SPRITE     0xD  // Dxyn - Draw sprite at (Vx, Vy)
#define OP_KEY_INPUT       0xE  // Ex__ - Keyboard skip instructions
#define OP_MISCELLANEOUS   0xF  // Fx__ - Timers, memory, and utility family

// System Sub-types
#define SUB_OP_CLS 0XE0
#define SUB_OP_RET 0xEE

// Arithmetic Sub-types (Last Nibble)
#define SUB_OP_LOAD_X_Y    0x0  // 8xy0 - Vx = Vy
#define SUB_OP_OR_X_Y      0x1  // 8xy1 - Vx = Vx | Vy
#define SUB_OP_AND_X_Y     0x2  // 8xy2 - Vx = Vx & Vy
#define SUB_OP_XOR_X_Y     0x3  // 8xy3 - Vx = Vx ^ Vy
#define SUB_OP_ADD_X_Y     0x4  // 8xy4 - Vx = Vx + Vy (Sets VF Carry)
#define SUB_OP_SUB_X_Y     0x5  // 8xy5 - Vx = Vx - Vy (Sets VF NOT Borrow)
#define SUB_OP_SHR_X       0x6  // 8xy6 - Shift Vx Right by 1
#define SUB_OP_SUBN_X_Y    0x7  // 8xy7 - Vx = Vy - Vx (Sets VF NOT Borrow)
#define SUB_OP_SHL_X       0xE  // 8xyE - Shift Vx Left by 1

// Key Input Sub-types (Last 8 Bits)
#define SUB_OP_SKIP_IF_KEY    0x9E // Ex9E - Skip if key in Vx is pressed
#define SUB_OP_SKIP_IF_NO_KEY 0xA1 // ExA1 - Skip if key in Vx is NOT pressed

// Miscellaneous Sub-types (Last 8 Bits)
#define SUB_OP_LOAD_X_DT      0x07 // Fx07 - Vx = Delay Timer
#define SUB_OP_WAIT_FOR_KEY   0x0A // Fx0A - Wait for key press, store in Vx
#define SUB_OP_SET_DT_X       0x15 // Fx15 - Delay Timer = Vx
#define SUB_OP_SET_ST_X       0x18 // Fx18 - Sound Timer = Vx
#define SUB_OP_ADD_I_X        0x1E // Fx1E - I = I + Vx
#define SUB_OP_LOAD_F_X       0x29 // Fx29 - Point I to character sprite for Vx
#define SUB_OP_STORE_BCD_X    0x33 // Fx33 - Store BCD representation of Vx
#define SUB_OP_STORE_REGS_I   0x55 // Fx55 - Store V0 through Vx in memory[I]
#define SUB_OP_LOAD_REGS_I    0x65 // Fx65 - Read V0 through Vx from memory[I]


typedef struct CHIP8 
{
    uint8_t memory[SIZE_4KB];
    uint8_t display[DISPLAY_SIZE];
    uint8_t v[REGISTER_COUNT];     // registers 
    uint16_t i;                    // index register  
    uint16_t pc;                   // program counter 
    uint16_t stack[STACK_SIZE];    // used to call subroutines and functions
    uint8_t keypad[16];            // chip8 keypad 1 pressed 0 not pressed 
    uint8_t stack_pointer;         // normal stack pointer, points to the top 
    uint8_t delay;                 // delay at 60hz 
    uint8_t sound_timer;            
} CHIP8;


typedef struct 
{
    uint8_t type;  // nibble 1
    uint8_t x;     // nibble 2
    uint8_t y;     // nibble 3 
    uint8_t n;     // nibble 4 
    uint8_t nn;    // last 8bits 
    uint16_t nnn;  // last 16bits 
} Instruction;


// initialize all values and points the stackpointer before the first 512 bits
void chip8_init(CHIP8* cpu);
// load game
void load_rom(CHIP8* cpu, const char* path);
// fetch-decode-execute
void chip8_cycle(CHIP8* cpu);
void chip8_run(CHIP8* cpu);
void update_timers(CHIP8* cpu);
#endif 
