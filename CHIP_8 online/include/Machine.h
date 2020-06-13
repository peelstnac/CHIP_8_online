#ifndef MACHINE_H
#define MACHINE_H
#include <string>

class Machine
{
    public:
        Machine();
        virtual ~Machine();

        bool load(std::string path);
    private:
        std::uint8_t memory[4096];
        std::uint8_t V[16]; //registers
        std::uint16_t _stack[16];
        std::uint16_t sp; //stack pointer
        std::uint8_t delay_timer;
        std::uint8_t sound_timer;
        std::uint16_t opcode;
        std::uint16_t pc;
        std::uint16_t I;
        std::uint8_t key[16];
        std::uint8_t display[2048];
};

#endif // MACHINE_H
