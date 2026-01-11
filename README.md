# CHIP-8 Emulator (Interpreter)

A low-level system emulator developed in C that implements a virtual CPU, memory management, and peripheral IO to interpret and execute CHIP-8 ROMS.

This project implements a complete virtual machine architecture, including a Fetch-Decode-Execute cycle, a custom fontset, and integration with SDL2 for graphics, sound (coin.wav), and keyboard input.

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)![SDL2](https://img.shields.io/badge/SDL2-5B2D90?style=for-the-badge&logo=sdl&logoColor=white)

---

## About the Project
An educational systems programming project designed to demystify how CPUs actually work "under the hood." This implementation handles raw opcodes, big-endian memory fetching, 8-bit arithmetic logic, and stack-based subroutine management.

The engine uses a **Big-Endian** memory architecture to fetch 16-bit opcodes, which are then decoded into nibbles (X, Y, N, NN, NNN) to execute specific CPU instructions.

**Main Objectives:**
* Implement a **Fetch-Decode-Execute** cycle.
* Manage **4KB of Virtual Memory** and 16 8-bit Registers ($V_0$ to $V_F$).
* Implement **Subroutine Support** via a 16-level Stack.
* Handle **Pixel-based Graphics** using XOR logic for sprite drawing.
* Implement **Delay & Sound Timers** synchronized at 60Hz.
* Integrate **SDL2** for hardware-accelerated rendering and audio feedback.

## Technologies

* **Language:** C (ISO C11)
* **Graphics/Input:** SDL2 (Simple DirectMedia Layer)
* **Audio:** SDL_mixer
* **Concepts:** Opcode Decoding, Big-Endian Fetching, Bitwise Operations, CPU State Machines.

## How to Build and Run

### Prerequisites
Before compiling, ensure you have the development libraries for SDL2 and SDL2_mixer installed. On Fedora, run:

sudo dnf install SDL2-devel SDL2_mixer-devel cmake make gcc

You can check SDL2 documentation for installation in other distros and systems. 

### Compiling
This project uses CMake for an out-of-source build to keep the repository clean. This means all temporary compilation files stay inside a dedicated folder:

1. mkdir build
2. cd build
3. cmake ..
4. make

### Running a Game
The emulator requires a CHIP-8 ROM to run. Acquire a game file (e.g., Pong.ch8), place it in the roms/ folder, and pass the path as a command-line argument:

From the project root:
./build/chip8 ./roms/YourGame.ch8

**Note on Paths:** If you execute the binary from inside the build/ folder, you must adjust the relative path to the ROM (e.g., ../roms/YourGame.ch8). The program uses your current terminal location (CWD) to resolve where the file is.

### Audio Customization
The emulator supports custom audio feedback. You can change the sounds played by the system by replacing the files in the `assets/` folder.

To ensure compatibility:
* Use only `.wav` files.
* The files must be located inside the `assets/` directory.
* Ensure the filename matches what is defined in your `Mix_LoadWAV` call (default is `coin.wav`).

If the emulator cannot find the audio file at the specified path, the sound subsystem will fail to load the chunk, but the CPU emulation will continue to run.

## CPU Architecture Logic

### Instruction Decoding (The Brain)
* **Fetch**: Opcodes are 16-bit, formed by joining two 8-bit memory locations: `(memory[pc] << 8) | memory[pc + 1]`.
* **Decode**: Uses bitwise masks and shifts to extract operands (X, Y, NN, NNN).
* **Execute**: A massive switch-case handles operations from simple jumps to complex sprite drawing with collision detection.

### Memory & Registers
* **Memory Layout**: 512 bytes reserved for the interpreter (containing the Fontset), with ROMs loaded at `0x200`.
* **Registers**: 16 general-purpose 8-bit registers. $V_F$ is specifically used as a flag for carry, borrow, and collision detection.

### Graphics & Collision
* **Drawing**: Sprites are drawn using XOR logic. If a pixel is flipped from 1 to 0, $V_F$ is set to 1, signaling a collision (essential for games like Pong).

---

## Technical Challenges & Debugging

During development, several low-level hurdles were overcome:

* **Operator Precedence & Bitmasks**: Early bugs in opcode extraction were caused by incorrect bitwise masks. Ensuring `(opcode & 0x0F00) >> 8` was handled correctly was vital for register addressing.
* **Garbage Values (The "Ghost" in the Machine)**: A major breakthrough occurred when implementing a full `memset` on the CPU struct. Uninitialized memory in the `keypad` and `display` arrays caused erratic behavior and visual corruption in ROMs like Pong.
* **Deprecated ROMs**: Testing with older ROMs revealed timing and instruction differences. Switching to modern debug suites like **Timendus** allowed for precise ALU and flag validation.
* **The Stack Pointer Trap**: Fixing the `CALL` instruction to save `PC + 2` instead of `PC` was the key to allowing subroutines to return correctly without infinite loops and understanding better the call stack.

## Resources

* **Cowgod's Chip-8 Technical Reference**: [The definitive guide for CHIP-8 opcodes](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0)
* **SDL2 Wiki**: [Documentation for rendering and input handling](https://wiki.libsdl.org/SDL2/FrontPage)
* **CHIP-8 Game Archive**: [Collection of ROMs for testing](https://johnearnest.github.io/chip8Archive/)

## Final Thoughts

Even though CHIP-8 is considered the "Hello World" of VMs and emulators, I had a great time understanding how instructions work under the hood. Building this project provided a hands-on experience with low-level concepts that are often abstracted away, making me appreciate the intricate dance between hardware and software.

I've used a lot of pre processed variables in the intent of make the code more legible, maybe for processess like this emulator is better to just hardcode it, but, since this is a project that his main goal is learning, using this variables make my understanding (and debug) a lot easier.
