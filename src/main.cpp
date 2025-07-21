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

        my_osc.process();
        my_filt.process();
        out[i] = my_mixer.process();

    }
    

}

void gui_loop() {
    InitWindow(1000, 400, "Virtual Modular Synth");
    SetTargetFPS(60);

    bool showMessageBox = false;
    float value;

    int x = 60;
    int y = 60;

    while (!WindowShouldClose())
    {
        // Draw
        //----------------------------------------------------------------------------------

        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            my_osc.draw();
            my_filt.draw();
            my_mixer.draw();
            my_patch_bay.draw();

        EndDrawing();
    }

    CloseWindow();
}

int main() {
    int user_data;

    my_osc.init(48000, &my_patch_bay);
    my_filt.init(48000, &my_patch_bay);
    my_mixer.init();

    my_patch_bay.add("my_osc_tri", &my_osc._audio_tri_out);
    my_patch_bay.add("my_osc_saw", &my_osc._audio_saw_out);
    my_patch_bay.add("my_osc_sqr", &my_osc._audio_sqr_out);
    my_patch_bay.add("my_osc_freq", &my_osc._audio_frequency_mod);

    my_patch_bay.add("my_osc_lfo_tri", &my_osc._lfo_tri_out);
    my_patch_bay.add("my_osc_lfo_saw", &my_osc._lfo_saw_out);
    my_patch_bay.add("my_osc_lfo_sqr", &my_osc._lfo_sqr_out);

    my_patch_bay.add("my_filt_in", &my_filt._in);
    my_patch_bay.add("my_filt_out", &my_filt._out);
    my_patch_bay.add("my_mixer_in_1", &my_mixer._in_1);
    my_patch_bay.add("my_mixer_in_2", &my_mixer._in_2);

    //my_patch_bay.connect("my_osc_out", "my_filt_in");
    //my_patch_bay.connect("my_filt_out", "my_mixer_in");

    ma_interface* ma = new ma_interface(&user_data, callback);
    ma->start();

    gui_loop();
}