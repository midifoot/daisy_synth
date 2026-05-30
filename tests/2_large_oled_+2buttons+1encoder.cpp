#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"
#include "hid/disp/oled_display.h" 
#include <stdio.h> 

using namespace daisy;

DaisySeed hw; 
using MyOledDriver = SSD130xI2c128x64Driver;
OledDisplay<MyOledDriver> display;

int main(void) {
    hw.Init();

    // ==========================================
    // 1. OLED INIT
    // ==========================================
    OledDisplay<MyOledDriver>::Config display_cfg;
    display_cfg.driver_config.transport_config.i2c_address = 0x3C;
    auto &i2c_cfg = display_cfg.driver_config.transport_config.i2c_config;
    i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_cfg.pin_config.scl = seed::D11; 
    i2c_cfg.pin_config.sda = seed::D12; 
    display.Init(display_cfg); 

    // ==========================================
    // 2. BUTTONS INIT
    // ==========================================
    daisy::GPIO G_button;
    daisy::GPIO R_button;
    G_button.Init(daisy::seed::D0, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::PULLUP);
    R_button.Init(daisy::seed::D1, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::PULLUP);

    // ==========================================
    // 3. SINGLE ENCODER INIT (Inverted Fix)
    // ==========================================
    daisy::Encoder enc1;
    enc1.Init(seed::D3, seed::D2, seed::D4); 

    // UI Variables
    int mainVolume = 100;
    char volStr[5]; 
    bool needsDisplayUpdate = true; 
    bool lastGB = false;
    bool lastRB = false;

    // Velocity / Acceleration Variables
    uint32_t lastTurnTime = 0; 
    const uint32_t fastTurnThreshold = 250; // The benchmarked threshold
    const int fastMultiplier = 10;           // +5 or -5 when turning fast

    while(1) {
        enc1.Debounce();
        System::Delay(1); 

        // --- CHECK BUTTONS ---
        bool GBPressed = !G_button.Read();
        bool RBPressed = !R_button.Read();

        if (GBPressed != lastGB || RBPressed != lastRB) {
            needsDisplayUpdate = true;
            lastGB = GBPressed;
            lastRB = RBPressed;
        }

        // --- CHECK ENCODER ROTATION WITH VELOCITY ---
        int inc = enc1.Increment();
        if (inc != 0) {
            uint32_t currentTime = System::GetNow();
            uint32_t timeDelta = currentTime - lastTurnTime;
            lastTurnTime = currentTime;

            // Safety catch: prevent the very first turn from jumping wildly
            if (timeDelta > 2000) timeDelta = 2000; 

            int step = inc; 
            
            // Check against our benchmarked threshold
            if (timeDelta < fastTurnThreshold) {
                step = inc * fastMultiplier; 
            }

            mainVolume += step;
            
            if (mainVolume > 100) mainVolume = 100;
            if (mainVolume < 0) mainVolume = 0;
            
            needsDisplayUpdate = true; 
        }

        // --- CHECK ENCODER BUTTON ---
        if (enc1.RisingEdge()) { 
            mainVolume = 50; 
            needsDisplayUpdate = true;
        }

        // ==========================================
        // ONLY DRAW IF SOMETHING CHANGED
        // ==========================================
        if (needsDisplayUpdate) {
            display.Fill(false); 

            // 1. MIDI Indicator
            if (GBPressed || RBPressed) {
                display.DrawCircle(4, 4, 3, true); 
                display.DrawCircle(4, 4, 2, true);
            }

            // 2. Dynamic Volume Box
            sprintf(volStr, "%d", mainVolume); 
            display.DrawRect(11, 0, 30, 9, true, true); 
            display.SetCursor(13, 0);
            display.WriteString(volStr, Font_5x8, false);

            display.SetCursor(32, 0); 
            display.WriteString("/ 100", Font_5x8, true);

            // 3. Mini Audio VU Meter
            display.DrawRect(50, 1, 82, 7, true, false); 
            display.DrawRect(50, 1, 62, 7, true, true);  

            // 4. CPU Load
            display.SetCursor(86, 0);
            display.WriteString("100%", Font_5x8, true);

            // 5. Mode
            display.SetCursor(107, 0);
            display.WriteString("PLAY", Font_5x8, true);
            
            // Line 2: System Messages
            display.SetCursor(0, 8);
            display.WriteString("STATUS: SYSTEM READY", Font_5x8, true);
            display.DrawLine(0, 17, 127, 17, true);

            // ZONE 2: SYNTHESIS
            display.SetCursor(0, 19);
            display.WriteString("024 ARCADE B", Font_7x10, true);
            display.SetCursor(0, 28);
            display.WriteString("WAVE | LADDER | REVERB", Font_4x6, true);
            display.DrawLine(0, 35, 127, 35, true);

            // ZONE 3: PHRASER
            display.SetCursor(0, 36);
            display.WriteString("012 acid_line_01", Font_5x8, true);
            display.SetCursor(0, 44);
            display.WriteString("BPM:120  TS:4/4  L:16", Font_4x6, true);
            display.DrawLine(0, 51, 127, 51, true);

            // ZONE 4: COMPRESSED HELP MENU
            display.SetCursor(0, 52); 
            display.WriteString("R1:SYNTH R2:PHRASE R3:VOL/MIX", Font_4x6, true);
            display.SetCursor(0, 58); 
            display.WriteString("RB:SYNTH EDIT GB:PHR EDIT", Font_4x6, true);

            display.Update();
            needsDisplayUpdate = false; 
        }
    }
}