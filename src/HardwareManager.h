#pragma once
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"
#include "hid/disp/oled_display.h"

using namespace daisy;

using MyOledDriver = SSD130xI2c128x64Driver;

class HardwareManager {
public:
    HardwareManager() {}
    ~HardwareManager() {}

    DaisySeed seed;
    OledDisplay<MyOledDriver> display;
    
    // The hardware MIDI serial listener engine
    MidiUartHandler midi;

    Encoder enc1; 
    Encoder enc2; 
    Encoder enc3; 
    
    GPIO btnGreen;
    GPIO btnRed;
    
    RgbLed rgb;

    void Init() {
        seed.Init();

        // 1. Init OLED
        OledDisplay<MyOledDriver>::Config display_cfg;
        display_cfg.driver_config.transport_config.i2c_address = 0x3C;
        auto &i2c_cfg = display_cfg.driver_config.transport_config.i2c_config;
        i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_1;
        i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        i2c_cfg.pin_config.scl = seed::D11; 
        i2c_cfg.pin_config.sda = seed::D12; 
        display.Init(display_cfg);

        // 2. LOGICAL FIX: Explicitly assign the physical pins to the transport structure
        MidiUartHandler::Config midi_cfg;
        midi_cfg.transport_config.rx = seed::D14; // Pin 14 maps to your voltage divider!
        midi_cfg.transport_config.tx = seed::D13; // Pin 13 maps to serial out (unused for now)
        midi.Init(midi_cfg);

        // 3. Init Buttons
        btnGreen.Init(seed::D7, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
        btnRed.Init(seed::D8, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

        // 4. Init Encoders
        enc1.Init(seed::D20, seed::D19, seed::D21); 
        enc2.Init(seed::D23, seed::D22, seed::D24); 
        enc3.Init(seed::D26, seed::D25, seed::D27); 

        // 5. Init RGB LED
        rgb.Init(seed::D16, seed::D17, seed::D18, false);
    }

    void ProcessInputs() {
        enc1.Debounce();
        enc2.Debounce();
        enc3.Debounce();
    }
};