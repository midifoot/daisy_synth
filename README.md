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
