#include "Machine.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <fstream>

//references
//1 - http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
//2 - http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
//3 - https://en.wikipedia.org/wiki/CHIP-8#Opcode_table

//other links that the comments refer to
//https://stackoverflow.com/questions/2711522/what-happens-if-i-assign-a-negative-value-to-an-unsigned-variable

/*
TODO:
-fix graphics op code overload
*/

Machine::Machine()
{
    //note that contents of init is not here because of future switch between game feature
    std::cout << "Created instance of Machine\n";
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
    std::cout << "init() SUCCESSFUL" << std::endl;
    return true;
}

//this function is buggy use the one taken from reference 1 until fixed
bool Machine::load(std::string path) {
    init();
    std::ifstream in;
    in.open(path);
    in.seekg(0, in.end);
    int length = in.tellg();
    /*
    MEMORY MAP (reference 1)
    0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    0x200-0xFFF - Program ROM and work RAM
    */
    if(length > 3584) {
        std::cout << "FILE TOO BIG, ABORTING" << std::endl;
        return false;
    }
    char* buffer = (char*)malloc(length * sizeof(char));
    in.seekg(0, in.beg);
    in.read(buffer, length);
    in.close();
    for(int i=0; i<length; i++) {
        memory[i + 512] = buffer[i];
    }
    std::cout << "load() SUCCESSFUL" << std::endl;
    return true;
}

//use this until top is fixed
bool Machine::loadApplication(const char * filename)
{
	init();
	printf("Loading: %s\n", filename);

	// Open file
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs ("File error", stderr);
		return false;
	}

	// Check file size
	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);

	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs ("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error",stderr);
		return false;
	}

	// Copy buffer to Chip8 memory
	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}

void Machine::cycle() {
    if(delay_timer) delay_timer--;
    if(sound_timer) sound_timer--;
    //NOTE IMPLEMENT SOUND TRIGGER ONCE WEBSOCKETS ARE SET UP

    //holder variables
    std::uint16_t reg;
    std::uint16_t val;
    std::uint16_t X;
    std::uint16_t Y;

    opcode = memory[pc] << 8 | memory[pc+1]; //16 bits
    //std::cout << opcode << std::endl;
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
        //std::cout << "BRUH " << pc << std::endl;
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
    case 0x9000:
        //9XY0
        X = (opcode & 0x0F00) >> (2*4);
        Y = (opcode & 0x00F0) >> (1*4);
        pc += 2;
        if(V[X] != V[Y]) pc += 2;
    break;
    case 0xA000:
        //ANNN
        I = opcode & 0x0FFF;
        pc += 2;
    break;
    case 0xB000:
        //BNNN
        pc = V[0] + (opcode & 0x0FFF);
    break;
    case 0xC000:
        //CXNN
        X = (opcode & 0x0F00) >> (2*4);
        V[X] = (std::rand() % 255) & (opcode & 0x00FF);
        pc += 2;
    break;
    case 0xD000:
    //DXYN
    //this snippit is taken straight from reference 1
    {

    unsigned short x = V[(opcode & 0x0F00) >> 8];
    unsigned short y = V[(opcode & 0x00F0) >> 4];
    unsigned short height = opcode & 0x000F;
    unsigned short pixel;

    V[0xF] = 0;
    for (int yline = 0; yline < height; yline++)
    {
    pixel = memory[I + yline];
    for(int xline = 0; xline < 8; xline++)
    {
      if((pixel & (0x80 >> xline)) != 0)
      {
        if(display[(x + xline + ((y + yline) * 64))] == 1)
          V[0xF] = 1;
        display[x + xline + ((y + yline) * 64)] ^= 1;
      }
    }
    }

    draw = true;
    pc += 2;

    }
    break;
    case 0xE000:
        switch(opcode & 0x000F) {
        case 0x000E:
            //EX9E
            X = (opcode & 0x0F00) >> (2*4);
            pc += 2;
            if(key[V[X]]) pc += 2;
        break;
        case 0x0001:
            //EXA1
            X = (opcode & 0x0F00) >> (2*4);
            pc += 2;
            if(!key[V[X]]) pc += 2;
        break;
        }
    break;
    case 0xF000:
        switch(opcode & 0x00FF) { //compare two values since there are a lot of F
        case 0x0007:
            //FX07
            X = (opcode & 0x0F00) >> (2*4);
            V[X] = delay_timer;
            pc += 2;
        break;
        case 0x000A: { //handing key presses wrap in block for variable declarations
            //FX0A
            bool pressed = false;
            X = (opcode & 0x0F00) >> (2*4);
            for(int i=0; i<16; i++) {
                if(key[i]) {
                    V[X] = i;
                    pressed = true;
                }
            }
            if(!pressed) return; //await
            pc += 2;
        }
        break;
        case 0x0015:
            //FX15
            X = (opcode & 0x0F00) >> (2*4);
            delay_timer = V[X];
            pc += 2;
        break;
        case 0x0018:
            //FX18
            X = (opcode & 0x0F00) >> (2*4);
            sound_timer = V[X];
            pc += 2;
        break;
        case 0x001E:
            //FX1E
            X = (opcode & 0x0F00) >> (2*4);
            if(I + V[X] > 0x0FFF) V[15] = 1;
            else V[15] = 0;
            I = I + V[X];
            pc += 2;
        break;
        case 0x0029:
            //FX29
            X = (opcode & 0x0F00) >> (2*4);
            I = V[X] * 5; //reference 1
            pc += 2;
        break;
        case 0x0033:
            //FX33
            X = (opcode & 0x0F00) >> (2*4);
            memory[I] = V[X] / 100;
            memory[I + 1] = (V[X] / 10) % 10;
            memory[I + 2] = (V[X] % 100) % 10;
            pc += 2;
        break;
        case 0x0055:
            X = (opcode & 0x0F00) >> (2*4);
            for (int i=0; i<=X; i++) {
                memory[I + i] = V[i];
            }
            I = I + X + 1;
            pc += 2;
        break;
        case 0x0065:
            //FX65
            X = (opcode & 0x0F00) >> (2*4);
            for(int i=0; i<=X; i++) {
                V[i] = memory[I + i];
            }
            I = I + X + 1;
            pc += 2;
        break;
        }
    }
}
