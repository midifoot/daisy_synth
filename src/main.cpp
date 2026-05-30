#include "HardwareManager.h"
#include "StateManager.h"

// Instantiate our Managers globally
HardwareManager hw;
StateManager state;

int main(void) {
    // 1. Initialize all hardware pins and peripherals
    hw.Init();

    // 2. Simple static boot screen (No 6-second animation here!)
    hw.display.Fill(false);
    hw.display.SetCursor(12, 25);
    hw.display.WriteString("MDS SYSTEM READY", Font_7x10, true);
    hw.display.Update();
    
    // Hold the boot screen for just 1 second so you know it powered on
    hw.seed.DelayMs(1000); 

    // 3. The Infinite Architecture Loop
    while(1) {
        // Read the physical encoders and buttons
        hw.ProcessInputs();
        
        // Let the StateMachine decide what mode we are in based on inputs
        state.ProcessState(hw);

        // Let the StateMachine draw the correct screen if the mode changed
        state.DrawUI(hw);

        // Keep the loop running smoothly (1ms tick)
        hw.seed.DelayMs(1);
    }
}