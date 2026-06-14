#pragma once
#include "daisy_seed.h"
#include "daisysp.h"

class MetronomeEngine {
public:
    volatile bool running = false;
    volatile float bpm = 120.0f;
    volatile int volume = 50;
    volatile int num = 4;
    volatile int den = 4;
    volatile int subdiv = 1;
    volatile bool ternary = false;

    daisysp::Oscillator clickOsc;
    uint32_t sampleCounter = 0;
    int currentBeat = 0;
    int currentSub = 0;

    void Init(float samplerate) {
        clickOsc.Init(samplerate);
        clickOsc.SetWaveform(daisysp::Oscillator::WAVE_SIN);
        clickOsc.SetAmp(0.0f);
        clickOsc.SetFreq(880.0f);
    }

    void UpdateTiming() {}

    void Process(float* out_left, float* out_right, float samplerate) {
        if (!running) {
            clickOsc.SetAmp(0.0f);
            return;
        }

        uint32_t samplesPerBeat = (uint32_t)((60.0f / bpm) * samplerate);
        int effectiveDivs = (ternary && subdiv == 2) ? 3 : subdiv;
        uint32_t samplesPerTick = samplesPerBeat / effectiveDivs;
        uint32_t tickDuration = (uint32_t)(0.05f * samplerate); // 50ms tick

        sampleCounter++;
        
        // Turn tick off after 50ms
        if (sampleCounter == tickDuration) {
            clickOsc.SetAmp(0.0f);
        }

        if (sampleCounter >= samplesPerTick) {
            sampleCounter = 0;
            
            // Ternary logic: skip the middle tick (index 1) of a 3-count group
            bool shouldTrigger = !(ternary && subdiv == 2 && currentSub == 1);
            
            if (shouldTrigger) {
                // FIXED: It now correctly detects beat 0 and sub 0
                clickOsc.SetFreq((currentBeat == 0 && currentSub == 0) ? 1200.0f : 880.0f);
                // FIXED: Volume is now 2x louder 
                clickOsc.SetAmp(volume / 100.0f); 
            }

            // Move the increments AFTER the trigger check!
            currentSub++;
            if (currentSub >= effectiveDivs) {
                currentSub = 0;
                currentBeat++;
                if (currentBeat >= num) currentBeat = 0;
            }
        }
        
        float sig = clickOsc.Process();
        *out_left += sig;
        *out_right += sig;
    }
};