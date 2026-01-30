
if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s TOTAL_STACK=32MB")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()

if (EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPLATFORM_WEB -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 -s TOTAL_STACK=32MB --preload-file /Users/lukenash/Documents/Github/synth/resources --shell-file /Users/lukenash/Documents/Github/raylib/src/minshell.html -sEXPORTED_FUNCTIONS=[_ma_device__on_notification_unlocked]")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_WEB -g --preload-file /Users/lukenash/Documents/Github/synth/resources --shell-file /Users/lukenash/Documents/Github/raylib/src/minshell.html -sEXPORTED_FUNCTIONS=[_main,_ma_device__on_notification_unlocked] -s TOTAL_STACK=32MB")
  set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()

set(ALL_SRC 
    main.cpp
    ma_interface.cpp
)

set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/dep
    ${CMAKE_SOURCE_DIR}/src
)

set(DAISY_DIR ${CMAKE_SOURCE_DIR}/dep/DaisySP)

set(DaisySP_SRC
    ${DAISY_DIR}/Source/Control/adenv.cpp
    ${DAISY_DIR}/Source/Control/adsr.cpp
    ${DAISY_DIR}/Source/Control/phasor.cpp
    ${DAISY_DIR}/Source/Drums/analogbassdrum.cpp
    ${DAISY_DIR}/Source/Drums/analogsnaredrum.cpp
    ${DAISY_DIR}/Source/Drums/hihat.cpp
    ${DAISY_DIR}/Source/Drums/synthbassdrum.cpp
    ${DAISY_DIR}/Source/Drums/synthsnaredrum.cpp
    ${DAISY_DIR}/Source/Dynamics/crossfade.cpp
    ${DAISY_DIR}/Source/Dynamics/limiter.cpp
    ${DAISY_DIR}/Source/Effects/autowah.cpp
    ${DAISY_DIR}/Source/Effects/chorus.cpp
    ${DAISY_DIR}/Source/Effects/decimator.cpp
    ${DAISY_DIR}/Source/Effects/flanger.cpp
    ${DAISY_DIR}/Source/Effects/overdrive.cpp
    ${DAISY_DIR}/Source/Effects/phaser.cpp
    ${DAISY_DIR}/Source/Effects/sampleratereducer.cpp
    ${DAISY_DIR}/Source/Effects/tremolo.cpp
    ${DAISY_DIR}/Source/Effects/wavefolder.cpp
    ${DAISY_DIR}/Source/Filters/ladder.cpp
    ${DAISY_DIR}/Source/Filters/svf.cpp
    ${DAISY_DIR}/Source/Filters/soap.cpp
    ${DAISY_DIR}/Source/Noise/clockednoise.cpp
    ${DAISY_DIR}/Source/Noise/grainlet.cpp
    ${DAISY_DIR}/Source/Noise/particle.cpp
    ${DAISY_DIR}/Source/PhysicalModeling/drip.cpp
    ${DAISY_DIR}/Source/PhysicalModeling/modalvoice.cpp
    ${DAISY_DIR}/Source/PhysicalModeling/resonator.cpp
    ${DAISY_DIR}/Source/PhysicalModeling/KarplusString.cpp
    ${DAISY_DIR}/Source/PhysicalModeling/stringvoice.cpp
    ${DAISY_DIR}/Source/Sampling/granularplayer.cpp
    ${DAISY_DIR}/Source/Synthesis/fm2.cpp
    ${DAISY_DIR}/Source/Synthesis/formantosc.cpp
    ${DAISY_DIR}/Source/Synthesis/oscillator.cpp
    ${DAISY_DIR}/Source/Synthesis/oscillatorbank.cpp
    ${DAISY_DIR}/Source/Synthesis/variablesawosc.cpp
    ${DAISY_DIR}/Source/Synthesis/variableshapeosc.cpp
    ${DAISY_DIR}/Source/Synthesis/vosim.cpp
    ${DAISY_DIR}/Source/Synthesis/zoscillator.cpp
    ${DAISY_DIR}/Source/Utility/dcblock.cpp
    ${DAISY_DIR}/Source/Utility/metro.cpp
)

set(DaisySP_INCLUDE_DIRS 
    ${DAISY_DIR}/Source
    ${DAISY_DIR}/Source/Control
    ${DAISY_DIR}/Source/Drums
    ${DAISY_DIR}/Source/Dynamics
    ${DAISY_DIR}/Source/Effects
    ${DAISY_DIR}/Source/Filters
    ${DAISY_DIR}/Source/Noise
    ${DAISY_DIR}/Source/PhysicalModeling
    ${DAISY_DIR}/Source/Synthesis
    ${DAISY_DIR}/Source/Utility
)
               

add_executable(Patchifico ${ALL_SRC} ${DaisySP_SRC})

if(APPLE)
    set(FRAMEWORKS "-framework CoreVideo"
                   "-framework IOKit" 
                   "-framework Cocoa"
                   "-framework GLUT" 
                   "-framework OpenGL")
    target_link_libraries(Patchifico PUBLIC ${FRAMEWORKS})
endif()

if (EMSCRIPTEN)
    set(RAYLIB_INCLUDE ${CMAKE_SOURCE_DIR}/dep/raylib/wasm/include)
    set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/dep/raylib/wasm/lib/libraylib.a)
    target_link_options(Patchifico PUBLIC "--preload-file /Users/lukenash/Documents/Github/synth/resources --shell-file /Users/lukenash/Documents/Github/raylib/src/minshell.html" -sEXPORTED_RUNTIME_METHODS=[HEAPF32])
else ()
    set(RAYLIB_INCLUDE ${CMAKE_SOURCE_DIR}/dep/raylib/include)
    set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/dep/raylib/lib/libraylib.a)
endif()

include_directories(Patchifico ${INCLUDE_DIRS} ${DaisySP_INCLUDE_DIRS} ${RAYLIB_INCLUDE})
target_link_libraries(Patchifico PUBLIC ${RAYLIB_LIB})