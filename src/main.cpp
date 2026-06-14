#include "HardwareManager.h"
#include "StateManager.h"
#include "UIManager.h" 
#include "SplashLogo.h" 
#include "BootSequence.h" 
#include "StorageManager.h" 
#include "SynthEngine.h"

HardwareManager hw;
StateManager state;
UIManager ui; 
StorageManager storage; 
SynthEngine synth;

// --- GLOBAL MEMORY WALL BYPASS ---
volatile bool g_isBooting = true;
volatile float g_bootFreq = 110.0f;

// The pristine, high-priority audio thread
void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
    // 1. The Global Boot Sweep Bypass
    if (g_isBooting) {
        static float phase = 0.0f;
        for (size_t i = 0; i < size; i++) {
            phase += g_bootFreq / 48000.0f;
            if (phase >= 1.0f) phase -= 1.0f;
            
            float sig = (phase > 0.5f) ? 0.2f : -0.2f; // Raw 20% Volume Square Wave
            out[0][i] = sig;
            out[1][i] = sig;
        }
        return; 
    }

    // 2. Normal Synth Operations
    for (size_t i = 0; i < size; i++) {
        synth.Process(&out[0][i], &out[1][i]);
    }
}

int main(void) {
    hw.Init();
    
    synth.Init(hw.seed.AudioSampleRate());
    storage.Init(state);
    
    hw.seed.StartAudio(AudioCallback);

    // --- ANIMATION BOOT ---
    // Pass the global frequency by reference so the boot file can update it!
    PlayCinematicBoot(hw, g_bootFreq);
    
    // Hand over control to the Synth Engine
    g_isBooting = false; 
    state.needsDisplayUpdate = true;

    // --- MAIN PLAY LOOP ---
    while(1) {
        hw.ProcessInputs();
        hw.midi.Listen();
        
        int overflow_protector = 0;
        bool stateChanged = false; 
        
        while(hw.midi.HasEvents() && overflow_protector < 15) {
            overflow_protector++;
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

        state.ProcessState(hw);
        ui.Draw(hw, state); 
        hw.seed.DelayMs(1);
    }
}