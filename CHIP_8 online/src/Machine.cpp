#include "Machine.h"
#include <iostream>
#include <cstring>
#include <fstream>

//references
//1 - http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
//2 - http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
//3 - https://en.wikipedia.org/wiki/CHIP-8#Opcode_table

//other links that the comments refer to
//https://stackoverflow.com/questions/2711522/what-happens-if-i-assign-a-negative-value-to-an-unsigned-variable

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
    return true;
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
    //holder variables
    std::uint16_t reg;
    std::uint16_t val;
    std::uint16_t X;
    std::uint16_t Y;

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
    case 0x1000:
        //1NNN
        //additional note: NNN just means a placeholder value
        pc = (opcode & 0x0FFF);
        //no need to increment pc here since goto is called
    break;
    case 0x2000:
        //2NNN
        //additional note: think of what C++ callstack does when calling function
        _stack[sp] = pc; //push current pointer to stack
        sp++; //increment stack
        pc = (opcode & 0x0FFF); //call function at 0NNN
    break;
    case 0x3000: //verify if this is correct later
        //3XNN
        reg = (opcode & 0x0F00) >> (2*4); //shift 2 places in hex
        val = (opcode & 0x00FF);
        pc += 2;
        if(V[reg] == val) pc += 2;
    break;
    case 0x4000:
        //4XNN
        reg = (opcode & 0x0F00) >> (2*4); //shift 2 places in hex
        val = (opcode & 0x00FF);
        pc += 2;
        if(V[reg] != val) pc += 2;
    break;
    case 0x5000:
        //5XY0
        X = (opcode & 0x0F00) >> (2*4);
        Y = (opcode & 0x00F0) >> (1*4);
        pc += 2;
        if(V[X] == V[Y]) pc += 2;
    break;
    case 0x6000:
        //6XNN
        X = (opcode & 0x0F00) >> (2*4);
        V[X] = (opcode & 0x00FF);
        pc += 2;
    break;
    case 0x7000:
        //7XNN
        X = (opcode & 0x0F00) >> (2*4);
        V[X] += (opcode & 0x00FF);
        pc += 2;
    break;
    case 0x8000:
        switch(opcode & 0x000F) {
        case 0x0000:
            //8XY0
            X = (opcode & 0x0F00) >> (2*4);
            Y = (opcode & 0x00F0) >> (1*4);
            V[X] = V[Y];
            pc += 2;
        break;
        case 0x0001:
            //8XY1
            X = (opcode & 0x0F00) >> (2*4);
            Y = (opcode & 0x00F0) >> (1*4);
            V[X] = V[X] | V[Y];
            pc += 2;
        break;
        case 0x0002:
            //8XY2
            X = (opcode & 0x0F00) >> (2*4);
            Y = (opcode & 0x00F0) >> (1*4);
            V[X] = V[X] & V[Y];
            pc += 2;
        break;
        case 0x0003:
            //8XY3
            X = (opcode & 0x0F00) >> (2*4);
            Y = (opcode & 0x00F0) >> (1*4);
            V[X] = V[X] ^ V[Y];
            pc += 2;
        break;
        case 0x0004:
            //8XY4
            //additional note: just think of carry as exceeding 0x00FF
            //set 16th register if carry
            X = (opcode & 0x0F00) >> (2*4);
            Y = (opcode & 0x00F0) >> (1*4);
            if(V[X] + V[Y] > 0x00FF) V[15] = 1;
            else V[15] = 0;
            V[X] = V[X] + V[Y];
            pc += 2;
        break;
        case 0x0005:
            //8XY5
            //additional note: think of carry as negative result
            X = (opcode & 0x0F00) >> (2*4);
            Y = (opcode & 0x00F0) >> (1*4);
            if(V[X] < V[Y]) V[15] = 0;
            else V[15] = 1;
            //note that it is ok for this value to be negative
            //see "what happens if I assign a negative value to unsigned char"
            V[X] = V[X] - V[Y];
            pc += 2;
        break;
        case 0x0006:
            //8XY6
            X = (opcode & 0x0F00) >> (2*4);
            V[15] = V[X] & 0x0001;
            V[X] = V[X] >> 1;
            pc += 2;
        break;
        case 0x0007:
            //8XY7
            //THIS IS NOT THE SAME AS 8XY5
            X = (opcode & 0x0F00) >> (2*4);
            Y = (opcode & 0x00F0) >> (1*4);
            if(V[X] > V[Y]) V[15] = 0;
            else V[15] = 1;
            V[X] = V[Y] - V[X];
            pc += 2;
        break;
        case 0x000E: //check if this is correct
            //8XYE
            X = (opcode & 0x0F00) >> (2*4);
            V[15] = V[X] >> 7; //most significant bit (important to note that it's a bit)
            V[X] = V[X] << 1;
            pc += 2;
        break;
        }
    }
}
