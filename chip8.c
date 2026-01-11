#include "chip8.h"

#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_mixer.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <stdio.h>
#include <sys/types.h>
#include <time.h> 

#include <SDL2/SDL.h>

void chip8_init(CHIP8* cpu)
{
    const uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
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
    
    memset(cpu, 0, sizeof(CHIP8));
    //  memset(cpu->memory,0, SIZE_4KB);
    //  memset(cpu->v, 0, REGISTER_COUNT);
    // random from current time
    srand(time(NULL));
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    for (int i = 0; i < 80; ++i)
    {
        cpu->memory[FONTSET_STARTPOINT + i] = fontset[i];
    }
    cpu->pc = INTERPRETER_RESERVED_MEMORY; // sets program counter to 512byte since 0 and 511 are reserved
    
}

void load_rom(CHIP8* cpu, const char *path)
{
    FILE* file_pointer = fopen(path, (const char*) "rb");
    if (file_pointer == NULL)
    {
        perror("CHIP8: Error while opening file: ");
        return;  
    }
    fseek(file_pointer, 0, SEEK_END);
    // since .ch8 stores raw bytes data, we can check the size of the file using ftell
    long size = ftell(file_pointer);
    if (size >= SIZE_4KB - INTERPRETER_RESERVED_MEMORY)
    {
        printf("CHIP8: Your file is too big for a chip 8 engine! terminating.\n");
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
    uint16_t opcode = ((cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1]);
    
    bool shouldjump = true; 
    
    // populating instruction struct 
    instruction.type = (opcode & 0XF000) >> 12; 
    instruction.x = (opcode & 0X0F00) >> 8;
    instruction.y = (opcode & 0X00F0) >> 4; 
    instruction.n = (opcode & 0X000F);
    instruction.nn = (opcode & 0X00FF);
    instruction.nnn = (opcode & 0x0FFF);
    
    // buffer for drawing sprite 
    uint8_t sprite_byte;
    
    switch (instruction.type)
    {  

        case OP_SYSTEM:
            if (instruction.nn == SUB_OP_CLS)
            {
                memset(cpu->display, 0, sizeof(cpu->display));
                break;
            }
            else if(instruction.nn == SUB_OP_RET)
            {
                cpu->stack_pointer--; 
                cpu->pc = cpu->stack[cpu->stack_pointer];
                shouldjump = false;
                break;
            }
            break;
        case OP_JUMP:
            // printf("SALTO DETECTADO: Indo para 0x%03X\n", instruction.nnn);
            cpu->pc = instruction.nnn;
            shouldjump = false;
            break;

        case OP_CALL:
            if (cpu->stack_pointer > 15) { return; }
            cpu->stack[cpu->stack_pointer] = cpu->pc + 2; // + 2 because its next instruction
            cpu->stack_pointer++;
            cpu->pc = instruction.nnn;
            shouldjump = false;
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
            cpu->v[instruction.x] = (uint8_t) instruction.nn; 
            break;

        case OP_ADD_X_BYTE:
            cpu->v[instruction.x] += (uint8_t) instruction.nn;
            break;

        case OP_ARITHMETIC:
            switch (instruction.n)
            {
                case SUB_OP_LOAD_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.y];
                    break;
                case SUB_OP_OR_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.x] | cpu->v[instruction.y];
                    cpu->v[REGISTER_VF] = 0;
                    break;
                case SUB_OP_AND_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.x] & cpu->v[instruction.y];
                    cpu->v[REGISTER_VF] = 0; 
                    break;
                case SUB_OP_XOR_X_Y:
                    cpu->v[instruction.x] = cpu->v[instruction.x] ^ cpu->v[instruction.y];
                    cpu->v[REGISTER_VF] = 0;
                    break;

                case SUB_OP_ADD_X_Y:
                    {   
                        uint8_t val_x = cpu->v[instruction.x];
                        uint8_t val_y = cpu->v[instruction.y];
                        uint16_t addition = (uint16_t) val_x + (uint16_t) val_y;
                        cpu->v[instruction.x] = (uint8_t) (addition & 0xFF); // truncate for 8 bit
                        if (addition > 255)
                        {
                            // v[15]
                            cpu->v[REGISTER_VF] = 1;
                        }
                        else 
                        {
                            cpu->v[REGISTER_VF] = 0;
                        }
                        break;
                    }

                case SUB_OP_SUB_X_Y:
                    {
                        uint8_t val_x = cpu->v[instruction.x];
                        uint8_t val_y = cpu->v[instruction.y];
                        uint8_t not_borrow = (val_x >= val_y) ? 1 : 0;
                    
                        cpu->v[instruction.x] = val_x - val_y;
                        cpu->v[REGISTER_VF] = not_borrow;
                        break;
                    }

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
                    if ((cpu->v[instruction.x] >> 7) & 0x1)
                    {
                        cpu->v[REGISTER_VF] = 1;
                    }
                    else
                    {
                        cpu->v[REGISTER_VF] = 0;
                    }
                        cpu->v[instruction.x] = cpu->v[instruction.x] << 1;
                    break;
            } 
            break;

        case OP_SKIP_X_NE_Y:
            if (cpu->v[instruction.x] != cpu->v[instruction.y])
            {
                cpu->pc += 2;
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
            // random number between 0 and 255 
            // just simplified the formula: rand() % (max - min + 1) + min 
            // since min is zero
            cpu->v[instruction.x] = (rand() % 256) & instruction.nn;
            break; 
        case OP_DRAW_SPRITE:
        {
            // loop through bites and check each bit 
            cpu->v[REGISTER_VF] = 0;
            for (int i = 0; i < instruction.n; ++i)
            {
                sprite_byte = cpu->memory[cpu->i + i];
                for (int col = 0; col < 8; col++)
                {
                    if ((sprite_byte & (0x80 >> col)) != 0)
                    {
                        uint8_t x = (cpu->v[instruction.x] + col) % 64; // % 64
                        uint8_t y = (cpu->v[instruction.y] + i) % 32; // % 32
                        
                        uint16_t index = (y * 64) + x;
                        if (index < 2048)
                        {
                                if (cpu->display[index] == 1)
                                {
                                    cpu->v[REGISTER_VF] = 1;
                                }
                                cpu->display[index] ^= 1;
                        }
                        else { printf("calcualtion error in index! \n"); }
                    }
                }
            }
            break;
        }    
        case OP_KEY_INPUT:
            switch(instruction.nn)
            {
                case SUB_OP_SKIP_IF_KEY:
                    if (cpu->keypad[cpu->v[instruction.x]] != 0)
                    {
                        cpu->pc += 2;
                    }
                    break;
                case SUB_OP_SKIP_IF_NO_KEY:
                    if (cpu->keypad[cpu->v[instruction.x]] == 0)
                    {
                        cpu->pc += 2;
                    }
            }
            break;

        case OP_MISCELLANEOUS:
            switch(instruction.nn)
            {
                case SUB_OP_LOAD_X_DT:
                {
                    cpu->v[instruction.x] = cpu->delay;
                    break;
                }
                case SUB_OP_WAIT_FOR_KEY:
                {   
                    bool key_pressed = false;
                    for (int i = 0; i < 16; ++i)
                    {
                        // if pressed
                        if (cpu->keypad[i] != 0)
                        {
                            cpu->v[instruction.x] = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    // if not pressed, retracts pc so he will check this instruction again
                    if (!key_pressed)
                    {
                        cpu->pc -= 2;    
                    }
                    break; 
                }
                case SUB_OP_SET_DT_X:
                {
                    cpu->delay = cpu->v[instruction.x];
                    break;
                }
                case SUB_OP_SET_ST_X:
                {
                    cpu->sound_timer = cpu->v[instruction.x];
                    break;
                }
                case SUB_OP_ADD_I_X:
                {
                    cpu->i += cpu->v[instruction.x];
                    break;
                }
                case SUB_OP_LOAD_F_X:
                {
                    //  our write in the fontset costs 5 bytes
                    cpu->i = FONTSET_STARTPOINT + (cpu->v[instruction.x] * 5); 
                    break;
                }
                case SUB_OP_STORE_BCD_X:
                {
                    uint8_t value = cpu->v[instruction.x];
                    
                    // get hundreds
                    cpu->memory[cpu->i] = value / 100;
                    // get tens
                    cpu->memory[cpu->i + 1] = (value / 10) % 10;
                    // get unity
                    cpu->memory[cpu->i + 2] = value % 10;
                    break;
                }
                case SUB_OP_STORE_REGS_I:
                {
                        // reg stands for register 
                    for (int reg = 0; reg <= instruction.x; ++reg)
                    {
                        cpu->memory[cpu->i + reg] = cpu->v[reg];
                    }
                    break;  
                }
                case SUB_OP_LOAD_REGS_I:
                {
                    for (int reg = 0; reg <= instruction.x; ++reg)
                    {
                        cpu->v[reg] = cpu->memory[cpu->i + reg]; 
                    }
                    break;
                }
            }
            break;
    }
    // jumps to the next instruction
    if (shouldjump) { cpu->pc += 2; }
}

void update_timers(CHIP8* cpu)
{
    if (cpu->delay > 0)
    {
        cpu->delay--;
    }
    if (cpu->sound_timer > 0)
    {
        cpu->sound_timer--;
    }
}

void render(CHIP8* cpu, SDL_Renderer* renderer)
{
    // black background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // draw white pixel
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // display array
    for (int y = 0; y < 32; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            // if has a value, it means it has a pixel 
            if (cpu->display[y * 64 + x])
            {
                SDL_Rect rect;
                rect.x = x * SCALE;
                rect.y = y * SCALE;
                rect.h = SCALE;
                rect.w = SCALE;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // updates screen 
    SDL_RenderPresent(renderer);
}

void chip8_run(CHIP8* cpu)
{
    bool game_running = true;

    SDL_Window* pwindow = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pwindow) { printf("Error while creating SDL window\n%s", SDL_GetError()); return; }
    
    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow, -1, 0);
    if (!prenderer) { printf("Error while generating renderer\n%s", SDL_GetError()); return; }

    uint32_t last_tick = SDL_GetTicks();
    
    Mix_Chunk* coin_sound = Mix_LoadWAV("assets/coin.wav");
    
    while (game_running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                printf("exiting chip8!\n");
                game_running = false;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) 
            {
                uint8_t state = (event.type == SDL_KEYDOWN) ? 1 : 0;
                
                switch (event.key.keysym.sym) 
                {
                    case SDLK_x: cpu->keypad[0] = state; break;
                    case SDLK_1: cpu->keypad[1] = state; break;
                    case SDLK_2: cpu->keypad[2] = state; break;
                    case SDLK_3: cpu->keypad[3] = state; break;
                    case SDLK_q: cpu->keypad[4] = state; break;
                    case SDLK_w: cpu->keypad[5] = state; break;
                    case SDLK_e: cpu->keypad[6] = state; break;
                    case SDLK_a: cpu->keypad[7] = state; break;
                    case SDLK_s: cpu->keypad[8] = state; break;
                    case SDLK_d: cpu->keypad[9] = state; break;
                    case SDLK_z: cpu->keypad[10] = state; break;
                    case SDLK_c: cpu->keypad[11] = state; break;
                    case SDLK_4: cpu->keypad[12] = state; break;
                    case SDLK_r: cpu->keypad[13] = state; break;
                    case SDLK_f: cpu->keypad[14] = state; break;
                    case SDLK_v: cpu->keypad[15] = state; break;
                }
                if (cpu->keypad[15] == 1) { printf("F pressed\n"); }
            } 
        }
        
        uint32_t current_time = SDL_GetTicks();

        if (current_time - last_tick >= 16) // 
        {
            for (int i = 0; i < CYCLES_PER_FRAME; i++)
            {
                chip8_cycle(cpu);
            }

            update_timers(cpu);
            if (cpu->sound_timer > 0)
            {
                if (!Mix_Playing(0))
                {
                    Mix_PlayChannel(0, coin_sound, -1); // -1 for loop
                }
                else
                {
                    Mix_HaltChannel(0); // stops when timer is over
                }
            }
            render(cpu, prenderer);
            last_tick = current_time;
        }
        SDL_Delay(1);
    }
    SDL_DestroyRenderer(prenderer);
    SDL_DestroyWindow(pwindow);
}


