#include "daisy_seed.h"
#include "../src/hardware.h" // Note the ../ to go up to the src folder

using namespace daisy;
MDS_Hardware hw;

int main(void) {
    hw.Init();
    // Your specific test code here (e.g., just testing the screen)
    while(1) {
        // ...
    }
}