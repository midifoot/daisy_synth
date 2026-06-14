#include "HardwareManager.h"
#include "StateManager.h"
#include "UIManager.h" 
#include "SplashLogo.h" 
#include "BootSequence.h" 
#include "StorageManager.h" 
#include "SynthEngine.h"
#include "MetronomeEngine.h"

HardwareManager hw;
StateManager state;
UIManager ui; 
StorageManager storage; 
SynthEngine synth;

volatile bool g_isBooting = true;
volatile float g_bootFreq = 110.0f;
volatile float g_bootVolume = 0.2f;
volatile float g_masterVolume = 0.5f;

void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
    if (g_isBooting) {
        static float phase = 0.0f;
        for (size_t i = 0; i < size; i++) {
            phase += g_bootFreq / 48000.0f;
            if (phase >= 1.0f) phase -= 1.0f;
            float sig = (phase > 0.5f) ? g_bootVolume : -g_bootVolume; 
            out[0][i] = sig; 
            out[1][i] = sig; 
        }
        return; 
    }

    for (size_t i = 0; i < size; i++) {
        synth.Process(&out[0][i], &out[1][i]);
        state.metro.Process(&out[0][i], &out[1][i], hw.seed.AudioSampleRate());
        out[0][i] *= g_masterVolume; 
        out[1][i] *= g_masterVolume;
    }
}

int main(void) {
    hw.Init();
    synth.Init(hw.seed.AudioSampleRate());
    state.metro.Init(hw.seed.AudioSampleRate());
    storage.Init(state);
    
    hw.seed.StartAudio(AudioCallback);
    state.UpdateLED(hw);
    
    g_bootVolume = state.menu.n_sysBootVol.value / 100.0f; 
    
    // CORRECTED: Call match signature
    PlayCinematicBoot(hw, g_bootFreq); 
    
    g_isBooting = false; 
    state.needsDisplayUpdate = true;

    while(1) {
        hw.ProcessInputs();
        hw.midi.Listen();
        
        bool stateChanged = false; 
        while(hw.midi.HasEvents()) {
            daisy::MidiEvent msg = hw.midi.PopEvent();
            if(msg.type == daisy::NoteOn) {
                daisy::NoteOnEvent p = msg.AsNoteOn();
                if(p.velocity > 0) synth.PlayNote(p.note, p.velocity);
                else synth.StopNote(p.note);
                stateChanged = true;
            } 
            else if(msg.type == daisy::NoteOff) {
                daisy::NoteOffEvent p = msg.AsNoteOff();
                synth.StopNote(p.note); 
                stateChanged = true;
            }
        }
        
        if (stateChanged) {
            state.midiActive = false;
            strcpy(state.sysMsg, "NOTES: "); 
            char noteBuf[8];
            for(int i = 0; i < 6; i++) {
                if(synth.voiceActive[i]) {
                    state.midiActive = true;
                    sprintf(noteBuf, "%d ", synth.voiceMidiNote[i]);
                    strcat(state.sysMsg, noteBuf);
                }
            }
            if(!state.midiActive) strcpy(state.sysMsg, "STATUS: WAITING...");
            state.needsDisplayUpdate = true;
        }

        if (state.isPanic) g_masterVolume = 0.0f; 
        else g_masterVolume = state.mainVolume / 100.0f; 

        state.ProcessState(hw);
        ui.Draw(hw, state); 
        hw.seed.DelayMs(1);
    }
}