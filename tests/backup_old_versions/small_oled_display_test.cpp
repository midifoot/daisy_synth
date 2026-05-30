#include "daisy_seed.h"
// Include the specific driver and the display template
#include "dev/oled_ssd130x.h"
#include "hid/disp/oled_display.h" 

using namespace daisy;

// 1. 'hw' avoids conflict with 'namespace seed'
DaisySeed hw; 

// 2. The driver definition for a 128x64 SSD1306 over I2C
// This is defined in dev/oled_ssd130x.h
using MyOledDriver = SSD130xI2c128x64Driver;
OledDisplay<MyOledDriver> display;

int main(void) {
    // Initialize Daisy Seed
    hw.Init();

    // 3. Configure the Display and I2C
    // In libDaisy, we configure the transport (I2C) inside the display config
    OledDisplay<MyOledDriver>::Config display_cfg;
    
    // 7-bit Address for a 0x78 labeled board is 0x3C
    display_cfg.driver_config.transport_config.i2c_address = 0x3C;
    
    // Setup I2C Peripheral and Pins (D11/D12 = Physical 12/13)
    auto &i2c_cfg = display_cfg.driver_config.transport_config.i2c_config;
    i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_cfg.pin_config.scl = seed::D11; // Physical 12
    i2c_cfg.pin_config.sda = seed::D12; // Physical 13

    // 4. Initialize the display
    display.Init(display_cfg);

    while(1) {
        // Clear the buffer
        display.Fill(false);

        // --- Drawing ---
        display.SetCursor(0, 0);
        display.WriteString("better than nothing ...", Font_7x10, true);

        display.SetCursor(0, 25);
        display.WriteString("Hey DAVID !", Font_11x18, true);

        display.SetCursor(0, 50);
        display.WriteString("I'm small but I work !", Font_6x8, true);

        // Update the screen
        display.Update();

        // Heartbeat LED
        hw.SetLed(true);
        System::Delay(250);
        hw.SetLed(false);
        System::Delay(250);
    }
}