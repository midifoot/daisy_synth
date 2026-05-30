# Project Name
TARGET = MDS_GFX_Test

# Locations of libraries relative to this folder (daisy_synth)
LIBDAISY_DIR = ../DaisyDSPlibs/libDaisy
DAISYSP_DIR = ../DaisyDSPlibs/DaisySP

# ---> USED FOR FS <---
USE_FATFS = 1

# Sources - Project files
# CPP_SOURCES = tests/oled_display_test.cpp # testing the oled display 
# CPP_SOURCES = tests/sd_spi_test.cpp
CPP_SOURCES = tests/7_SDcard_alone_Test.cpp

# Core location and generic libDaisy Makefile
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile