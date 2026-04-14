#include "daisy_seed.h"
#include "hardware.h"

using namespace daisy;

MDS_Hardware hw;

int main(void) {
    hw.Init();
    
    // Start with a Blue LED to show system is alive
    hw.led_b.Set(1.0f);
    hw.led_b.Update();

    while(1) {
        hw.ProcessInputs();

        // Simple feedback: Green button turns LED Green
        if(hw.btn_green.Pressed()) {
            hw.led_g.Set(1.0f);
        } else {
            hw.led_g.Set(0.0f);
        }

        hw.led_g.Update();
        System::Delay(10);
    }
}