#include "Machine.h"
#include <iostream>
#include <cstring>
#include <fstream>

Machine::Machine()
{
    //ctor
}

Machine::~Machine()
{
    //dtor
}

bool Machine::init() {
    sp = 0;
    memset(_stack, 0, sizeof(_stack));
    opcode = 0;
    pc = 0x200; //start of ROM
    memset(memory, 0, sizeof(memory));
    const std::uint8_t fontset[80] = //thank you reference 1
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
    };
    for(int i=0; i<80; i++) memory[i] = fontset[i];
    draw = true;
    I = 0;
    memset(display, 0, sizeof(display));
}

bool Machine::load(std::string path) {
    std::ifstream in;
    in.open(path);
    char* buffer;
    in.seekg(0, in.end);
    int length = in.tellg();
    /*
    MEMORY MAP (reference 1)
    0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    0x200-0xFFF - Program ROM and work RAM
    */
    if(length > 3584) {
        std::cout << "FILE TOO BIG, ABORTING.\n";
        return false;
    }
    in.seekg(0, in.beg);
    in.read(buffer, length);
    in.close();
    for(int i=0; i<length; i++) {
        memory[i + 512] = buffer[i];
    }
    init();
    return true;
}

void Machine::cycle() {
    opcode = memory[pc] << 8 | memory[pc+1]; //16 bits
    //consider starting letter
    switch(opcode & 0xF000) {
    case 0x0000:
        switch(opcode & 0x000F) {
        case 0x0000:
            //00E0
            memset(display, 0, sizeof(display));
        break;
        case 0x000E:
            //00EE
            //additional note: just think of C++ callstack when function returns
            sp--;
            pc = _stack[sp];
            pc += 2; //thank you reference 1
        break;
        }
    break;
    }
}
