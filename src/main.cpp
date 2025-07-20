#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "ma_interface.h"

#define RAYGUI_IMPLEMENTATION

#include "daisysp.h"

#include "modules.h"


// static daisysp::OnePole flt;
// static daisysp::Oscillator osc, lfo;
// float saw, freq, output;

oscillator my_osc;
filter my_filt;
mixer my_mixer;
patch_manager my_patch_bay;

void callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    float* input = (float*) pInput;
    float* out = (float*) pOutput;

    //access userdata in the callback:
    pDevice->pUserData;

    float sample;

    for (int i = 0; i < frameCount; i++) {
        // freq = lfo.Process();
        // saw = osc.Process();

        // flt.SetFrequency(freq+0.11);
        // output = flt.Process(saw);

        // //std::cout << output << std::endl;


        // sample = my_osc.process();
        // sample = my_filt.process(sample);
        // out[i] = my_mixer.process(sample, 0.0f);

        my_osc.process();
        my_filt.process();
        out[i] = my_mixer.process();

        //std::cout << sample << std::endl;

        // out[i] = my_osc.process();
    }
    

}

void gui_loop() {
    InitWindow(1000, 400, "Virtual Modular Synth");
    SetTargetFPS(60);

    bool showMessageBox = false;
    float value;

    while (!WindowShouldClose())
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            my_osc.draw();
            my_filt.draw();
            my_mixer.draw();

        EndDrawing();
    }

    CloseWindow();
}

int main() {
    int user_data;
    // osc.Init(48000);
    // lfo.Init(48000);
    // flt.Init();

    // osc.SetWaveform(daisysp::Oscillator::WAVE_SAW);
    // osc.SetFreq(220);
    // lfo.SetFreq(1);
    // lfo.SetAmp(0.1);

    my_osc.init(48000, &my_patch_bay);
    my_filt.init(48000, &my_patch_bay);
    my_mixer.init();

    my_patch_bay.add("my_osc_out", &my_osc._out);
    my_patch_bay.add("my_filt_in", &my_filt._in);
    my_patch_bay.add("my_filt_out", &my_filt._out);
    my_patch_bay.add("my_mixer_in", &my_mixer._in_1);

    my_patch_bay.connect("my_osc_out", "my_filt_in");
    my_patch_bay.connect("my_filt_out", "my_mixer_in");

    ma_interface* ma = new ma_interface(&user_data, callback);
    ma->start();

    gui_loop();
}