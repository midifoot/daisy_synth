#pragma once
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"
#include "hid/disp/oled_display.h"

using namespace daisy;

// Typedef for our specific OLED screen
using MyOledDriver = SSD130xI2c128x64Driver;

class HardwareManager {
public:
    HardwareManager() {}
    ~HardwareManager() {}

    // Public hardware objects
    DaisySeed seed;
    OledDisplay<MyOledDriver> display;
    
    Encoder enc1; // Synth
    Encoder enc2; // Phrase
    Encoder enc3; // Vol/Mix
    
    GPIO btnGreen;
    GPIO btnRed;
    
    RgbLed rgb;

    void Init() {
        // 1. Init System
        seed.Init();

        // 2. Init OLED (I2C on D11/D12)
        OledDisplay<MyOledDriver>::Config display_cfg;
        display_cfg.driver_config.transport_config.i2c_address = 0x3C;
        auto &i2c_cfg = display_cfg.driver_config.transport_config.i2c_config;
        i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
        i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        i2c_cfg.pin_config.scl = seed::D11; 
        i2c_cfg.pin_config.sda = seed::D12; 
        display.Init(display_cfg);

        // 3. Init Buttons (Physical Pins 8, 9)
        btnGreen.Init(seed::D7, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
        btnRed.Init(seed::D8, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

        // 4. Init Encoders (Phase A, Phase B, Click)
        enc1.Init(seed::D20, seed::D19, seed::D21); // Physical 26, 27, 28
        enc2.Init(seed::D23, seed::D22, seed::D24); // Physical 29, 30, 31
        enc3.Init(seed::D26, seed::D25, seed::D27); // Physical 32, 33, 34

        // 5. Init RGB LED (Physical Pins 22, 23, 24)
        rgb.Init(seed::D16, seed::D17, seed::D18, false);
    }

    void ProcessInputs() {
        // Debounce encoders every frame
        enc1.Debounce();
        enc2.Debounce();
        enc3.Debounce();
    }
};