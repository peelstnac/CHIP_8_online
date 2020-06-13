#include "Machine.h"
#include <iostream>
#include <fstream>

Machine::Machine()
{
    //ctor
}

Machine::~Machine()
{
    //dtor
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
    return true;

}
