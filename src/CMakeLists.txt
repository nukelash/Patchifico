set(ALL_SRC 
    main.cpp
    ma_interface.cpp
)

set(INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/dep
    ${CMAKE_SOURCE_DIR}/dep/raylib/include
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
               

add_executable(virtual-modular-app ${ALL_SRC} ${DaisySP_SRC})

include_directories(virtual-modular-app ${INCLUDE_DIRS} ${DaisySP_INCLUDE_DIRS})

target_link_libraries(virtual-modular-app PUBLIC ${CMAKE_SOURCE_DIR}/dep/raylib/lib/libraylib.a)

if(APPLE)
    set(FRAMEWORKS "-framework CoreVideo"
                   "-framework IOKit" 
                   "-framework Cocoa"
                   "-framework GLUT" 
                   "-framework OpenGL")
    target_link_libraries(virtual-modular-app PUBLIC ${FRAMEWORKS})
endif()