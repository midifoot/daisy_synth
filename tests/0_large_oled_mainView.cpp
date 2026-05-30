#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"
#include "hid/disp/oled_display.h" 

using namespace daisy;

DaisySeed hw; 

// THE FIX: We use the SSD130x driver, which inherently supports your 2.42" SSD1309
using MyOledDriver = SSD130xI2c128x64Driver;
OledDisplay<MyOledDriver> display;

int main(void) {
    hw.Init();

    OledDisplay<MyOledDriver>::Config display_cfg;
    
    // I2C address for a 2.42" OLED is usually 0x3C
    display_cfg.driver_config.transport_config.i2c_address = 0x3C;
    
    // I2C_1 Configuration matching Physical Pins 12 and 13
    auto &i2c_cfg = display_cfg.driver_config.transport_config.i2c_config;
    i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_cfg.pin_config.scl = seed::D11; // Physical Pin 12 (Right side, counting UP from bottom)
    i2c_cfg.pin_config.sda = seed::D12; // Physical Pin 13 (Right side, counting UP from bottom)

    display.Init(display_cfg); // display is 128 x 64 

while(1) {
        display.Fill(false); // Clear screen

        // ==========================================
        // ZONE 1: HEADER & SYSTEM (Y: 0 to 16)
        // ==========================================
        
        // 1. MIDI Indicator (Small Circle)
        // display.DrawCircle(4, 4, 3, true); 
        display.DrawCircle(4, 4, 3, true); 
        display.DrawCircle(4, 4, 2, true);
        // display.DrawCircle(4, 4, 1, true);
        //display.DrawPixel(4, 4, true);

// 2. Draw the solid white highlight box for the first number
        // X goes from 11 to 23 (12 pixels wide, enough for two digits + padding)
        // Y goes from 0 to 9 (10 pixels high, enough for the 8px font + padding)
        display.DrawRect(11, 0, 25, 9, true, true); 

        // 2. Write the first number INSIDE the box 
        // Notice the last parameter is 'false' - this draws black pixels to carve out the text!
        display.SetCursor(13, 0);
        display.WriteString("100", Font_5x8, false);

        // 3. Write the separator immediately after the box
        // Last parameter is 'true' because we are back to drawing normal white text
        display.SetCursor(29, 0);
        display.WriteString("/", Font_5x8, true);

        // 4. Write the second number immediately after the separator
        display.SetCursor(34, 0);
        display.WriteString("100", Font_5x8, true);

        // 3. Mini Audio VU Meter (Horizontal bar)
        display.DrawRect(50, 1, 82, 7, true, false); // Outline
        display.DrawRect(50, 1, 62, 7, true, true);  // Active level

        // 4. CPU Load
        display.SetCursor(86, 0);
        display.WriteString("100%", Font_5x8, true);

        // 5. Mode
        display.SetCursor(107, 0);
        display.WriteString("PLAY", Font_5x8, true);

        // Line 2: System Messages
        display.SetCursor(0, 8);
        display.WriteString("STATUS: SYSTEM READY", Font_5x8, true);

        // Separator Line
        display.DrawLine(0, 17, 127, 17, true);

        // ==========================================
        // ZONE 2: SYNTHESIS (Y: 19 to 35)
        // ==========================================
        
        // Preset Number & Name (Using slightly larger 7x10 font)
        display.SetCursor(0, 19);
        display.WriteString("024 ARCADE B", Font_7x10, true);

        // Synth Model | Filter | FX
        display.SetCursor(0, 28);
        display.WriteString("WAVE | LADDER | REVERB", Font_4x6, true);

        // Separator Line
        display.DrawLine(0, 35, 127, 35, true);

        // ==========================================
        // ZONE 3: PHRASER (Y: 36 to 51)
        // ==========================================
        
        display.SetCursor(0, 36);
        display.WriteString("012 acid_line_01", Font_5x8, true);

        display.SetCursor(0, 44);
        display.WriteString("BPM:120  TS:4/4  L:16", Font_4x6, true);

        // Separator Line
        display.DrawLine(0, 51, 127, 51, true);

        // ==========================================
        // ZONE 4: COMPRESSED HELP MENU (52 TO 64)
        // ==========================================
        display.SetCursor(0, 52); // Bottom of the screen
        display.WriteString("R1:SYNTH R2:PHRASE R3:VOL/MIX", Font_4x6, true);

        display.SetCursor(0, 58); // Bottom of the screen
        display.WriteString("RB:SYNTH EDIT GB:PHR EDIT", Font_4x6, true);

        // Update the screen
        display.Update();

        // Slow down the loop slightly
        System::Delay(50); 
    }
}