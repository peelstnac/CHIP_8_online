#include <iostream>
#include "Machine.h"

using namespace std;
Machine machine;

int main()
{
    machine.loadApplication("pong2.c8");
    for(int i=0; i<30000; i++) {
        machine.cycle();
            for(int j=0; j<32; j++) {
            for(int k=0; k<64; k++) cout << machine.display[k+j*64];
            cout << endl;
    }
    }
}
