#pragma once
#include "daisy_seed.h"
#include "daisysp.h"

#define MAX_VOICES 6

class SynthEngine {
public:
    daisysp::Oscillator voices[MAX_VOICES];
    volatile bool voiceActive[MAX_VOICES];
    volatile uint8_t voiceMidiNote[MAX_VOICES];

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
        float safeAmp = (velocity / 127.0f) * (1.0f / MAX_VOICES);

        // 1. Check if this exact note is ALREADY playing. (Re-trigger it!)
        for(int i = 0; i < MAX_VOICES; i++) {
            if(voiceActive[i] && voiceMidiNote[i] == midiNote) {
                voices[i].SetAmp(safeAmp); 
                return; // We re-triggered it, so we are done.
            }
        }

        // 2. If it's a new note, find the first empty voice slot.
        for(int i = 0; i < MAX_VOICES; i++) {
            if(!voiceActive[i]) {
                float freq = daisysp::mtof(midiNote);
                voices[i].SetFreq(freq);
                voices[i].SetAmp(safeAmp); 
                voiceMidiNote[i] = midiNote;
                voiceActive[i] = true;
                return; // Note placed successfully.
            }
        }
    }

    void StopNote(uint8_t midiNote) {
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
        for(int i = 0; i < MAX_VOICES; i++) {
            if(voiceActive[i]) {
                mix += voices[i].Process();
            }
        }
        *out_left = mix;
        *out_right = mix;
    }
};