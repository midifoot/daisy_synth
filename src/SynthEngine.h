#pragma once
#include "daisy_seed.h"
#include "daisysp.h"

#define MAX_VOICES 6

class SynthEngine {
public:
    daisysp::Oscillator voices[MAX_VOICES];
    bool voiceActive[MAX_VOICES];
    uint8_t voiceMidiNote[MAX_VOICES];
    
    // Boot Sweep Variables
    bool isSoundcheck = false;
    float currentFreq = 110.0f;
    float volume = 0.2f; 

    void Init(float samplerate) {
        for(int i = 0; i < MAX_VOICES; i++) {
            voices[i].Init(samplerate);
            voices[i].SetWaveform(daisysp::Oscillator::WAVE_SIN);
            voices[i].SetAmp(0.0f);
            voiceActive[i] = false;
            voiceMidiNote[i] = 0;
        }
    }

    void PlayNote(uint8_t midiNote, uint8_t velocity) {
        // Find the first available voice
        for(int i = 0; i < MAX_VOICES; i++) {
            if(!voiceActive[i]) {
                float freq = daisysp::mtof(midiNote);
                voices[i].SetFreq(freq);
                
                // Safe Volume Scaling: Divides max volume by 6 so chords never clip!
                float safeAmp = (velocity / 127.0f) * (1.0f / MAX_VOICES);
                voices[i].SetAmp(safeAmp); 
                
                voiceMidiNote[i] = midiNote;
                voiceActive[i] = true;
                break; // Stop searching once we assign the note
            }
        }
    }

    void StopNote(uint8_t midiNote) {
        // Find the specific voice playing this note and stop it
        for(int i = 0; i < MAX_VOICES; i++) {
            if(voiceActive[i] && voiceMidiNote[i] == midiNote) {
                voices[i].SetAmp(0.0f);
                voiceActive[i] = false;
                voiceMidiNote[i] = 0;
            }
        }
    }

    void Process(float* out_left, float* out_right) {
        float mix = 0.0f;
        
        if (isSoundcheck) {
            voices[0].SetFreq(currentFreq);
            voices[0].SetAmp(volume);
            mix = voices[0].Process();
        } else {
            // Mix all active voices together!
            for(int i = 0; i < MAX_VOICES; i++) {
                if(voiceActive[i]) {
                    mix += voices[i].Process();
                }
            }
        }
        
        *out_left = mix;
        *out_right = mix;
    }
};