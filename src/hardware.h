#pragma once
#ifndef MDS_HARDWARE_H
#define MDS_HARDWARE_H

#include "daisy_seed.h"

using namespace daisy;

// --- PIN MAPPING ---
#define PIN_ENC1_A 14
#define PIN_ENC1_B 15
#define PIN_ENC1_SW 16

#define PIN_ENC2_A 17
#define PIN_ENC2_B 18
#define PIN_ENC2_SW 19

#define PIN_ENC3_A 20
#define PIN_ENC3_B 21
#define PIN_ENC3_SW 22

#define PIN_BTN_GREEN 23
#define PIN_BTN_RED 24

#define PIN_LED_R 25
#define PIN_LED_G 26
#define PIN_LED_B 27

struct MDS_Hardware {
    DaisySeed seed;
    Encoder   enc1, enc2, enc3;
    Switch    btn_green, btn_red;
    Led       led_r, led_g, led_b;

    void Init() {
        seed.Init();

        // Encoders: Init(PinA, PinB, PinClick)
        enc1.Init(seed.GetPin(PIN_ENC1_A), seed.GetPin(PIN_ENC1_B), seed.GetPin(PIN_ENC1_SW));
        enc2.Init(seed.GetPin(PIN_ENC2_A), seed.GetPin(PIN_ENC2_B), seed.GetPin(PIN_ENC2_SW));
        enc3.Init(seed.GetPin(PIN_ENC3_A), seed.GetPin(PIN_ENC3_B), seed.GetPin(PIN_ENC3_SW));

        /** * Simplified Button Init
         * This uses the standard defaults: Momentary, Active Low, Pull-up enabled.
         * The only parameters are the Pin and the Callback Rate (1000Hz).
         */
        btn_green.Init(seed.GetPin(PIN_BTN_GREEN), 1000.0f);
        btn_red.Init(seed.GetPin(PIN_BTN_RED), 1000.0f);

        // LED
        led_r.Init(seed.GetPin(PIN_LED_R), false);
        led_g.Init(seed.GetPin(PIN_LED_G), false);
        led_b.Init(seed.GetPin(PIN_LED_B), false);
    }

    void ProcessInputs() {
        enc1.Debounce();
        enc2.Debounce();
        enc3.Debounce();
        btn_green.Debounce();
        btn_red.Debounce();
    }
};

#endif