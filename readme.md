# Patchifico

Patchifico is a virtual modular synthesizer themed around Pacifico beer. The patchable interface is inspired by VCV Rack, and the synth engine (in particular the sequencer) is inspired by the Moog DFAM.

<img src="demo_screenshot.png"/>

## Dependencies
This project makes use of raylib for GUI, miniaudio for audio IO, and DaisySP for audio DSP,

## Building instructions
On MacOS:
```
cmake -DCMAKE_BUILD_TYPE=DEBUG -S . -B build
cmake --build build
```

For Web via Emscripten:

```
emcmake cmake -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_TOOLCHAIN_FILE=/Users/lukenash/Documents/Github/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -S . -B build

cmake --build build
```