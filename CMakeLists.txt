cmake_minimum_required(VERSION 3.10)
project(CHIP8_Emulator)

# compilation configs
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Dependencies
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Includes 

# Executable
add_executable(chip8 main.c chip8.c)
add_definitions("-Wall -Wextra -g")

# Linking

target_link_libraries(chip8 PRIVATE SDL2::SDL2 SDL2_mixer::SDL2_mixer)

