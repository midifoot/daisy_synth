#pragma once
#include "HardwareManager.h"
#include <stdio.h>

enum class NodeType { FOLDER, PARAM_INT };

struct MenuNode {
    const char* name;
    NodeType type;
    int value;
    int minVal;
    int maxVal;
    MenuNode* parent;
    MenuNode* child; 
    MenuNode* next;  
    MenuNode* prev;  
};

class MenuManager {
public:
    MenuNode* currentSelection;
    MenuNode* topVisibleNode; 
    
    MenuNode* rootSynth;
    MenuNode* rootPhrase;
    MenuNode* rootSystem;

    bool needsDisplayUpdate = true;

    // --- ACCELERATION TIMER FOR ENC 3 ---
    uint32_t lastMenuEnc3Time = 0; 

    const int MAX_VISIBLE_ITEMS = 4; 
    const int ITEM_HEIGHT = 12;      

    // --- SYNTH MENU DATA ---
    MenuNode n_osc, n_filter, n_lfo, n_fx; 
    MenuNode n_oscWave, n_oscTune, n_oscSub, n_oscFM, n_oscSync; 
    MenuNode n_fltCutoff, n_fltRes;

    // --- PHRASE MENU DATA ---
    MenuNode n_phrLength, n_phrSwing, n_phrProb, n_phrOctave;

    // --- SYSTEM MENU DATA ---
    MenuNode n_sysMidiCh, n_sysBright, n_sysClock;

    void Init() {
        // 1. BUILD SYNTH TREE
        n_osc    = {"Oscillators", NodeType::FOLDER, 0, 0, 0, nullptr, &n_oscWave, &n_filter, nullptr};
        n_filter = {"Filters",     NodeType::FOLDER, 0, 0, 0, nullptr, &n_fltCutoff, &n_lfo, &n_osc};
        n_lfo    = {"LFOs",        NodeType::FOLDER, 0, 0, 0, nullptr, nullptr, &n_fx, &n_filter};
        n_fx     = {"Effects",     NodeType::FOLDER, 0, 0, 0, nullptr, nullptr, nullptr, &n_lfo};

        n_oscWave = {"Waveform", NodeType::PARAM_INT, 1,  0, 4,   &n_osc, nullptr, &n_oscTune, nullptr};
        n_oscTune = {"Tuning",   NodeType::PARAM_INT, 50, 0, 100, &n_osc, nullptr, &n_oscSub, &n_oscWave};
        n_oscSub  = {"Sub Osc",  NodeType::PARAM_INT, 0,  0, 100, &n_osc, nullptr, &n_oscFM, &n_oscTune};
        n_oscFM   = {"FM Depth", NodeType::PARAM_INT, 0,  0, 100, &n_osc, nullptr, &n_oscSync, &n_oscSub};
        n_oscSync = {"Hard Sync",NodeType::PARAM_INT, 0,  0, 1,   &n_osc, nullptr, nullptr, &n_oscFM};

        n_fltCutoff = {"Cutoff", NodeType::PARAM_INT, 80, 0, 100, &n_filter, nullptr, &n_fltRes, nullptr};
        n_fltRes    = {"Resonance", NodeType::PARAM_INT, 10, 0, 100, &n_filter, nullptr, nullptr, &n_fltCutoff};
        rootSynth = &n_osc;

        // 2. BUILD PHRASE TREE
        n_phrLength = {"Seq Length", NodeType::PARAM_INT, 16, 1, 64, nullptr, nullptr, &n_phrSwing, nullptr};
        n_phrSwing  = {"Swing %",    NodeType::PARAM_INT, 50, 0, 100, nullptr, nullptr, &n_phrProb, &n_phrLength};
        n_phrProb   = {"Probability",NodeType::PARAM_INT, 100, 0, 100, nullptr, nullptr, &n_phrOctave, &n_phrSwing};
        n_phrOctave = {"Octave Rng", NodeType::PARAM_INT, 2,  1, 4, nullptr, nullptr, nullptr, &n_phrProb};
        rootPhrase = &n_phrLength;

        // 3. BUILD SYSTEM TREE
        n_sysMidiCh = {"MIDI Channel", NodeType::PARAM_INT, 1, 1, 16, nullptr, nullptr, &n_sysBright, nullptr};
        n_sysBright = {"Brightness",   NodeType::PARAM_INT, 100, 10, 100, nullptr, nullptr, &n_sysClock, &n_sysMidiCh};
        n_sysClock  = {"Clock Src",    NodeType::PARAM_INT, 0, 0, 1, nullptr, nullptr, nullptr, &n_sysBright}; 
        rootSystem = &n_sysMidiCh;

        currentSelection = rootSynth;
        topVisibleNode = currentSelection;
    }

    void SetActiveTree(int treeID) {
        if (treeID == 1) currentSelection = rootSynth;
        else if (treeID == 2) currentSelection = rootPhrase;
        else if (treeID == 3) currentSelection = rootSystem;
        
        topVisibleNode = currentSelection;
        needsDisplayUpdate = true;
    }

    void UpdateCamera() {
        MenuNode* check = topVisibleNode;
        while (check != nullptr) {
            if (check == currentSelection) {
                topVisibleNode = currentSelection;
                return;
            }
            check = check->prev;
        }

        check = topVisibleNode;
        for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
            if (check == currentSelection) return; 
            if (check == nullptr) break;
            check = check->next;
        }
        
        check = currentSelection;
        for(int i = 0; i < MAX_VISIBLE_ITEMS - 1; i++) {
            if (check->prev != nullptr) check = check->prev;
        }
        topVisibleNode = check;
    }

    void ProcessInput(HardwareManager& hw) {
        uint32_t now = hw.seed.system.GetNow();

        // ==========================================
        // ENCODER 1: STRICTLY NAVIGATION (Target)
        // ==========================================
        int inc1 = hw.enc1.Increment();
        bool clicked1 = hw.enc1.RisingEdge();

        if (inc1 != 0) {
            if (inc1 > 0 && currentSelection->next != nullptr) {
                currentSelection = currentSelection->next;
            } else if (inc1 < 0 && currentSelection->prev != nullptr) {
                currentSelection = currentSelection->prev;
            }
            UpdateCamera(); 
            needsDisplayUpdate = true;
        }

        if (clicked1) {
            // Push only works on folders now!
            if (currentSelection->type == NodeType::FOLDER && currentSelection->child != nullptr) {
                currentSelection = currentSelection->child;
                topVisibleNode = currentSelection; 
                needsDisplayUpdate = true;
            }
        }

        // ==========================================
        // ENCODER 3: STRICTLY VALUE EDITING (Fire)
        // ==========================================
        int inc3 = hw.enc3.Increment();
        if (inc3 != 0 && currentSelection->type == NodeType::PARAM_INT) {
            uint32_t timeDelta = now - lastMenuEnc3Time;
            lastMenuEnc3Time = now;
            if (timeDelta > 2000) timeDelta = 2000; 

            int step = inc3;
            if (timeDelta < 150) step = inc3 * 10; // FAST TURN JUMPS BY 10

            currentSelection->value += step;
            if (currentSelection->value > currentSelection->maxVal) currentSelection->value = currentSelection->maxVal;
            if (currentSelection->value < currentSelection->minVal) currentSelection->value = currentSelection->minVal;
            
            needsDisplayUpdate = true;
        }
    }

    void DrawUI(HardwareManager& hw) {
        if (!needsDisplayUpdate) return;
        hw.display.Fill(false);
        
        hw.display.SetCursor(0, 0);
        
        if (currentSelection->parent == nullptr) {
            if (currentSelection == rootSynth) hw.display.WriteString("SYNTH EDIT", Font_7x10, true);
            else if (currentSelection == rootPhrase) hw.display.WriteString("PHRASE EDIT", Font_7x10, true);
            else if (currentSelection == rootSystem) hw.display.WriteString("SYSTEM EDIT", Font_7x10, true);
            else hw.display.WriteString("- PARAMETERS -", Font_7x10, true); 
        } else {
            hw.display.WriteString(currentSelection->parent->name, Font_7x10, true);
        }
        
        hw.display.DrawLine(0, 11, 127, 11, true);

        int yPos = 14;
        MenuNode* drawNode = topVisibleNode;
        int itemsDrawn = 0;

        while (drawNode != nullptr && itemsDrawn < MAX_VISIBLE_ITEMS) {
            if (drawNode == currentSelection) {
                hw.display.SetCursor(0, yPos);
                hw.display.WriteString(">", Font_7x10, true);
            }

            hw.display.SetCursor(10, yPos);
            hw.display.WriteString(drawNode->name, Font_7x10, true);

            if (drawNode->type == NodeType::PARAM_INT) {
                char valStr[16];
                sprintf(valStr, ": %d", drawNode->value);
                hw.display.WriteString(valStr, Font_7x10, true);
                
                // Always draw the box if it is the current selection, to show E3 is "live"
                if (drawNode == currentSelection) {
                    hw.display.DrawRect(0, yPos-1, 127, yPos + ITEM_HEIGHT - 2, true, false); 
                }
            } else if (drawNode->type == NodeType::FOLDER) {
                hw.display.WriteString(" [...]", Font_7x10, true);
            }

            drawNode = drawNode->next;
            yPos += ITEM_HEIGHT;
            itemsDrawn++;
        }

        if (drawNode != nullptr) {
            hw.display.DrawPixel(126, 60, true);
            hw.display.DrawPixel(125, 59, true);
            hw.display.DrawPixel(127, 59, true); 
        }
        if (topVisibleNode->prev != nullptr) {
            hw.display.DrawPixel(126, 14, true);
            hw.display.DrawPixel(125, 15, true);
            hw.display.DrawPixel(127, 15, true); 
        }

        hw.display.Update();
        needsDisplayUpdate = false;
    }
};