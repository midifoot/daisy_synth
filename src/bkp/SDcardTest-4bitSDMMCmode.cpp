#include "HardwareManager.h"
#include "fatfs.h"
#include <string.h>
#include <stdio.h>

using namespace daisy; 

HardwareManager hw;

// Global Scope
SdmmcHandler sdmmc; 
FatFSInterface fsi;
FIL file;

int main(void) {
    hw.Init();
    hw.seed.DelayMs(200);

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("4-Bit SD Test...", Font_7x10, true);
    hw.display.Update();

    // 1. Initialize standard FatFS 
    FatFSInterface::Config fsi_cfg;
    fsi_cfg.media = FatFSInterface::Config::MEDIA_SD;
    fsi.Init(fsi_cfg);

    // 2. THE 4-BIT OVERRIDE
    // We hijack the hardware to use all 4 data lanes (DAT0-DAT3) by default,
    // but we keep the speed SLOW to survive the physical wires.
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd_cfg.speed = SdmmcHandler::Speed::SLOW;      
    sdmmc.Init(sd_cfg);

    hw.seed.DelayMs(200);

    // 3. Attempt the Mount
    FRESULT fres = f_mount(&fsi.GetSDFileSystem(), "/", 1); 

    hw.display.SetCursor(0, 15);
    if (fres == FR_OK) {
        hw.display.WriteString("MOUNT OK!", Font_7x10, true);
        
        fres = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
        hw.display.SetCursor(0, 30);
        
        if (fres == FR_OK) {
            UINT bytesWritten;
            const char* msg = "Hello from full 4-Bit SDMMC!\n";
            f_write(&file, msg, strlen(msg), &bytesWritten);
            f_close(&file);
            hw.display.WriteString("WRITE OK!", Font_7x10, true);
        }
    } else {
        char err[16];
        sprintf(err, "FAIL CODE: %d", fres);
        hw.display.WriteString(err, Font_7x10, true);
    }

    hw.display.Update();

    while(1) { hw.seed.DelayMs(100); }
}