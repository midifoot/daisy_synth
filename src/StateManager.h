#pragma once
#include "HardwareManager.h"
#include "MenuManager.h" 

enum class AppMode { PLAY, PATCH_EDIT, PHRASE_EDIT, SYSTEM_EDIT };

class StateManager {
public:
    AppMode currentMode = AppMode::PLAY;
    bool needsDisplayUpdate = true;
    bool isBootInitialized = false; 
    
    MenuManager menu; 

    // --- PLAY MODE VARIABLES ---
    bool lastGB = false;
    bool lastRB = false;
    char sysMsg[40] = "STATUS: SYSTEM READY";
    
    int synthPreview = 0;
    int synthLoaded = 0;
    int phrasePreview = 0;
    int phraseLoaded = 0;
    
    bool isVolSelected = true; 
    int mainVolume = 100;
    int fxMix = 50;

    // --- DUMMY DATA FOR PLAY MODE ---
    const char* synthNames[10] = {"ARCADE B", "DEEP BASS", "SAW LEAD", "FM BELLS", "PULSE PAD", "NOISE HIT", "SOFT KEY", "HARD SYNC", "VOWEL SEQ", "ACID DROP"};
    const char* synthParams[10] = {"WAVE | LDR | REV", "SUB | LPF | CHO", "SAW | HPF | DLY", "FM | BPF | REV", "PWM | LPF | CHO", "NOI | BPF | FLA", "SINE | LPF | REV", "SYNC | LPF | DLY", "VOW | BPF | FLA", "SAW | LPF | DLY"};
    const char* phraseNames[10] = {"acid_line_01", "bass_groove", "arp_updown", "chord_stab", "lead_riff", "drum_loop", "poly_seq", "random_sH", "drone_fx", "melody_A"};
    const char* phraseParams[10] = {"BPM:120 TS:4/4 L:16", "BPM:110 TS:4/4 L:08", "BPM:130 TS:3/4 L:32", "BPM:120 TS:4/4 L:04", "BPM:125 TS:4/4 L:16", "BPM:140 TS:4/4 L:16", "BPM:115 TS:7/8 L:32", "BPM:120 TS:4/4 L:16", "BPM:090 TS:4/4 L:64", "BPM:120 TS:4/4 L:16"};

    StateManager() {
        menu.Init(); 
    }

    // Helper function to handle RGB LED and Menu loading simultaneously
    void SwitchMode(AppMode newMode, HardwareManager& hw) {
        currentMode = newMode;
        needsDisplayUpdate = true;

        if (newMode == AppMode::PLAY) {
            hw.rgb.Set(0, 0, 1); // Blue
        } else if (newMode == AppMode::PATCH_EDIT) {
            hw.rgb.Set(0, 1, 0); // Green
            menu.SetActiveTree(1);
        } else if (newMode == AppMode::PHRASE_EDIT) {
            hw.rgb.Set(1, 1, 0); // Yellow (R+G)
            menu.SetActiveTree(2);
        } else if (newMode == AppMode::SYSTEM_EDIT) {
            hw.rgb.Set(1, 0, 0); // Red
            menu.SetActiveTree(3);
        }
        
        hw.rgb.Update();
        hw.seed.DelayMs(300); // Debounce
    }

    void ProcessState(HardwareManager& hw) {
        // Run once right after boot animation to set LED to Blue
        if (!isBootInitialized) {
            hw.rgb.Set(0, 0, 1); 
            hw.rgb.Update();
            isBootInitialized = true;
        }

        // ==========================================
        // 1. IF IN A MENU MODE -> Route to MenuManager
        // ==========================================
        if (currentMode != AppMode::PLAY) {
            menu.ProcessInput(hw);
            
            bool backBtn = !hw.btnRed.Read();
            
            // If in the menu, the red button goes up a folder.
            if (backBtn && !menu.isEditing && menu.currentSelection->parent != nullptr) {
                menu.currentSelection = menu.currentSelection->parent;
                menu.topVisibleNode = menu.currentSelection;
                menu.needsDisplayUpdate = true;
                hw.seed.DelayMs(200); 
            }
            // If we are at the ROOT of the menu, the red button exits to PLAY mode.
            else if (backBtn && !menu.isEditing && menu.currentSelection->parent == nullptr) {
                SwitchMode(AppMode::PLAY, hw);
            }
            return; // Skip Play Mode inputs
        }

        // ==========================================
        // 2. PLAY MODE: COMBOS & NAVIGATION
        // ==========================================
        bool p1 = hw.enc1.Pressed(); 
        bool p2 = hw.enc2.Pressed();
        bool p3 = hw.enc3.Pressed();

        // Check Combos FIRST
        if (p1 && p2 && !p3) { SwitchMode(AppMode::PATCH_EDIT, hw); return; } 
        else if (!p1 && p2 && p3) { SwitchMode(AppMode::PHRASE_EDIT, hw); return; } 
        else if (p1 && !p2 && p3) { SwitchMode(AppMode::SYSTEM_EDIT, hw); return; }

        // --- ENCODER 1: SYNTH ---
        int inc1 = hw.enc1.Increment();
        if (inc1 != 0) {
            synthPreview += inc1;
            if (synthPreview > 9) synthPreview = 0;
            if (synthPreview < 0) synthPreview = 9;
            needsDisplayUpdate = true;
        }
        // Only trigger click if NO other encoders are being pressed (safety check)
        if (hw.enc1.RisingEdge() && !p2 && !p3) { 
            synthLoaded = synthPreview; 
            sprintf(sysMsg, "SYNTH LOADED");
            needsDisplayUpdate = true;
        }

        // --- ENCODER 2: PHRASE ---
        int inc2 = hw.enc2.Increment();
        if (inc2 != 0) {
            phrasePreview += inc2;
            if (phrasePreview > 9) phrasePreview = 0;
            if (phrasePreview < 0) phrasePreview = 9;
            needsDisplayUpdate = true;
        }
        if (hw.enc2.RisingEdge() && !p1 && !p3) { 
            phraseLoaded = phrasePreview; 
            sprintf(sysMsg, "PHRASE LOADED");
            needsDisplayUpdate = true;
        }

        // --- ENCODER 3: VOL/MIX ---
        int inc3 = hw.enc3.Increment();
        if (inc3 != 0) {
            if (isVolSelected) {
                mainVolume += inc3;
                if (mainVolume > 100) mainVolume = 100;
                if (mainVolume < 0) mainVolume = 0;
            } else {
                fxMix += inc3;
                if (fxMix > 100) fxMix = 100;
                if (fxMix < 0) fxMix = 0;
            }
            needsDisplayUpdate = true;
        }
        if (hw.enc3.RisingEdge() && !p1 && !p2) { 
            isVolSelected = !isVolSelected; 
            needsDisplayUpdate = true;
        }

        // Buttons (MIDI Simulation)
        bool GBPressed = !hw.btnGreen.Read();
        bool RBPressed = !hw.btnRed.Read();
        if (GBPressed != lastGB || RBPressed != lastRB) {
            needsDisplayUpdate = true;
            lastGB = GBPressed;
            lastRB = RBPressed;
        }
    }

    void DrawUI(HardwareManager& hw) {
        if (currentMode != AppMode::PLAY) {
            menu.DrawUI(hw); 
            return;
        }

        if (needsDisplayUpdate) {
            hw.display.Fill(false); 

            // MIDI Indicator
            if (lastGB || lastRB) {
                hw.display.DrawCircle(4, 4, 3, true); 
                hw.display.DrawCircle(4, 4, 2, true);
            }

            char valStr[40]; 
            
            // Volume Box
            hw.display.DrawRect(11, 0, 46, 9, true, isVolSelected); 
            hw.display.SetCursor(13, 1);
            sprintf(valStr, "V:%03d", mainVolume);
            hw.display.WriteString(valStr, Font_5x8, !isVolSelected); 

            // FX Box
            hw.display.DrawRect(49, 0, 84, 9, true, !isVolSelected); 
            hw.display.SetCursor(51, 1);
            sprintf(valStr, "F:%03d", fxMix);
            hw.display.WriteString(valStr, Font_5x8, isVolSelected); 

            // Mode Label
            hw.display.SetCursor(107, 0);
            hw.display.WriteString("PLAY", Font_5x8, true);
            
            hw.display.SetCursor(0, 9);
            hw.display.WriteString(sysMsg, Font_5x8, true);
            hw.display.DrawLine(0, 17, 127, 17, true);

            // ZONE 2: SYNTH
            bool synthIsFloating = (synthPreview != synthLoaded);
            if (synthIsFloating) hw.display.DrawRect(0, 18, 127, 28, true, true);
            
            hw.display.SetCursor(2, 19);
            sprintf(valStr, "%03d %s", synthPreview, synthNames[synthPreview]);
            hw.display.WriteString(valStr, Font_7x10, !synthIsFloating); 

            hw.display.SetCursor(2, 29);
            hw.display.WriteString(synthParams[synthPreview], Font_4x6, true);
            hw.display.DrawLine(0, 35, 127, 35, true);

            // ZONE 3: PHRASE
            bool phraseIsFloating = (phrasePreview != phraseLoaded);
            if (phraseIsFloating) hw.display.DrawRect(0, 36, 127, 44, true, true);
            
            hw.display.SetCursor(2, 37);
            sprintf(valStr, "%03d %s", phrasePreview, phraseNames[phrasePreview]);
            hw.display.WriteString(valStr, Font_5x8, !phraseIsFloating);

            hw.display.SetCursor(2, 45);
            hw.display.WriteString(phraseParams[phrasePreview], Font_4x6, true);
            hw.display.DrawLine(0, 52, 127, 52, true);

            // ZONE 4: HELP
            hw.display.SetCursor(0, 53); 
            hw.display.WriteString("R1:SYNTH R2:PHRASE R3:VOL/MIX", Font_4x6, true);
            hw.display.SetCursor(0, 59); 
            hw.display.WriteString("1+2:SYN EDT 2+3:PHR 1+3:SYS", Font_4x6, true);

            hw.display.Update();
            needsDisplayUpdate = false; 
        }
    }
};