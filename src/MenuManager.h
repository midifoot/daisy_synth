/*
defining a double list dynamic menu 
check 
MAX_VISIBLE_ITEMS
and
ITEM_HEIGHT
to change the menu appearance
*/
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
    
    // The three roots of our different menus
    MenuNode* rootSynth;
    MenuNode* rootPhrase;
    MenuNode* rootSystem;

    bool isEditing = false;
    bool needsDisplayUpdate = true;

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

        // Default to Synth
        currentSelection = rootSynth;
        topVisibleNode = currentSelection;
    }

    // NEW: Function to hot-swap the active menu based on combo keys
    void SetActiveTree(int treeID) {
        if (treeID == 1) currentSelection = rootSynth;
        else if (treeID == 2) currentSelection = rootPhrase;
        else if (treeID == 3) currentSelection = rootSystem;
        
        topVisibleNode = currentSelection;
        isEditing = false;
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
        int inc = hw.enc1.Increment();
        bool clicked = hw.enc1.RisingEdge();

        if (inc != 0) {
            if (isEditing && currentSelection->type == NodeType::PARAM_INT) {
                currentSelection->value += inc;
                if (currentSelection->value > currentSelection->maxVal) currentSelection->value = currentSelection->maxVal;
                if (currentSelection->value < currentSelection->minVal) currentSelection->value = currentSelection->minVal;
            } else {
                if (inc > 0 && currentSelection->next != nullptr) {
                    currentSelection = currentSelection->next;
                } else if (inc < 0 && currentSelection->prev != nullptr) {
                    currentSelection = currentSelection->prev;
                }
                UpdateCamera(); 
            }
            needsDisplayUpdate = true;
        }

        if (clicked) {
            if (currentSelection->type == NodeType::FOLDER) {
                if (currentSelection->child != nullptr) {
                    currentSelection = currentSelection->child;
                    topVisibleNode = currentSelection; 
                }
            } else if (currentSelection->type == NodeType::PARAM_INT) {
                isEditing = !isEditing;
            }
            needsDisplayUpdate = true;
        }
    }

    void DrawUI(HardwareManager& hw) {
        if (!needsDisplayUpdate) return;
        hw.display.Fill(false);
        
        hw.display.SetCursor(0, 0);
        if (currentSelection->parent == nullptr) hw.display.WriteString("- PARAMETERS -", Font_7x10, true);
        else hw.display.WriteString(currentSelection->parent->name, Font_7x10, true);
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
                
                if (isEditing && drawNode == currentSelection) {
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