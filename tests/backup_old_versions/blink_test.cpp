#include "daisy_seed.h"
#include "../src/hardware.h"

using namespace daisy;

MDS_Hardware hw;

int main(void) {
    // 1. Initialize hardware
    hw.Init();
    
    // 2. Immediate feedback: Turn LED ON
    hw.seed.SetLed(true);

    while(1) {
        // 3. Toggle the LED every 500ms
        hw.seed.SetLed(true);
        System::Delay(500);
        
        hw.seed.SetLed(false);
        System::Delay(500);
    }
}