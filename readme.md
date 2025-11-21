# Patchifico

Patchifico is a virtual modular synthesizer themed around Pacifico beer. The patchable interface is inspired by VCV Rack, and the synth engine (in particular the sequencer) is inspired by the Moog DFAM.

<img src="demo_screenshot.png"/>

## Dependencies
This project makes use of raylib and miniaudio, two absolutely wonderful libraries.

## Building instructions
So far, building this project has only been tested on MacOS using the following commands:
```
cmake -DCMAKE_BUILD_TYPE=DEBUG -S . -B build
cmake --build build
```