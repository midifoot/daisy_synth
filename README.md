(see also the TOTO.txt inside main workDIR)

# TODO 

- wire the two encoders like the first, test with pushes 
- adapt the rgb led to find pins and test 
- test the sd card reading/writing 

# Reminders 

to compile : 
- indicate in the makefile the right .cpp to compile (with proper links ...)
- extract errors with : make 2> errors.txt or make 2>&1 | tee build_log.txt
- go into dfu mode BOOT+RESET(while keeping boot) 
- upload with : make program-dfu


# daisy_synth 
project involving electroSmith Daisy Seed Board as a fully implemented Synth for MFKB

first step : 
- make specs for the synth engine & periphs (display & Sd card + 3+3 encoders / buttons)  
- make a test run for daisy workflow (ard IDE then full C++) 
- make test for separate periphs 
- test MFKB midi serial receive (keep USB/DIN5 outs)

second step :
- build main synth environment / UI  
- test first synth engines 

third step : 
- implement full synth engines 
- create patches for every synth engine (env 20 each) 

fourth :
- test and run 


## config and start project : 
1. setting up the workflow 
- installed vscodium
- created and configured github repo  
- configured "continue" extension with gemini Flash 3 API + MCP server 
- created basic file arbo (Makefile , /src , /tests)
- created basic hardware.h and main.cpp to test the workflow and compilation (OK) 


sizing on the display : 
oled are 128 x 64 pixels (0-127 / 0-63)

font sizes : 
if you use Font_11x18 for the (middle) text.
11 is the width of each character in pixels.
18 is the height of the character in pixels.
Available fonts : 
Font_4x6
Font_4x8
Font_5x8
Font_6x7
Font_6x8
Font_7x10
Font_11x18
Font_16x26