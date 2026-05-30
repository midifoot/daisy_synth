#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"
#include "hid/disp/oled_display.h" 
#include <stdio.h> 

using namespace daisy;

DaisySeed hw; 
using MyOledDriver = SSD130xI2c128x64Driver;
OledDisplay<MyOledDriver> display;

// ==========================================
// DUMMY DATA LISTS FOR PREVIEW TESTING
// ==========================================
const char* synthNames[10] = {"ARCADE B", "DEEP BASS", "SAW LEAD", "FM BELLS", "PULSE PAD", "NOISE HIT", "SOFT KEY", "HARD SYNC", "VOWEL SEQ", "ACID DROP"};
const char* synthParams[10] = {"WAVE | LDR | REV", "SUB | LPF | CHO", "SAW | HPF | DLY", "FM | BPF | REV", "PWM | LPF | CHO", "NOI | BPF | FLA", "SINE | LPF | REV", "SYNC | LPF | DLY", "VOW | BPF | FLA", "SAW | LPF | DLY"};

const char* phraseNames[10] = {"acid_line_01", "bass_groove", "arp_updown", "chord_stab", "lead_riff", "drum_loop", "poly_seq", "random_sH", "drone_fx", "melody_A"};
const char* phraseParams[10] = {"BPM:120 TS:4/4 L:16", "BPM:110 TS:4/4 L:08", "BPM:130 TS:3/4 L:32", "BPM:120 TS:4/4 L:04", "BPM:125 TS:4/4 L:16", "BPM:140 TS:4/4 L:16", "BPM:115 TS:7/8 L:32", "BPM:120 TS:4/4 L:16", "BPM:090 TS:4/4 L:64", "BPM:120 TS:4/4 L:16"};

int main(void) {
    hw.Init();

    // ==========================================
    // 1. OLED INIT
    // ==========================================
    OledDisplay<MyOledDriver>::Config display_cfg;
    display_cfg.driver_config.transport_config.i2c_address = 0x3C;
    auto &i2c_cfg = display_cfg.driver_config.transport_config.i2c_config;
    i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_cfg.pin_config.scl = seed::D11; 
    i2c_cfg.pin_config.sda = seed::D12; 
    display.Init(display_cfg); 

    // ==========================================
    // 2. BUTTONS & ENCODERS INIT 
    // ==========================================
    daisy::GPIO G_button;
    daisy::GPIO R_button;
    G_button.Init(daisy::seed::D0, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::PULLUP);
    R_button.Init(daisy::seed::D1, daisy::GPIO::Mode::INPUT, daisy::GPIO::Pull::PULLUP);

    daisy::Encoder enc1;
    daisy::Encoder enc2;
    daisy::Encoder enc3;
    enc1.Init(seed::D3, seed::D2, seed::D4);   // Synth
    enc2.Init(seed::D6, seed::D5, seed::D7);   // Phrase
    enc3.Init(seed::D9, seed::D8, seed::D10);  // Vol/Mix

    // ==========================================
    // UI STATE VARIABLES
    // ==========================================
    bool needsDisplayUpdate = true; 
    bool lastGB = false;
    bool lastRB = false;
    char sysMsg[30] = "STATUS: SYSTEM READY";

    // Enc 1 (Synth) State
    int synthPreview = 0;
    int synthLoaded = 0;

    // Enc 2 (Phrase) State
    int phrasePreview = 0;
    int phraseLoaded = 0;

    // Enc 3 (Vol/Mix) State
    bool isVolSelected = true; // true = Vol, false = FX
    int mainVolume = 100;
    int fxMix = 50;
    
    // Velocity tracking only needed for Enc 3 now
    uint32_t lastTurnTime3 = 0; 
    const uint32_t fastTurnThreshold = 150; 
    const int fastMultiplier = 5;           

    while(1) {
        enc1.Debounce();
        enc2.Debounce();
        enc3.Debounce();
        System::Delay(1); 

        // --- CHECK BUTTONS (MIDI Indicator) ---
        bool GBPressed = !G_button.Read();
        bool RBPressed = !R_button.Read();
        if (GBPressed != lastGB || RBPressed != lastRB) {
            needsDisplayUpdate = true;
            lastGB = GBPressed;
            lastRB = RBPressed;
        }

        // ==========================================
        // --- ENCODER 1: SYNTH BROWSE/LOAD ---
        // ==========================================
        int inc1 = enc1.Increment();
        if (inc1 != 0) {
            synthPreview += inc1;
            // Wraparound the 10 item list (0-9)
            if (synthPreview > 9) synthPreview = 0;
            if (synthPreview < 0) synthPreview = 9;
            sprintf(sysMsg, "PREVIEWING SYNTH...");
            needsDisplayUpdate = true; 
        }
        if (enc1.RisingEdge()) { 
            synthLoaded = synthPreview; 
            sprintf(sysMsg, "SYNTH PATCH LOADED");
            needsDisplayUpdate = true;
        }

        // ==========================================
        // --- ENCODER 2: PHRASE BROWSE/LOAD ---
        // ==========================================
        int inc2 = enc2.Increment();
        if (inc2 != 0) {
            phrasePreview += inc2;
            if (phrasePreview > 9) phrasePreview = 0;
            if (phrasePreview < 0) phrasePreview = 9;
            sprintf(sysMsg, "PREVIEWING PHRASE...");
            needsDisplayUpdate = true; 
        }
        if (enc2.RisingEdge()) { 
            phraseLoaded = phrasePreview; 
            sprintf(sysMsg, "PHRASE LOADED");
            needsDisplayUpdate = true;
        }

        // ==========================================
        // --- ENCODER 3: VOL/FX MIX ---
        // ==========================================
        int inc3 = enc3.Increment();
        if (inc3 != 0) {
            uint32_t currentTime = System::GetNow();
            uint32_t timeDelta = currentTime - lastTurnTime3;
            lastTurnTime3 = currentTime;
            if (timeDelta > 2000) timeDelta = 2000; 

            int step = inc3; 
            if (timeDelta < fastTurnThreshold) step = inc3 * fastMultiplier; 

            if (isVolSelected) {
                mainVolume += step;
                if (mainVolume > 100) mainVolume = 100;
                if (mainVolume < 0) mainVolume = 0;
            } else {
                fxMix += step;
                if (fxMix > 100) fxMix = 100;
                if (fxMix < 0) fxMix = 0;
            }
            needsDisplayUpdate = true; 
        }
        // Toggle active parameter on push
        if (enc3.RisingEdge()) { 
            isVolSelected = !isVolSelected; 
            needsDisplayUpdate = true;
        }

        // ==========================================
        // DISPLAY DRAWING
        // ==========================================
        if (needsDisplayUpdate) {
            display.Fill(false); 

            // 1. MIDI Indicator
            if (GBPressed || RBPressed) {
                display.DrawCircle(4, 4, 3, true); 
                display.DrawCircle(4, 4, 2, true);
            }

            // 2. Dynamic Vol / FX Top Bar
            char valStr[10];
            
            // Draw Volume Box
            display.DrawRect(11, 0, 46, 9, true, isVolSelected); 
            display.SetCursor(13, 1);
            sprintf(valStr, "V:%03d", mainVolume);
            display.WriteString(valStr, Font_5x8, !isVolSelected); // Invert text if selected

            // Draw FX Box
            display.DrawRect(49, 0, 84, 9, true, !isVolSelected); 
            display.SetCursor(51, 1);
            sprintf(valStr, "F:%03d", fxMix);
            display.WriteString(valStr, Font_5x8, isVolSelected); // Invert text if selected

            // Mode Label
            display.SetCursor(107, 0);
            display.WriteString("PLAY", Font_5x8, true);
            
            // Line 2: System Messages
            display.SetCursor(0, 9);
            display.WriteString(sysMsg, Font_5x8, true);
            display.DrawLine(0, 17, 127, 17, true);

            // ZONE 2: SYNTHESIS
            bool synthIsFloating = (synthPreview != synthLoaded);
            if (synthIsFloating) {
                // Draw a filled block behind the text to indicate preview mode
                display.DrawRect(0, 18, 127, 28, true, true);
            }
            display.SetCursor(2, 19);
            sprintf(valStr, "%03d %s", synthPreview, synthNames[synthPreview]);
            // Write text in black if floating, white if loaded
            display.WriteString(valStr, Font_7x10, !synthIsFloating); 

            display.SetCursor(2, 29);
            display.WriteString(synthParams[synthPreview], Font_4x6, true);
            display.DrawLine(0, 35, 127, 35, true);

            // ZONE 3: PHRASER
            bool phraseIsFloating = (phrasePreview != phraseLoaded);
            if (phraseIsFloating) {
                display.DrawRect(0, 36, 127, 44, true, true);
            }
            display.SetCursor(2, 37);
            sprintf(valStr, "%03d %s", phrasePreview, phraseNames[phrasePreview]);
            display.WriteString(valStr, Font_5x8, !phraseIsFloating);

            display.SetCursor(2, 45);
            display.WriteString(phraseParams[phrasePreview], Font_4x6, true);
            display.DrawLine(0, 52, 127, 52, true);

            // ZONE 4: COMPRESSED HELP MENU
            display.SetCursor(0, 53); 
            display.WriteString("R1:SYNTH R2:PHRASE R3:VOL/MIX", Font_4x6, true);
            display.SetCursor(0, 59); 
            display.WriteString("RB:SYNTH EDIT GB:PHR EDIT", Font_4x6, true);

            display.Update();
            needsDisplayUpdate = false; 
        }
    }
}