#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "ma_interface.h"

#define RAYGUI_IMPLEMENTATION

#include "daisysp.h"

#include "modules.h"
#include "gui_components.h"


// static daisysp::OnePole flt;
// static daisysp::Oscillator osc, lfo;
// float saw, freq, output;

oscillator my_osc;
lfo my_lfo;
filter my_filt;
mixer my_mixer;
envelope_generator my_envelope;
vca my_vca;
mult my_mult;
sequencer my_sequencer;
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
        my_lfo.process();
        my_filt.process();
        my_envelope.process();
        my_vca.process();
        my_mult.process();
        my_sequencer.process();
        out[i] = my_mixer.process();

    }
    

}

void gui_loop() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(720, 420, "Virtual Modular Synth");
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
            my_lfo.draw();
            my_filt.draw();
            my_mixer.draw();
            my_envelope.draw();
            my_vca.draw();
            my_mult.draw();
            my_sequencer.draw();
            my_patch_bay.draw();

        EndDrawing();
    }

    CloseWindow();
}

int main() {
    int user_data;

    my_osc.init(48000, &my_patch_bay);
    my_lfo.init(48000, &my_patch_bay);
    my_filt.init(48000, &my_patch_bay);
    my_envelope.init(48000);
    my_vca.init();
    my_mult.init();
    my_sequencer.init(48000, 90);
    my_mixer.init();

    my_patch_bay.add("my_osc_tri", &my_osc._audio_tri_out);
    my_patch_bay.add("my_osc_saw", &my_osc._audio_saw_out);
    my_patch_bay.add("my_osc_sqr", &my_osc._audio_sqr_out);
    my_patch_bay.add("my_osc_freq", &my_osc._audio_frequency_mod);
    my_patch_bay.add("my_osc_pw", &my_osc._pulse_width);

    my_patch_bay.add("my_osc_lfo_tri", &my_lfo._lfo_tri_out);
    my_patch_bay.add("my_osc_lfo_saw", &my_lfo._lfo_saw_out);
    my_patch_bay.add("my_osc_lfo_sqr", &my_lfo._lfo_sqr_out);
    my_patch_bay.add("my_osc_lfo_pw", &my_lfo._pulse_width);
    my_patch_bay.add("my_osc_lfo_reset", &my_lfo._retrig);

    my_patch_bay.add("env_trigger", &my_envelope._trigger);
    my_patch_bay.add("env_out", &my_envelope._output);

    my_patch_bay.add("vca_in_a_1", &my_vca._in_a1);
    my_patch_bay.add("vca_in_a_2", &my_vca._in_a2);
    my_patch_bay.add("vca_out_a", &my_vca._out_a1);
    my_patch_bay.add("vca_in_b_1", &my_vca._in_b1);
    my_patch_bay.add("vca_in_b_2", &my_vca._in_b2);
    my_patch_bay.add("vca_out_b", &my_vca._out_b1);

    my_patch_bay.add("mult_in", &my_mult._in);
    my_patch_bay.add("mult_out1", &my_mult._out1);
    my_patch_bay.add("mult_out2", &my_mult._out2);
    my_patch_bay.add("mult_out3", &my_mult._out3);

    my_patch_bay.add("sequencer_trig_out", &my_sequencer._trig);
    my_patch_bay.add("sequencer_cv_out", &my_sequencer._cv);

    my_patch_bay.add("my_filt_in", &my_filt._in);
    my_patch_bay.add("my_filt_out", &my_filt._out);
    my_patch_bay.add("my_filt_cutoff", &my_filt._cutoff_mod);

    my_patch_bay.add("my_mixer_in_1", &my_mixer._in_1);
    my_patch_bay.add("my_mixer_in_2", &my_mixer._in_2);

    //my_patch_bay.connect("my_osc_out", "my_filt_in");
    //my_patch_bay.connect("my_filt_out", "my_mixer_in");

    ma_interface* ma = new ma_interface(&user_data, callback);
    ma->start();

    gui_loop();
}