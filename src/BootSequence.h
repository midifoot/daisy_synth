#pragma once
#include "HardwareManager.h"
#include "StateManager.h"
#include "SplashLogo.h" 

// The 'inline' keyword allows us to define this function in a header file safely
inline void PlayCinematicBoot(HardwareManager& hw, StateManager& state) {
    const uint32_t ANIM_TOTAL_TIME = 4000; 
    
    uint32_t startTime = hw.seed.system.GetNow();
    uint32_t elapsed = 0;

    while (elapsed < ANIM_TOTAL_TIME) {
        elapsed = hw.seed.system.GetNow() - startTime;
        hw.display.Fill(false);

        // 1. Draw Logo
        for (int y = 0; y < 68; y++) {
            for (int x = 0; x < 124; x++) {
                int byteIndex = (y * 16) + (x / 8);
                int bitIndex = 7 - (x % 8);
                
                bool isPixelSet = (daisyLogo[byteIndex] & (1 << bitIndex)) == 0; 
                
                if (isPixelSet && (y - 2 >= 0) && (y - 2 < 64)) {
                     hw.display.DrawPixel(2 + x, y - 2, true);
                }
            }
        }

        // 2. Text 1 Slide 
        if (elapsed > 1000) {
            int slideProgress = (elapsed - 1000) / 40; 
            int text1_x = 0 + slideProgress; 
            if (text1_x > 25) text1_x = 25;  

            hw.display.DrawRect(text1_x, 25, text1_x + 75, 36, false, true); 
            hw.display.SetCursor(text1_x + 2, 26);
            hw.display.WriteString("Daisy Synth", Font_7x10, true);
        }

        // 3. Text 2 Slide 
        if (elapsed > 2000) {
            int slideProgress = (elapsed - 2000) / 9; 
            int text2_x = 127 - slideProgress; 
            if (text2_x < 18) text2_x = 18; 

            hw.display.DrawRect(text2_x, 40, text2_x + 85, 50, false, true);
            hw.display.SetCursor(text2_x + 2, 41);
            hw.display.WriteString("Step into music !", Font_5x8, true);
        }

        hw.display.Update();
        hw.seed.DelayMs(5); 
    }

    // Blackout transition
    hw.display.Fill(false);
    hw.display.Update();
    hw.seed.DelayMs(200); 

    // Trigger the Play Mode UI
    state.needsDisplayUpdate = true;
}