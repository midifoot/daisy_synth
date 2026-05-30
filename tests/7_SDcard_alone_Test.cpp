#include "daisy_seed.h"
#include "fatfs.h"
#include "dev/oled_ssd130x.h"
#include "hid/disp/oled_display.h" 
#include <stdio.h> 

using namespace daisy;

DaisySeed hw; 
using MyOledDriver = SSD130xI2c128x64Driver;
OledDisplay<MyOledDriver> display;

// SD Card and File System Variables
SdmmcHandler sd;
FatFSInterface fsi;
FIL SDFile;

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

    // Show a loading screen
    display.Fill(false);
    display.SetCursor(0, 20);
    display.WriteString("BOOTING SDMMC...", Font_7x10, true);
    display.Update();
    System::Delay(1000); // Wait 1 second so you can see it

    // ==========================================
    // 2. SDMMC INIT
    // ==========================================
// ==========================================
    // 2. SDMMC INIT
    // ==========================================
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults(); 
    
    // THE BREADBOARD FIX: Drop speed from 50MHz to 12.5MHz
    sd_cfg.speed = SdmmcHandler::Speed::MEDIUM_SLOW; 
    sd_cfg.width = SdmmcHandler::BusWidth::BITS_1;
    sd.Init(sd_cfg);

    FatFSInterface::Config fsi_cfg;
    fsi_cfg.media = FatFSInterface::Config::MEDIA_SD;
    fsi.Init(fsi_cfg);

    // Message Buffers
    char msgMount[30] = "MOUNT: FAILED";
    char msgFile[30]  = "FILE: NOT READ";
    char fileContent[30] = "";

    // ==========================================
    // 3. MOUNT & READ LOGIC
    // ==========================================
    
    // Attempt to Mount the SD Card volume
    if (f_mount(&fsi.GetSDFileSystem(), "/", 1) == FR_OK) {
        sprintf(msgMount, "MOUNT: SUCCESS!");
        
        // Attempt to open test.txt in READ mode
        if (f_open(&SDFile, "test.txt", FA_READ) == FR_OK) {
            UINT bytesRead;
            
            // Read up to 20 characters from the file
            f_read(&SDFile, fileContent, 20, &bytesRead); 
            fileContent[bytesRead] = '\0'; // Properly end the string
            
            f_close(&SDFile);
            sprintf(msgFile, "FILE: SUCCESS!");
        } else {
            sprintf(msgFile, "FILE: NOT FOUND");
        }
    } else {
        sprintf(msgMount, "MOUNT: FAILED");
    }

    // ==========================================
    // 4. DISPLAY FINAL RESULTS
    // ==========================================
    display.Fill(false);
    
    display.SetCursor(0, 0);
    display.WriteString("--- SDMMC DIAGNOSTIC ---", Font_5x8, true);
    
    // Print Mount Status
    display.SetCursor(0, 15);
    display.WriteString(msgMount, Font_7x10, true);
    
    // Print File Open Status
    display.SetCursor(0, 30);
    display.WriteString(msgFile, Font_5x8, true);

    // Print the actual text found inside the file
    display.SetCursor(0, 45);
    display.WriteString("TEXT: ", Font_5x8, true);
    display.SetCursor(35, 45);
    display.WriteString(fileContent, Font_5x8, true);

    display.Update();

    while(1) {
        // Just sit here forever so you can read the screen
        System::Delay(100);
    }
}