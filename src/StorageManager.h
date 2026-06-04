#pragma once
#include "HardwareManager.h"
#include "StateManager.h"
#include "fatfs.h"
#include <stdio.h>

class StorageManager {
public:
    daisy::SdmmcHandler sdmmc; 
    daisy::FatFSInterface fsi;

void Init(StateManager& state) {
        daisy::FatFSInterface::Config fsi_cfg;
        fsi_cfg.media = daisy::FatFSInterface::Config::MEDIA_SD;
        fsi.Init(fsi_cfg);

        daisy::SdmmcHandler::Config sd_cfg;
        sd_cfg.Defaults();
        sd_cfg.speed = daisy::SdmmcHandler::Speed::SLOW;      
        sd_cfg.width = daisy::SdmmcHandler::BusWidth::BITS_1; // Keep 1-bit for safety
        sdmmc.Init(sd_cfg);

        daisy::System::Delay(200);

        FRESULT fres = f_mount(&fsi.GetSDFileSystem(), "/", 1); 

        if (fres == FR_OK) {
            sprintf(state.sysMsg, "STATUS: SD MOUNTED OK");

            // --- NEW: CALCULATE SD CARD PERCENTAGE ---
            DWORD fre_clust;
            FATFS* fs;
            if (f_getfree("/", &fre_clust, &fs) == FR_OK) {
                // Math to calculate percentage used
                DWORD tot_sect = (fs->n_fatent - 2) * fs->csize;
                DWORD fre_sect = fre_clust * fs->csize;
                DWORD used_sect = tot_sect - fre_sect;
                
                // Multiply by 100 first to avoid float math, then divide
                state.sdSpaceUsed = (used_sect * 100) / tot_sect;
            } else {
                state.sdSpaceUsed = 0; // Error fallback
            }

        } else {
            sprintf(state.sysMsg, "STATUS: SD FAIL %d", fres);
            state.sdSpaceUsed = 0; 
        }
        
        state.needsDisplayUpdate = true;
    }
};