#include "daisy_seed.h"
using namespace daisy;

DaisySeed hw;

// Manual Pin Definitions (Physical 8, 9, 10, 11)
GPIO m_cs, m_sck, m_mosi, m_miso;

// A very slow, manual "Send Byte" function
uint8_t BitBang_Transfer(uint8_t data) {
    uint8_t read_val = 0;
    for (int i = 0; i < 8; i++) {
        // Set MOSI (MSB first)
        m_mosi.Write((data & 0x80) != 0);
        data <<= 1;

        // Pulse Clock HIGH
        System::DelayUs(10); // Very slow for stability
        m_sck.Write(true);
        System::DelayUs(10);

        // Read MISO
        read_val <<= 1;
        if (m_miso.Read()) read_val |= 0x01;

        // Pulse Clock LOW
        m_sck.Write(false);
    }
    return read_val;
}

int main(void) {
    hw.Init();

    // Initialize Pins as manual GPIOs
    m_cs.Init(seed::D7, GPIO::Mode::OUTPUT);       // Pin 8
    m_sck.Init(seed::D8, GPIO::Mode::OUTPUT);      // Pin 9
    m_mosi.Init(seed::D10, GPIO::Mode::OUTPUT);    // Pin 11
    m_miso.Init(seed::D9, GPIO::Mode::INPUT, GPIO::Pull::PULLUP); // Pin 10

    m_cs.Write(true);
    m_sck.Write(false);

    while(1) {
        // 1. Wake up sequence (80+ clocks)
        m_cs.Write(true);
        for(int i = 0; i < 20; i++) BitBang_Transfer(0xFF);

        // 2. Send CMD0 (Reset)
        // 0x40, 0, 0, 0, 0, 0x95
        m_cs.Write(false);
        System::DelayUs(100);
        
        BitBang_Transfer(0x40);
        BitBang_Transfer(0x00);
        BitBang_Transfer(0x00);
        BitBang_Transfer(0x00);
        BitBang_Transfer(0x00);
        BitBang_Transfer(0x95);

        // 3. Wait for response
        uint8_t res = 0xFF;
        for(int i = 0; i < 100; i++) {
            res = BitBang_Transfer(0xFF);
            if(res != 0xFF) break;
        }
        m_cs.Write(true);

        // --- VERDICT ---
        if(res == 0x01) {
            // SUCCESS (Fast Blink)
            hw.SetLed(true); System::Delay(50);
            hw.SetLed(false); System::Delay(50);
        } else {
            // FAIL (Slow Pulse)
            hw.SetLed(true); System::Delay(1000);
            hw.SetLed(false); System::Delay(1000);
        }
        System::Delay(1000);
    }
}