# Project Name
TARGET = MDS_Synth

# Sources - We point to the file in the src folder
CPP_SOURCES = src/main.cpp 

# Library Locations (ADJUST THESE TO YOUR ACTUAL PATHS)
# Example: if DaisyExamples is next to your project, use ../libDaisy
LIBDAISY_DIR = ../DaisyDSPlibs/libDaisy
DAISYSP_DIR = ../DaisyDSPlibs/DaisySP

# Core location, leave this be
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile