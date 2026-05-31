#include "HardwareManager.h"
#include "StateManager.h"
#include "BootSequence.h" 

HardwareManager hw;
StateManager state;

int main(void) {
    hw.Init();
    PlayCinematicBoot(hw, state);

    while(1) {
        hw.ProcessInputs();
        state.ProcessState(hw);
        state.DrawUI(hw);
        hw.seed.DelayMs(1);
    }
}