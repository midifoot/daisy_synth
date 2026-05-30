#include "HardwareManager.h"
#include "StateManager.h"
#include "BootSequence.h" 

HardwareManager hw;
StateManager state;

int main(void) {
    // 1. Initialize Hardware
    hw.Init();

    // 2. Play Boot Animation
    PlayCinematicBoot(hw, state);

    // 3. Enter Main App Loop
    while(1) {
        hw.ProcessInputs();
        state.ProcessState(hw);
        state.DrawUI(hw);
        hw.seed.DelayMs(1);
    }
}