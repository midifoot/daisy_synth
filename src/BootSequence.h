#pragma once
#include "HardwareManager.h"
#include "StateManager.h"
#include "SplashLogo.h" 

inline void PlayCinematicBoot(HardwareManager& hw, StateManager& state) {
    const uint32_t ANIM_TOTAL_TIME = 4000; 
    
    uint32_t startTime = hw.seed.system.GetNow();
    uint32_t elapsed = 0;

    while (elapsed < ANIM_TOTAL_TIME) {
        elapsed = hw.seed.system.GetNow() - startTime;
        hw.display.Fill(false);

        // --- THE FLIPBOOK ENGINE ---
        // Change frame every 80ms for a smooth spin
        int currentFrame = (elapsed / 80) % 3;
        const uint8_t* activeLogo = daisyLogos[currentFrame];

        // 1. Draw Spinning Logo (68x68 pixels, 9 bytes per row)
        for (int y = 0; y < 68; y++) {
            for (int x = 0; x < 68; x++) { 
                int byteIndex = (y * 9) + (x / 8); 
                int bitIndex = 7 - (x % 8);
                
                bool isPixelSet = (activeLogo[byteIndex] & (1 << bitIndex)) != 0; 
                
                if (isPixelSet) {
                    // Center the 68x68 image on the 128x64 screen
                    int screenX = 30 + x; 
                    int screenY = y - 2;  
                    
                    // Safety check to prevent memory crashes
                    if (screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 64) {
                         hw.display.DrawPixel(screenX, screenY, true); 
                    }
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