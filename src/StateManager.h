#pragma once
#include "HardwareManager.h"
#include "MenuManager.h" // <-- NEW

enum class AppMode {
    PLAY,
    PATCH_EDIT,
    PHRASE_EDIT,
    SYSTEM_EDIT
};

class StateManager {
public:
    AppMode currentMode = AppMode::PLAY;
    bool needsDisplayUpdate = true;
    
    MenuManager menu; // <-- NEW: The state machine owns the menu now

    StateManager() {
        menu.Init(); // Initialize the dummy tree
    }

    void ProcessState(HardwareManager& hw) {
        // If we are in the menu, let the MenuManager handle inputs!
        if (currentMode == AppMode::PATCH_EDIT) {
            menu.ProcessInput(hw);
            
            // If the user hit the Red Button in the menu, it might have backed all the way out.
            // Let's check if the red button was pressed while at the root menu to exit back to PLAY mode
            if (!hw.btnRed.Read() && !menu.isEditing && menu.currentSelection->parent == nullptr) {
                currentMode = AppMode::PLAY;
                needsDisplayUpdate = true;
                hw.seed.DelayMs(300); // Debounce
            }
            return; // Don't process the standard Play Mode inputs
        }

        // --- PLAY MODE INPUTS ---
        bool p1 = hw.enc1.RisingEdge(); 
        // ... (Your other play mode inputs here)

        if (p1) {
            currentMode = AppMode::PATCH_EDIT;
            menu.needsDisplayUpdate = true; // Force menu to draw
            needsDisplayUpdate = true;
        }
    }

    void DrawUI(HardwareManager& hw) {
        if (currentMode == AppMode::PATCH_EDIT) {
            menu.DrawUI(hw); // Let the menu draw itself!
            return;
        }

        // --- PLAY MODE DRAWING ---
        if (needsDisplayUpdate) {
            hw.display.Fill(false);
            hw.display.SetCursor(30, 25);
            hw.display.WriteString("PLAY MODE", Font_7x10, true);
            hw.display.SetCursor(10, 40);
            hw.display.WriteString("Press Enc 1 for Menu", Font_5x8, true);
            hw.display.Update();
            needsDisplayUpdate = false;
        }
    }
};