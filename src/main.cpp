#include "HardwareManager.h"
#include "StateManager.h"
#include "UIManager.h" // NEW
#include "BootSequence.h" 

HardwareManager hw;
StateManager state;
UIManager ui; // NEW

int main(void) {
    hw.Init();
    PlayCinematicBoot(hw, state);

    while(1) {
        hw.ProcessInputs();
        state.ProcessState(hw);
        ui.Draw(hw, state); // Tying the Model to the View
        hw.seed.DelayMs(1);
    }
}