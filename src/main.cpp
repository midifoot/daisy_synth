#include "HardwareManager.h"
#include "StateManager.h"
#include "UIManager.h" 
#include "BootSequence.h" 
#include "StorageManager.h" // NEW

HardwareManager hw;
StateManager state;
UIManager ui; 
StorageManager storage; // NEW

int main(void) {
    hw.Init();
    
    // Initialize the SD Card in the background
    storage.Init(state);

    // Play your cinematic boot animation
    PlayCinematicBoot(hw, state);

    // Your pristine, unmodified polling loop
    while(1) {
        hw.ProcessInputs();
        state.ProcessState(hw);
        ui.Draw(hw, state); 
        hw.seed.DelayMs(1);
    }
}