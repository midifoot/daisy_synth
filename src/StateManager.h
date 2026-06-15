#pragma once
#include "HardwareManager.h"
#include "MenuManager.h" 
#include "MetronomeEngine.h"

enum class AppMode { PLAY, PATCH_EDIT, PHRASE_EDIT, SYSTEM_EDIT };

class StateManager {
public:
    AppMode currentMode = AppMode::PLAY;
    bool needsDisplayUpdate = true;
    bool isBootInitialized = false; 
    
    MenuManager menu; 
    MetronomeEngine metro; 

    bool isPanic = false; 

    uint32_t lastEnc1Time = 0;
    uint32_t lastEnc2Time = 0;
    uint32_t lastEnc3Time = 0;
    
    bool lastGB = false;
    uint32_t lastGBTime = 0; 
    
    bool lastRB = false;
    uint32_t redBtnPressTime = 0;
    bool redBtnLongTriggered = false;

    char sysMsg[40] = "STATUS: SYSTEM READY";
    
    int synthPreview = 0;
    int synthLoaded = 0;
    int phrasePreview = 0;
    int phraseLoaded = 0;
    
    bool isVolSelected = true; 
    int mainVolume = 50; 
    int fxMix = 50;

    int currentVuLevel = 4; 
    int currentCpuLoad = 12; 
    int sdSpaceUsed = 0; 
    bool midiActive = false; 

    const char* synthNames[10] = {"ARCADE B", "DEEP BASS", "SAW LEAD", "FM BELLS", "PULSE PAD", "NOISE HIT", "SOFT KEY", "HARD SYNC", "VOWEL SEQ", "ACID DROP"};
    const char* synthParams[10] = {"WAVE | LDR | REV", "SUB | LPF | CHO", "SAW | HPF | DLY", "FM | BPF | REV", "PWM | LPF | CHO", "NOI | BPF | FLA", "SINE | LPF | REV", "SYNC | LPF | DLY", "VOW | BPF | FLA", "SAW | LPF | DLY"};
    const char* phraseNames[10] = {"acid_line_01", "bass_groove", "arp_updown", "chord_stab", "lead_riff", "drum_loop", "poly_seq", "random_sH", "drone_fx", "melody_A"};
    const char* phraseParams[10] = {"BPM:120 TS:4/4 L:16", "BPM:110 TS:4/4 L:08", "BPM:130 TS:3/4 L:32", "BPM:120 TS:4/4 L:04", "BPM:125 TS:4/4 L:16", "BPM:140 TS:4/4 L:16", "BPM:115 TS:7/8 L:32", "BPM:120 TS:4/4 L:16", "BPM:090 TS:4/4 L:64", "BPM:120 TS:4/4 L:16"};

    StateManager() {
        menu.Init(); 
    }

    void UpdateLED(HardwareManager& hw) {
        if (!isBootInitialized || currentMode == AppMode::PLAY) hw.rgb.Set(0, 0, 1.0f); 
        else if (currentMode == AppMode::PATCH_EDIT) hw.rgb.Set(0, 1.0f, 0); 
        else if (currentMode == AppMode::PHRASE_EDIT) hw.rgb.Set(1.0f, 1.0f, 0); 
        else if (currentMode == AppMode::SYSTEM_EDIT) hw.rgb.Set(1.0f, 0, 0); 
        hw.rgb.Update();
    }

    void SwitchMode(AppMode newMode, HardwareManager& hw) {
        currentMode = newMode;
        if (newMode == AppMode::PATCH_EDIT) menu.SetActiveTree(1);
        else if (newMode == AppMode::PHRASE_EDIT) menu.SetActiveTree(2);
        else if (newMode == AppMode::SYSTEM_EDIT) menu.SetActiveTree(3);
        needsDisplayUpdate = true;
        UpdateLED(hw); 
        hw.seed.DelayMs(300); 
    }

    void ProcessState(HardwareManager& hw) {
        uint32_t now = hw.seed.system.GetNow();
        if (!isBootInitialized) { UpdateLED(hw); isBootInitialized = true; }

        // ==========================================
        // 1. MENU MODE LOGIC
        // ==========================================
        if (currentMode != AppMode::PLAY) {
            menu.ProcessInput(hw);
            
            metro.bpm = menu.n_bpm.value;
            metro.volume = menu.n_vol.value;
            metro.beats = menu.n_beats.value; // Updated mapping
            metro.subdiv = menu.n_sub.value;
            metro.ternary = (menu.n_ternary.value != 0);
            metro.UpdateTiming();

            // RESTORED: Short/Long Press Menu Navigation
            bool RBPressed = !hw.btnRed.Read();
            if (RBPressed && !lastRB) {
                redBtnPressTime = now;
                redBtnLongTriggered = false;
            }
            else if (RBPressed && lastRB) {
                if (!redBtnLongTriggered && (now - redBtnPressTime > 800)) {
                    SwitchMode(AppMode::PLAY, hw);
                    redBtnLongTriggered = true;
                }
            }
            else if (!RBPressed && lastRB) {
                if (!redBtnLongTriggered) {
                    if (menu.currentSelection->parent != nullptr) {
                        menu.currentSelection = menu.currentSelection->parent;
                        menu.topVisibleNode = menu.currentSelection;
                        menu.needsDisplayUpdate = true;
                    } else {
                        SwitchMode(AppMode::PLAY, hw);
                    }
                }
            }
            lastRB = RBPressed; 
            return; 
        }

        // ==========================================
        // 2. PLAY MODE LOGIC
        // ==========================================
        bool p1 = hw.enc1.Pressed(); 
        bool p2 = hw.enc2.Pressed();
        bool p3 = hw.enc3.Pressed();

        if (p1 && p2 && !p3) { SwitchMode(AppMode::PATCH_EDIT, hw); return; } 
        else if (!p1 && p2 && p3) { SwitchMode(AppMode::PHRASE_EDIT, hw); return; } 
        else if (p1 && !p2 && p3) { SwitchMode(AppMode::SYSTEM_EDIT, hw); return; }

        // --- GREEN BUTTON: METRONOME TOGGLE ---
        bool GBPressed = !hw.btnGreen.Read();
        if (GBPressed && !lastGB && (now - lastGBTime > 200)) {
            metro.running = !metro.running;
            sprintf(sysMsg, metro.running ? "METRO: ON" : "METRO: OFF");
            needsDisplayUpdate = true;
            lastGBTime = now;
        }
        lastGB = GBPressed;

        // --- RED BUTTON: AUDIO SHUTTER (PANIC) ---
        bool RBPressed = !hw.btnRed.Read();
        if (RBPressed && !lastRB) {
            isPanic = true;
            sprintf(sysMsg, "PANIC ACTIVE: MUTED");
            needsDisplayUpdate = true;
        }
        lastRB = RBPressed;

        // --- ENCODER 1 (RESTORED ACCELERATION) ---
        int inc1 = hw.enc1.Increment();
        if (inc1 != 0) {
            uint32_t timeDelta = now - lastEnc1Time;
            lastEnc1Time = now;
            int step = inc1; 
            if (timeDelta < 150) step = inc1 * 10; 

            synthPreview += step;
            while (synthPreview > 9) synthPreview -= 10;
            while (synthPreview < 0) synthPreview += 10;
            needsDisplayUpdate = true;
        }
        if (hw.enc1.RisingEdge() && !p2 && !p3) { synthLoaded = synthPreview; sprintf(sysMsg, "SYNTH LOADED"); needsDisplayUpdate = true; }

        // --- ENCODER 2 (RESTORED ACCELERATION) ---
        int inc2 = hw.enc2.Increment();
        if (inc2 != 0) {
            uint32_t timeDelta = now - lastEnc2Time;
            lastEnc2Time = now;
            int step = inc2; 
            if (timeDelta < 150) step = inc2 * 10; 

            phrasePreview += step;
            while (phrasePreview > 9) phrasePreview -= 10;
            while (phrasePreview < 0) phrasePreview += 10;
            needsDisplayUpdate = true;
        }
        if (hw.enc2.RisingEdge() && !p1 && !p3) { phraseLoaded = phrasePreview; sprintf(sysMsg, "PHRASE LOADED"); needsDisplayUpdate = true; }

        // --- ENCODER 3: VOL/MIX (RESTORED ACCELERATION) ---
        int inc3 = hw.enc3.Increment();
        if (isPanic && (inc3 != 0 || hw.enc3.RisingEdge())) { isPanic = false; sprintf(sysMsg, "STATUS: SYSTEM READY"); needsDisplayUpdate = true; }
        
        if (inc3 != 0) {
            uint32_t timeDelta = now - lastEnc3Time;
            lastEnc3Time = now;
            int step = inc3; 
            if (timeDelta < 150) step = inc3 * 10; 

            if (isVolSelected) { 
                mainVolume += step; 
                if (mainVolume > 100) mainVolume = 100; 
                if (mainVolume < 0) mainVolume = 0; 
            }
            else { 
                fxMix += step; 
                if (fxMix > 100) fxMix = 100; 
                if (fxMix < 0) fxMix = 0; 
            }
            needsDisplayUpdate = true;
        }
        if (hw.enc3.RisingEdge() && !p1 && !p2) { isVolSelected = !isVolSelected; needsDisplayUpdate = true; }
    }
};