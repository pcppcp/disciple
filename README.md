# disciple
A software synth done right, built with teensy board, FreeRTOS and fixed point arithmetics.

WIP - It unfortunately still uses some arduino libraries, but I'll do my best to get rid of them.

You are not supposed to compile this atm, but you can use the code as a reference to things that are not so easy to do with Teensy core library.

## Features
- taking full advantage of hardware available: DAC is using its buffer, buffer is fed with DMA transfers. CPU has better things to do.
- encoders use interrupts and lookup table for state read & debouncing, as it should be. No pin polling.
- fixed point and optimized DSP arithmetics. Real men do not use floats.
- FreeRTOS to keep code structured and nice.

## Hardware used 
- teensy 3.2
- two encoders (EC21)
- button
- ili9341 controlled TFT 320x240 display

## Credits
- PJRC Teensy & Teensy Audio Lib
- CMake build for arduino
- the internet
