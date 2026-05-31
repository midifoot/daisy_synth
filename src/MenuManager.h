/*
defining a double list dynamic menu 
SEE 
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
    MenuNode* topVisibleNode; // NEW: The "Camera" that tracks the top of the screen
    
    bool isEditing = false;
    bool needsDisplayUpdate = true;

    // --- UI SETTINGS ---
    const int MAX_VISIBLE_ITEMS = 4; // Change this to 3 if you use a larger font
    const int ITEM_HEIGHT = 12;      // Spacing between rows

    // --- DUMMY DATA ---
    MenuNode n_osc, n_filter, n_lfo, n_fx; // Added an extra folder to test scrolling
    MenuNode n_oscWave, n_oscTune, n_oscSub, n_oscFM, n_oscSync; // 5 items to test scrolling
    MenuNode n_fltCutoff, n_fltRes;

    void Init() {
        // ROOT LEVEL
        n_osc    = {"Oscillators", NodeType::FOLDER, 0, 0, 0, nullptr, &n_oscWave, &n_filter, nullptr};
        n_filter = {"Filters",     NodeType::FOLDER, 0, 0, 0, nullptr, &n_fltCutoff, &n_lfo, &n_osc};
        n_lfo    = {"LFOs",        NodeType::FOLDER, 0, 0, 0, nullptr, nullptr, &n_fx, &n_filter};
        n_fx     = {"Effects",     NodeType::FOLDER, 0, 0, 0, nullptr, nullptr, nullptr, &n_lfo};

        // OSCILLATOR CHILDREN (5 items so we can test the scroll!)
        n_oscWave = {"Waveform", NodeType::PARAM_INT, 1,  0, 4,   &n_osc, nullptr, &n_oscTune, nullptr};
        n_oscTune = {"Tuning",   NodeType::PARAM_INT, 50, 0, 100, &n_osc, nullptr, &n_oscSub, &n_oscWave};
        n_oscSub  = {"Sub Osc",  NodeType::PARAM_INT, 0,  0, 100, &n_osc, nullptr, &n_oscFM, &n_oscTune};
        n_oscFM   = {"FM Depth", NodeType::PARAM_INT, 0,  0, 100, &n_osc, nullptr, &n_oscSync, &n_oscSub};
        n_oscSync = {"Hard Sync",NodeType::PARAM_INT, 0,  0, 1,   &n_osc, nullptr, nullptr, &n_oscFM};

        // FILTER CHILDREN
        n_fltCutoff = {"Cutoff", NodeType::PARAM_INT, 80, 0, 100, &n_filter, nullptr, &n_fltRes, nullptr};
        n_fltRes    = {"Resonance", NodeType::PARAM_INT, 10, 0, 100, &n_filter, nullptr, nullptr, &n_fltCutoff};

        currentSelection = &n_osc;
        topVisibleNode = currentSelection;
    }

    // NEW: The Viewport Logic
    void UpdateCamera() {
        // 1. Is the cursor ABOVE the camera? If so, snap camera to cursor.
        MenuNode* check = topVisibleNode;
        while (check != nullptr) {
            if (check == currentSelection) {
                topVisibleNode = currentSelection;
                return;
            }
            check = check->prev;
        }

        // 2. Is the cursor BELOW the camera viewport?
        check = topVisibleNode;
        for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
            if (check == currentSelection) return; // It is safely on screen!
            if (check == nullptr) break;
            check = check->next;
        }
        
        // 3. If we reach here, the cursor went off the bottom of the screen.
        // Move the camera down so the cursor becomes the very bottom item.
        check = currentSelection;
        for(int i = 0; i < MAX_VISIBLE_ITEMS - 1; i++) {
            if (check->prev != nullptr) check = check->prev;
        }
        topVisibleNode = check;
    }

    void ProcessInput(HardwareManager& hw) {
        int inc = hw.enc1.Increment();
        bool clicked = hw.enc1.RisingEdge();
        bool backBtn = !hw.btnRed.Read(); 

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
                UpdateCamera(); // Keep the screen locked onto the cursor
            }
            needsDisplayUpdate = true;
        }

        if (clicked) {
            if (currentSelection->type == NodeType::FOLDER) {
                if (currentSelection->child != nullptr) {
                    currentSelection = currentSelection->child;
                    topVisibleNode = currentSelection; // Reset camera when entering folder
                }
            } else if (currentSelection->type == NodeType::PARAM_INT) {
                isEditing = !isEditing;
            }
            needsDisplayUpdate = true;
        }

        if (backBtn) {
            if (isEditing) {
                isEditing = false; 
                hw.seed.DelayMs(200); 
            } else if (currentSelection->parent != nullptr) {
                currentSelection = currentSelection->parent;
                topVisibleNode = currentSelection; // Reset camera when exiting folder
                hw.seed.DelayMs(200); 
            }
            needsDisplayUpdate = true;
        }
    }

    void DrawUI(HardwareManager& hw) {
        if (!needsDisplayUpdate) return;

        hw.display.Fill(false);
        
        // 1. Draw Header
        hw.display.SetCursor(0, 0);
        if (currentSelection->parent == nullptr) {
            hw.display.WriteString("- MAIN MENU -", Font_7x10, true);
        } else {
            hw.display.WriteString(currentSelection->parent->name, Font_7x10, true);
        }
        hw.display.DrawLine(0, 11, 127, 11, true);

        // 2. Draw List starting strictly from topVisibleNode
        int yPos = 14;
        MenuNode* drawNode = topVisibleNode;
        int itemsDrawn = 0;

        while (drawNode != nullptr && itemsDrawn < MAX_VISIBLE_ITEMS) {
            
            // Draw Cursor
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

        // 3. Draw a scrollbar indicator on the right edge if there are more items below
        if (drawNode != nullptr) {
            hw.display.DrawPixel(126, 60, true);
            hw.display.DrawPixel(125, 59, true);
            hw.display.DrawPixel(127, 59, true); // Little down arrow
        }
        // Draw up arrow if there are items above
        if (topVisibleNode->prev != nullptr) {
            hw.display.DrawPixel(126, 14, true);
            hw.display.DrawPixel(125, 15, true);
            hw.display.DrawPixel(127, 15, true); // Little up arrow
        }

        hw.display.Update();
        needsDisplayUpdate = false;
    }
};