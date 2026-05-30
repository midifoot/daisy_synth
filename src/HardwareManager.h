//HAL for all components

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

    // We make these public so the rest of the app can read them
    DaisySeed seed;
    OledDisplay<MyOledDriver> display;
    
    Encoder enc1; // Synth/Browse
    Encoder enc2; // Phrase
    Encoder enc3; // Vol/Mix
    
    GPIO btnRed;
    GPIO btnGreen;

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

        // 3. Init Encoders (Applying the inverted fix from yesterday)
        enc1.Init(seed::D3, seed::D2, seed::D4);
        enc2.Init(seed::D6, seed::D5, seed::D7);
        enc3.Init(seed::D9, seed::D8, seed::D10);

        // 4. Init Buttons
        btnGreen.Init(seed::D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
        btnRed.Init(seed::D1, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    }

    void ProcessInputs() {
        // Call this every frame to debounce all hardware
        enc1.Debounce();
        enc2.Debounce();
        enc3.Debounce();
        // (Note: GPIOs don't have a native Daisy debounce method, 
        // we just read them directly or build a custom debounce later)
    }
};