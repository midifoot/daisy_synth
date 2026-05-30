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
        // 1. DRAWING A FRAME AROUND TEXT
        // ==========================================
        // DrawRect(StartX, StartY, EndX, EndY, Color, Fill)
        display.DrawRect(5, 5, 122, 22, true, false); 
        
        display.SetCursor(25, 10);
        display.WriteString("FRAMED TEXT!", Font_7x10, true);


        // ==========================================
        // 2. BUILDING A VU METER
        // ==========================================
        // First, let's create a fake "volume" value that goes up and down
        // We use the system timer and some math to make it bounce between 0 and 100
        int fake_volume = (System::GetNow() / 15) % 200; 
        if (fake_volume > 100) fake_volume = 200 - fake_volume; // Make it bounce back down

        // Draw the empty outer box for the VU meter
        display.DrawRect(14, 40, 114, 52, true, false);

        // Draw the solid inner bar that changes width based on 'fake_volume'
        // We add the volume value to the starting X coordinate (14)
        display.DrawRect(14, 40, 14 + fake_volume, 52, true, true);
        
        // Add some labels
        display.SetCursor(0, 42);
        display.WriteString("L", Font_6x8, true);
        display.SetCursor(120, 42);
        display.WriteString("R", Font_6x8, true);

        // Update the screen
        display.Update();

        // Very short delay so the VU meter moves smoothly
        System::Delay(10); 
    }
}