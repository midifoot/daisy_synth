#pragma once
#ifndef MDS_HARDWARE_H
#define MDS_HARDWARE_H

#include "daisy_seed.h"

using namespace daisy;

struct MDS_Hardware {
    DaisySeed seed;
    SpiHandle spi;
    GPIO pin_dc, pin_rst, pin_cs;

    void Init() {
        seed.Init();

        // 1. Setup Control Pins (DC, RST, CS)
        pin_dc.Init(seed::D24, GPIO::Mode::OUTPUT);
        pin_rst.Init(seed::D25, GPIO::Mode::OUTPUT);
        pin_cs.Init(seed::D21, GPIO::Mode::OUTPUT);

        // 2. Hardware Reset
        pin_rst.Write(false);
        System::Delay(100);
        pin_rst.Write(true);
        System::Delay(200);

        // 3. Setup Hardware SPI1 (Pins 8 and 11)
        SpiHandle::Config spi_conf;
        spi_conf.mode           = SpiHandle::Config::Mode::MASTER;
        spi_conf.periph         = SpiHandle::Config::Peripheral::SPI_1;
        spi_conf.direction      = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
        spi_conf.datasize       = 8;
        spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
        spi_conf.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE; // Mode 0
        spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_128;
        
        // Pin Mapping for SPI1
        spi_conf.pin_config.sclk = seed::D7;   // Physical 8
        spi_conf.pin_config.mosi = seed::D10;  // Physical 11
        spi_conf.pin_config.miso = Pin();      // Corrected: Universal "No Pin"
        spi_conf.pin_config.nss  = Pin();      // Corrected: Universal "No Pin"

        spi.Init(spi_conf);
    }

    void SendCommand(uint8_t cmd) {
        pin_dc.Write(false);
        pin_cs.Write(false);
        spi.BlockingTransmit(&cmd, 1);
        pin_cs.Write(true);
    }

    void SendData(uint8_t data) {
        pin_dc.Write(true);
        pin_cs.Write(false);
        spi.BlockingTransmit(&data, 1);
        pin_cs.Write(true);
    }
};

#endif