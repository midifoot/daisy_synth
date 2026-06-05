#include "HardwareManager.h"
#include "StateManager.h"
#include "UIManager.h" 
#include "BootSequence.h" 
#include "StorageManager.h" 
#include "SynthEngine.h" // NEW

HardwareManager hw;
StateManager state;
UIManager ui; 
StorageManager storage; 
SynthEngine synth; // NEW

// The pristine, high-priority audio thread
void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
    for (size_t i = 0; i < size; i++) {
        synth.Process(&out[0][i], &out[1][i]);
    }
}

int main(void) {
    hw.Init();
    
    // 1. Init Audio and start the callback in the background
    synth.Init(hw.seed.AudioSampleRate());
    hw.seed.StartAudio(AudioCallback);

    // 2. Initialize the SD Card
    storage.Init(state);

    // 3. Play animation (which now triggers the audio sweep!)
    PlayCinematicBoot(hw, state, synth);

    while(1) {
        hw.ProcessInputs();
        state.ProcessState(hw);
        ui.Draw(hw, state); 
        hw.seed.DelayMs(1);
    }
}