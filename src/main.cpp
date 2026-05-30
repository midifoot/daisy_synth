#include "HardwareManager.h"

// Instantiate our HAL globally
HardwareManager hw;

int main(void) {
    // Set up all pins and hardware
    hw.Init();

    // Boot screen
    hw.display.Fill(false);
    hw.display.SetCursor(20, 25);
    hw.display.WriteString("MDS SYSTEM BOOT", Font_7x10, true);
    hw.display.Update();

    hw.seed.DelayMs(1500); // Hold boot screen for 1.5 seconds

    while(1) {
        // Read physical hardware state
        hw.ProcessInputs();
        hw.seed.DelayMs(1);

        // ==========================================
        // TEMPORARY TEST: Ensure HAL is working
        // ==========================================
        if(hw.enc1.RisingEdge()) {
            hw.display.Fill(false);
            hw.display.SetCursor(10, 30);
            hw.display.WriteString("ENC 1 CLICKED!", Font_7x10, true);
            hw.display.Update();
        }
    }
}