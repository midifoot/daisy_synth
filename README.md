(see also the TOTO.txt inside main workDIR)

# TODO 

- check why SD card is not working 

everyday roadmap : 
- make a consistant check routine inside system/settings (really useful ? ) 

- make the models list and detail the MasterMixerModel as a meta model, combining 2 to 4 other models based on the weight of each one ... make a score (+5,+3,+2 for each model to know what resource consuming to expect)
(ok incorporated in the MASTER_CODEX but not yet implemented)

### MAIN ROADMAP FOR NEXT WEEK : 

The Master Roadmap Confirmation
Here is how we will execute your plan when you get back:

1. The Final Hardware Boss (MIDI Input)
You will wire the voltage divider from the Teensy 2.0 TX split directly into the Daisy D14.
We will add the code to read those incoming MIDI bytes and trigger that simple sine wave we built. This proves the hardware is 100% finished.
(DONE)

2. The System & Storage Backbone
We will build out the System Menu so it actually does things (e.g., changing screen brightness, selecting the MIDI channel).
We will finalize the StorageManager so it can actually save and load the StateManager variables to the SD card.
(PARTIAL as SD CARD still unreliable)

3. The Phrase Engine (Sequencer)
We will build the logic that reads your arrays of notes (the phrases) and plays them back in time with the MIDI clock or internal BPM.
We will ensure the UI smoothly switches between the phrases.

4. The DSP Playground (The Fun Part)

Once steps 1-3 are rock solid, we will finally open the floodgates on SynthEngine.h and start building the multi-oscillator architecture, filters, and effects, linking them to your perfectly functioning UI menus.

# Reminders 

to compile : 
- indicate in the makefile the right .cpp to compile (with proper links ...)
- extract errors with : make 2> errors.txt or make 2>&1 | tee build_log.txt
- go into dfu mode BOOT+RESET(while keeping boot) 
- upload with : make program-dfu (or use the automation bash script )


# daisy_synth 
project involving electroSmith Daisy Seed Board as a fully implemented Synth for MFKB

first step : 
- make specs for the synth engine & periphs (display & Sd card + 3+3 encoders / buttons)  
(done, see relevant doc)
- make a test run for daisy workflow (ard IDE then full C++) 
(done)
- make test for separate periphs 
(done , except for sd card )
- test MFKB midi serial receive (keep USB/DIN5 outs)
(done)

second step :
- build main synth environment / UI  
(done)
- test first synth engines 
(done in boot sound (no envelopes))

third step : 
- implement full synth engines 
- create patches for every synth engine (env 20 each) 

fourth :
- test and debug , add the companion app for file management, advertize & sell 


----- OTHER infos / notes to remind 

## config and start project : 
1. setting up the workflow 
- installed vscodium
- created and configured github repo  
- configured "continue" extension with gemini Flash 3 API + MCP server 
- created basic file arbo (Makefile , /src , /tests)
- created basic hardware.h and main.cpp to test the workflow and compilation (OK) 
- displace to "tests" dir and make the single componenet tests (change the makefile ... )
(done)

--- infos on OLED font sizes 

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