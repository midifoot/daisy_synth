#pragma once
#include "daisy_seed.h"
#include "daisysp.h"

class SynthEngine {
public:
    daisysp::Oscillator osc;
    
    // Variables for our diagnostic sweep
    bool isSoundcheck = false;
    float currentFreq = 110.0f;
    float volume = 0.05f; // Very low volume to protect your ears/speakers!

    void Init(float samplerate) {
        osc.Init(samplerate);
        osc.SetWaveform(daisysp::Oscillator::WAVE_SIN);
        osc.SetAmp(volume);
    }

    void Process(float* out_left, float* out_right) {
        if (isSoundcheck) {
            osc.SetFreq(currentFreq);
            float sig = osc.Process();
            *out_left = sig;
            *out_right = sig;
        } else {
            *out_left = 0.0f;
            *out_right = 0.0f;
        }
    }
};