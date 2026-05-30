#pragma once
#include "HardwareManager.h"

// Define our global application states
enum class AppMode {
    PLAY,
    PATCH_EDIT,
    PHRASE_EDIT,
    SYSTEM_EDIT
};

class StateManager {
public:
    AppMode currentMode = AppMode::PLAY;
    
    // Set to true so it immediately draws the Play screen after the splash animation finishes
    bool needsDisplayUpdate = true; 

   void ProcessState(HardwareManager& hw) {
        // libDaisy's Encoder class uses .Pressed() 
        // It returns true when physically held down
        bool p1 = hw.enc1.Pressed(); 
        bool p2 = hw.enc2.Pressed();
        bool p3 = hw.enc3.Pressed();
        
        // Raw GPIO uses .Read() 
        // It returns false when pressed due to PULLUP, so we invert it with !
        bool redBtn = !hw.btnRed.Read();

        AppMode newMode = currentMode;

        // Check for Multi-Button Combos
        if (p1 && p2 && !p3) {
            newMode = AppMode::PATCH_EDIT;
        } 
        else if (!p1 && p2 && p3) {
            newMode = AppMode::PHRASE_EDIT;
        } 
        else if (p1 && !p2 && p3) {
            newMode = AppMode::SYSTEM_EDIT;
        }
        
        // Red button acts as global "Exit / Back to Play"
        if (redBtn) {
            newMode = AppMode::PLAY;
        }

        // If the mode changed, flag the display for an update
        if (newMode != currentMode) {
            currentMode = newMode;
            needsDisplayUpdate = true;
            
            // 300ms debounce for the combo
            hw.seed.DelayMs(300); 
        }
    }

    void DrawUI(HardwareManager& hw) {
        if (!needsDisplayUpdate) return;

        hw.display.Fill(false);
        hw.display.SetCursor(0, 0);

        // Draw different screens based on the current mode
        switch (currentMode) {
            case AppMode::PLAY:
                hw.display.WriteString("== PLAY MODE ==", Font_7x10, true);
                hw.display.SetCursor(0, 20);
                hw.display.WriteString("Hold P1+P2: Patch", Font_5x8, true);
                hw.display.SetCursor(0, 30);
                hw.display.WriteString("Hold P2+P3: Phrase", Font_5x8, true);
                hw.display.SetCursor(0, 40);
                hw.display.WriteString("Hold P1+P3: System", Font_5x8, true);
                break;

            case AppMode::PATCH_EDIT:
                hw.display.WriteString("== PATCH EDIT ==", Font_7x10, true);
                hw.display.SetCursor(0, 30);
                hw.display.WriteString("[Red Btn] to Exit", Font_5x8, true);
                break;

            case AppMode::PHRASE_EDIT:
                hw.display.WriteString("== PHRASE EDIT ==", Font_7x10, true);
                hw.display.SetCursor(0, 30);
                hw.display.WriteString("[Red Btn] to Exit", Font_5x8, true);
                break;

            case AppMode::SYSTEM_EDIT:
                hw.display.WriteString("== SYSTEM EDIT ==", Font_7x10, true);
                hw.display.SetCursor(0, 30);
                hw.display.WriteString("[Red Btn] to Exit", Font_5x8, true);
                break;
        }

        hw.display.Update();
        needsDisplayUpdate = false;
    }
};