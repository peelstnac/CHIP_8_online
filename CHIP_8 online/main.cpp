#include <iostream>
#include "Machine.h"

Machine machine;

int main()
{
    machine.loadApplication("pong2.c8");
    for(int i=0; i<30005; i++) {
        using namespace std;
        machine.cycle();
        if(i<10000) continue;
        //testing display
        for(int j=0; j<32; j++) {
            for(int k=0; k<64; k++) cout << (int)machine.display[k+j*64];
            cout << endl;
        }
    }
}
