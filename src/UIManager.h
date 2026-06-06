#pragma once
#include "HardwareManager.h"
#include "StateManager.h"

class UIManager {
public:
    void Draw(HardwareManager& hw, StateManager& state) {
        if (state.currentMode != AppMode::PLAY) {
            if (!state.menu.needsDisplayUpdate) return;
            
            hw.display.Fill(false);
            DrawMenu(hw, state.menu);
            hw.display.Update();
            
            state.menu.needsDisplayUpdate = false;
        } else {
            if (!state.needsDisplayUpdate) return;
            
            hw.display.Fill(false);
            DrawPlayMode(hw, state);
            hw.display.Update();
            
            state.needsDisplayUpdate = false;
        }
    }

private:
    void DrawMenu(HardwareManager& hw, MenuManager& menu) {
        hw.display.SetCursor(0, 0);
        
        if (menu.currentSelection->parent == nullptr) {
            if (menu.currentSelection == menu.rootSynth) hw.display.WriteString("SYNTH EDIT", Font_7x10, true);
            else if (menu.currentSelection == menu.rootPhrase) hw.display.WriteString("PHRASE EDIT", Font_7x10, true);
            else if (menu.currentSelection == menu.rootSystem) hw.display.WriteString("SYSTEM EDIT", Font_7x10, true);
            else hw.display.WriteString("- PARAMETERS -", Font_7x10, true); 
        } else {
            hw.display.WriteString(menu.currentSelection->parent->name, Font_7x10, true);
        }
        
        hw.display.DrawLine(0, 11, 127, 11, true);

        int yPos = 14;
        MenuNode* drawNode = menu.topVisibleNode;
        int itemsDrawn = 0;

        while (drawNode != nullptr && itemsDrawn < menu.MAX_VISIBLE_ITEMS) {
            if (drawNode == menu.currentSelection) {
                hw.display.SetCursor(0, yPos);
                hw.display.WriteString(">", Font_7x10, true);
            }

            hw.display.SetCursor(10, yPos);
            hw.display.WriteString(drawNode->name, Font_7x10, true);

            if (drawNode->type == NodeType::PARAM_INT) {
                char valStr[16];
                sprintf(valStr, ": %d", drawNode->value);
                hw.display.WriteString(valStr, Font_7x10, true);
                
                if (drawNode == menu.currentSelection) {
                    hw.display.DrawRect(0, yPos-1, 127, yPos + menu.ITEM_HEIGHT - 2, true, false); 
                }
            } else if (drawNode->type == NodeType::FOLDER) {
                hw.display.WriteString(" [...]", Font_7x10, true);
            }

            drawNode = drawNode->next;
            yPos += menu.ITEM_HEIGHT;
            itemsDrawn++;
        }

        if (drawNode != nullptr) {
            hw.display.DrawPixel(126, 60, true);
            hw.display.DrawPixel(125, 59, true);
            hw.display.DrawPixel(127, 59, true); 
        }
        if (menu.topVisibleNode->prev != nullptr) {
            hw.display.DrawPixel(126, 14, true);
            hw.display.DrawPixel(125, 15, true);
            hw.display.DrawPixel(127, 15, true); 
        }
    }

    void DrawPlayMode(HardwareManager& hw, StateManager& state) {
        // ZONE 1: THE STATUS BAR (MIDI Activity Blinker)
        if (state.midiActive) {
            hw.display.DrawCircle(2, 4, 2, true); // Outer circle
            hw.display.DrawCircle(2, 4, 1, true); // Inner fill
        }

        char valStr[40]; 
        
        hw.display.DrawRect(7, 0, 36, 9, true, state.isVolSelected); 
        hw.display.SetCursor(9, 0); 
        sprintf(valStr, "V:%03d", state.mainVolume);
        hw.display.WriteString(valStr, Font_5x8, !state.isVolSelected); 

        hw.display.DrawRect(38, 0, 67, 9, true, !state.isVolSelected); 
        hw.display.SetCursor(40, 0); 
        sprintf(valStr, "F:%03d", state.fxMix);
        hw.display.WriteString(valStr, Font_5x8, state.isVolSelected); 

        // Static VU Meter 
        for(int i = 0; i < 7; i++) {
            int x_vu = 73 + (i * 3);
            int h_vu = i + 2; 
            if (i < state.currentVuLevel) hw.display.DrawLine(x_vu, 8, x_vu, 8 - h_vu, true); 
            else hw.display.DrawPixel(x_vu, 8, true); 
        }

        // CPU LOAD + SD space left
        hw.display.SetCursor(99, 2);
        sprintf(valStr, "%02d%% %02d%%", state.currentCpuLoad, state.sdSpaceUsed);
        hw.display.WriteString(valStr, Font_4x6, true);
        
        // LOWER ZONES
        hw.display.SetCursor(0, 10);
        hw.display.WriteString(state.sysMsg, Font_5x8, true);
        hw.display.DrawLine(0, 19, 127, 19, true); 

        // ZONE 2: SYNTH
        bool synthIsFloating = (state.synthPreview != state.synthLoaded);
        if (synthIsFloating) hw.display.DrawRect(0, 21, 127, 30, true, true);
        hw.display.SetCursor(2, 21);
        sprintf(valStr, "%03d %s", state.synthPreview, state.synthNames[state.synthPreview]);
        hw.display.WriteString(valStr, Font_7x10, !synthIsFloating); 
        
        hw.display.SetCursor(2, 31);
        hw.display.WriteString(state.synthParams[state.synthPreview], Font_4x6, true);
        hw.display.DrawLine(0, 37, 127, 37, true); 

        // ZONE 3: PHRASE
        bool phraseIsFloating = (state.phrasePreview != state.phraseLoaded);
        if (phraseIsFloating) hw.display.DrawRect(0, 39, 127, 46, true, true);
        hw.display.SetCursor(2, 39);
        sprintf(valStr, "%03d %s", state.phrasePreview, state.phraseNames[state.phrasePreview]);
        hw.display.WriteString(valStr, Font_5x8, !phraseIsFloating);
        
        hw.display.SetCursor(2, 48);
        hw.display.WriteString(state.phraseParams[state.phrasePreview], Font_4x6, true);
        hw.display.DrawLine(0, 54, 127, 54, true);

        // ZONE 4: HELP
        hw.display.SetCursor(0, 55); 
        hw.display.WriteString("R1:SYNTH R2:PHRASE R3:VOL/MIX", Font_4x6, true);
        hw.display.SetCursor(0, 61); 
        hw.display.WriteString("1+2:SYN EDT 2+3:PHR 1+3:SYS", Font_4x6, true);
    }
};