#include "HardwareManager.h"
#include "StateManager.h"
#include "UIManager.h" 
#include "BootSequence.h" 
#include "daisysp.h" 
#include <string.h> 

using namespace daisy; 

HardwareManager hw;
StateManager state;
UIManager ui;

// --- POLYPHONIC AUDIO VOICE ARCHITECTURE ---
struct SynthVoice {
    daisysp::Oscillator osc;
    int note = -1;          // -1 means this voice slot is empty and available
    float amplitude = 0.0f;  // Target volume for this voice
};

const int MAX_VOICES = 3;   // Perfect fit for our 3-note testing chord
SynthVoice voices[MAX_VOICES];

// --- THE POLYPHONIC AUDIO CALLBACK ---
void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
    // 1. Safe Panic Shutter
    if (state.isPanic) {
        for (size_t i = 0; i < size; i++) {
            out[0][i] = 0.0f;
            out[1][i] = 0.0f;
        }
        return;
    }

    // 2. Headroom-tamed master volume math
    float masterVolume = (state.mainVolume / 100.0f) * 0.15f; 

    // 3. Render every single sample in the block buffer
    for (size_t i = 0; i < size; i++) {
        float mixedSignal = 0.0f;

        // Sum up the active waveforms from all 3 voice tracking slots
        for (int v = 0; v < MAX_VOICES; v++) {
            if (voices[v].note != -1) {
                mixedSignal += voices[v].osc.Process() * voices[v].amplitude;
            }
        }

        // Divide by MAX_VOICES to prevent digital clipping/distortion when mixed together
        float finalOutput = (mixedSignal / (float)MAX_VOICES) * masterVolume;

        out[0][i] = finalOutput; // Left Channel
        out[1][i] = finalOutput; // Right Channel
    }
}

int main(void) {
    hw.Init();
    
    float sample_rate = hw.seed.AudioSampleRate();
    
    // --- INITIALIZE ALL VOICE OSCILLATORS ---
    for (int v = 0; v < MAX_VOICES; v++) {
        voices[v].osc.Init(sample_rate);
        voices[v].osc.SetWaveform(daisysp::Oscillator::WAVE_SAW); // Rich, classic synth sound
        voices[v].note = -1;
        voices[v].amplitude = 0.0f;
    }

    PlayCinematicBoot(hw, state);
    hw.seed.StartAudio(AudioCallback);

    hw.midi.StartReceive();

    while(1) {
        hw.ProcessInputs();
        state.ProcessState(hw);

        hw.midi.Listen();

        if (hw.midi.HasEvents()) {
            char noteList[80] = ""; 
            bool isNoteOnBatch = false;
            bool isNoteOffBatch = false;
            int chordCount = 0;

            while (hw.midi.HasEvents()) {
                auto msg = hw.midi.PopEvent();
                
                if (chordCount >= 6 || strlen(noteList) > 65) {
                    continue; 
                }
                
                // ==========================================
                // ROUTE NOTE ON TO AN AVAILABLE AUDIO SLOT
                // ==========================================
                if (msg.type == NoteOn && msg.AsNoteOn().velocity > 0) {
                    isNoteOnBatch = true;
                    auto noteData = msg.AsNoteOn();

                    // Find an empty audio slot to hold this new note
                    int targetSlot = -1;
                    for (int v = 0; v < MAX_VOICES; v++) {
                        if (voices[v].note == -1) {
                            targetSlot = v;
                            break;
                        }
                    }

                    // Assign the pitch if an open slot was found
                    if (targetSlot != -1) {
                        voices[targetSlot].note = noteData.note;
                        // FIXED: Corrected namespace call to daisysp::mtof
                        voices[targetSlot].osc.SetFreq(daisysp::mtof(noteData.note));
                        voices[targetSlot].amplitude = (noteData.velocity / 127.0f);
                    }

                    // Append to screen display string
                    if (chordCount > 0) strcat(noteList, ",");
                    char temp[8];
                    sprintf(temp, "%d", noteData.note);
                    strcat(noteList, temp);
                    chordCount++;
                }
                
                // ==========================================
                // ROUTE NOTE OFF TO SILENCE THE AUDIO SLOT
                // ==========================================
                else if (msg.type == NoteOff || (msg.type == NoteOn && msg.AsNoteOn().velocity == 0)) {
                    isNoteOffBatch = true;
                    int targetNote = (msg.type == NoteOff) ? msg.AsNoteOff().note : msg.AsNoteOn().note;

                    // Locate the voice slot currently playing this specific note number and clear it
                    for (int v = 0; v < MAX_VOICES; v++) {
                        if (voices[v].note == targetNote) {
                            voices[v].note = -1; // Frees up the voice slot immediately
                            voices[v].amplitude = 0.0f;
                        }
                    }

                    // Append to screen display string
                    if (chordCount > 0) strcat(noteList, ",");
                    char temp[8];
                    sprintf(temp, "%d", targetNote);
                    strcat(noteList, temp);
                    chordCount++;
                }
            }

            if (chordCount > 0) {
                if (isNoteOnBatch) {
                    snprintf(state.sysMsg, sizeof(state.sysMsg), "CHORD ON: %s", noteList);
                } else if (isNoteOffBatch) {
                    snprintf(state.sysMsg, sizeof(state.sysMsg), "CHORD OFF: %s", noteList);
                }
                state.needsDisplayUpdate = true;
            }
        }

        ui.Draw(hw, state);
        hw.seed.DelayMs(1);
    }
}