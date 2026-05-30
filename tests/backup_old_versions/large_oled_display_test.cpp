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
        display.Fill(false);
        // remind font size : Font_widthxheights
        display.SetCursor(0, 0);
        display.WriteString("2.42 OLED Test", Font_7x10, true);

        display.SetCursor(0, 11);
        display.WriteString("small text 4x8 end of line ...", Font_4x8, true);
        
        display.SetCursor(0, 20);
        display.WriteString("DAISY SYNTH !", Font_11x18, true);
        
        display.SetCursor(0, 39);
        display.WriteString("another font size 6x7", Font_6x7, true);
        
        display.SetCursor(0, 48);
        display.WriteString("I'm ALIVE & HUNGRY !", Font_6x8, true);

        display.SetCursor(0, 56);
        display.WriteString("End of the line .....", Font_6x8, true);
        display.Update();

        // Heartbeat LED
        hw.SetLed(true);   System::Delay(250);
        hw.SetLed(false);  System::Delay(250);
    }
}