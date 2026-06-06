#include "HardwareManager.h"
#include "StateManager.h"
#include "UIManager.h" 
#include "BootSequence.h" 
#include "StorageManager.h" 
#include "SynthEngine.h"

HardwareManager hw;
StateManager state;
UIManager ui; 
StorageManager storage; 
SynthEngine synth;

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

        hw.midi.Listen();
        int overflow_protector = 0;
        bool stateChanged = false; // To trigger UI updates efficiently
        
        while(hw.midi.HasEvents() && overflow_protector < 15) {
            overflow_protector++;
            daisy::MidiEvent msg = hw.midi.PopEvent();
            
            if(msg.type == daisy::NoteOn) {
                daisy::NoteOnEvent p = msg.AsNoteOn();
                if(p.velocity > 0) {
                    synth.PlayNote(p.note, p.velocity);
                } else {
                    synth.StopNote(p.note); // Pass the specific note to stop!
                }
                stateChanged = true;
            } 
            else if(msg.type == daisy::NoteOff) {
                daisy::NoteOffEvent p = msg.AsNoteOff();
                synth.StopNote(p.note); // Pass the specific note to stop!
                stateChanged = true;
            }
        }

        // --- NEW: UI FEEDBACK LOGIC ---
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
            if(!state.midiActive) {
                strcpy(state.sysMsg, "STATUS: WAITING...");
            }
            state.needsDisplayUpdate = true;
        }

        state.ProcessState(hw);
        ui.Draw(hw, state); 
        hw.seed.DelayMs(1);
    }
}