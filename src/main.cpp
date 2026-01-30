#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "ma_interface.h"

#define RAYGUI_IMPLEMENTATION

#include "daisysp.h"

#include "modules.h"
#include "gui_components.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

oscillator my_osc;
lfo my_lfo;
filter my_filt;
mixer my_mixer;
envelope_generator my_envelope;
vca my_vca;
mult my_mult;
sequencer my_sequencer;
patch_manager my_patch_bay;
group* created_by_card;
help_button my_help_button;

ma_interface* ma;

Font title_font;
Texture2D logo;
Texture2D anchor;
Texture2D vine;

bool audio_initialized = false;

void callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    float* out = (float*) pOutput;

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

void initial_preset() {
    my_patch_bay.connect("my_osc_sqr", "vca_in_a_1", PACIFICO_RED);
    my_patch_bay.connect("sequencer_trig_out", "env_trigger", PACIFICO_BLUE);
    my_patch_bay.connect("env_out", "vca_in_a_2", PACIFICO_GREEN);
    my_patch_bay.connect("vca_out_a", "my_mixer_in_1", PACIFICO_RED);

    my_sequencer.set_trig(0, 1);
    my_sequencer.set_trig(5, 1);

    my_envelope.set_attack(0.001);
}

void update_draw_frame() {

#if defined(PLATFORM_WEB)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (!audio_initialized)) {
        ma->start();
        audio_initialized = true;
    }
#endif

    if(IsWindowResized()){
        float ratio = 680.0f / 497.5f;
        int width = GetScreenWidth() < 680 ? 680 : GetScreenWidth();
        int height = width / ratio;
        // int k_scale = EM_ASM_INT(return window.devicePixelRatio);

        SetWindowSize(width, height);
        BASE_UNIT = width / 680.0f;

#if defined(PLATFORM_WEB)
        // This makes the UI render at the correct resolution on web
        EM_ASM(
            const scale = window.devicePixelRatio;
            const canvas = document.getElementById('canvas');
            canvas.style.width = Math.floor(canvas.width / scale) + "px";
            canvas.style.height = Math.floor(canvas.height / scale) + "px";
            canvas.style["image-rendering"] = "pixelated";
        );
#endif
    }

    BeginDrawing();
        ClearBackground(PACIFICO_BROWN);

        my_osc.draw();
        my_lfo.draw();
        my_filt.draw();
        my_mixer.draw();
        my_envelope.draw();
        my_vca.draw();
        my_mult.draw();
        my_sequencer.draw();
        my_patch_bay.draw();

        DrawTextureEx(logo, (Vector2){5, 0}*BASE_UNIT, 0,  0.45*BASE_UNIT, WHITE);
        created_by_card->draw();
        DrawTextureEx(vine, (Vector2){457, 60}*BASE_UNIT, 0, 0.27*BASE_UNIT, WHITE);
        DrawTextureEx(anchor, (Vector2){350, -12}*BASE_UNIT, 0, 0.34*BASE_UNIT, WHITE);
        DrawTextEx(CERVEZA_FONT, "CREATED BY LUKE NASH", (Vector2){170, 7}*BASE_UNIT, CERVEZA_FONT_SIZE*BASE_UNIT, CERVEZA_FONT_SPACING*BASE_UNIT, PACIFICO_RED);
        my_help_button.draw();

    EndDrawing();
}

void gui_loop() {

    bool showMessageBox = false;
    float value;

    int x = 60;
    int y = 60;

    logo = LoadTexture("/Users/lukenash/Documents/Github/synth/resources/logo.png");
    anchor = LoadTexture("/Users/lukenash/Documents/Github/synth/resources/anchor.png");
    vine = LoadTexture("/Users/lukenash/Documents/Github/synth/resources/vine.png");
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    SetTargetFPS(60);

    ma->start();
    audio_initialized = true;

    while (!WindowShouldClose()) {   
        update_draw_frame();
    }
#endif

    CloseWindow();
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_TOPMOST);
    InitWindow(680, 497.5, "Patchifico");
    InitVisualConfig();
    srand(20);
    int user_data;

    my_osc.init(48000, &my_patch_bay);
    my_lfo.init(48000, &my_patch_bay);
    my_filt.init(48000, &my_patch_bay);
    my_envelope.init(48000);
    my_vca.init();
    my_mult.init();
    my_sequencer.init(48000, 0.5);
    my_mixer.init();
    created_by_card = new group({160, 5, 180, 15});

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

    ma = new ma_interface(&user_data, callback);

    my_help_button.init(ma); //need to init help button AFTER initializing ma

    initial_preset();

    gui_loop();
}