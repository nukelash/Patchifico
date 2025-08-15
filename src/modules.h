
//modules: oscillator, lfo, vca, env, filter, sequencer(?)

#include "daisysp.h"
#include "raygui.h"
#include "raylib.h"

#include "gui_components.h"

#include <unordered_map>

class patch_point_gui {
public:
    patch_point_gui() {}
    ~patch_point_gui() {}

    void init(std::string label, Vector2 position) {

        // float width = 40;
        // float height = 100;

        // float x = position.x + (0.5*width);
        // float y = position.y + (0.7*height);
        _radius = 15;
        _circle_position = {position.x+_radius, position.y+_radius};
        

        // y = position.y + (0.3*height);
        _label_position = {position.x, position.y-10};
        _label = label; 
    }

    void draw() {
        DrawCircleV(_circle_position, _radius, RED);
        DrawCircleV(_circle_position, _radius-3, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        DrawText(_label.c_str(), _label_position.x, _label_position.y, 11, BLACK);
    }

    std::string _label;
    Vector2 _label_position;
    Vector2 _circle_position;
    float _radius;
};

struct patch_source {
    float value;
    patch_point_gui gui;
};

struct patch_destination {
    patch_source* source = nullptr;
    bool connected = false;
    patch_point_gui gui;

    float val() {
        if (connected) {
            return source->value;
        }
        else {
            return 0.0f;
        }
    }
};

class patch_manager {
public:
    patch_manager() {}
    ~patch_manager() {}

    struct connection {
        std::string _src_name;
        Vector2 _src_coordinates;
        std::string _dest_name;
        Vector2 _dest_coordinates;

        void draw() {
            DrawLineEx(_src_coordinates, _dest_coordinates, 4, RED);
        }
    };

    void add(std::string name, patch_source* source) {
        _sources.insert({name, source});
    }

    void add(std::string name, patch_destination* dest) {
        _destinations.insert({name, dest});
    }
    
    void draw() {

        Vector2 mouse_location = GetMousePosition();
        std::string name;

        if ((!_connection_in_progress) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (point_colliding_patch_source(mouse_location, &name)) {

                // if this name exists i.e the patch point is in use
                for (auto i : _connections) {
                    auto idx = i.first;
                    if (_connections[idx]->_src_name == name) {
                        _connection_in_progress = true;
                        
                        connection* c = new connection();
                        c->_dest_name = _connections[idx]->_dest_name;
                        c->_dest_coordinates = _connections[idx]->_dest_coordinates;
                        c->_src_name = "mouse";
                        c->_src_coordinates = mouse_location;
                        _connections.insert({"mouse", c});
                        disconnect(_connections[idx]->_dest_name);
                        _connections.erase(idx);
                        //return;
                    }
                }


                _connection_in_progress = true;
                connection* c = new connection();
                c->_src_name = name;
                c->_src_coordinates = _sources[name]->gui._circle_position;
                c->_dest_name = "mouse";
                c->_dest_coordinates = mouse_location;
                _connections.insert({"mouse", c});
            }
            else if (point_colliding_patch_destination(mouse_location, &name)) {
                for (auto i : _connections) {
                    auto idx = i.first;
                    if (_connections[idx]->_dest_name == name) {
                        _connection_in_progress = true;
                        
                        connection* c = new connection();
                        c->_src_name = _connections[idx]->_src_name;
                        c->_src_coordinates = _connections[idx]->_src_coordinates;
                        c->_dest_name = "mouse";
                        c->_dest_coordinates = mouse_location;
                        _connections.insert({"mouse", c});
                        disconnect(_connections[idx]->_dest_name);
                        _connections.erase(idx);
                        //return; // for some reason commenting out this return prevents stuttering... there may be something backwards when the next connection gets made though?
                    }
                }
                
                _connection_in_progress = true;
                connection* c = new connection();
                c->_src_name = "mouse";
                c->_src_coordinates = mouse_location;
                c->_dest_name = name;
                c->_dest_coordinates = _destinations[name]->gui._circle_position;
                _connections.insert({"mouse", c});
            }
        }
        else if (_connection_in_progress) {
            if (_connections["mouse"]->_dest_name == "mouse") {
                _connections["mouse"]->_dest_coordinates = mouse_location;
            }
            else if (_connections["mouse"]->_src_name == "mouse") {
                _connections["mouse"]->_src_coordinates = mouse_location;
            }
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (point_colliding_patch_source(mouse_location, &name) && (_connections["mouse"]->_src_name == "mouse")) {
                    connect(name, _connections["mouse"]->_dest_name);
                    _connections.erase("mouse");
                    _connection_in_progress = false;

                }
                else if (point_colliding_patch_destination(mouse_location, &name) && (_connections["mouse"]->_dest_name == "mouse")) {
                    connect(_connections["mouse"]->_src_name, name);
                    _connections.erase("mouse");
                    _connection_in_progress = false;
                }
                else {
                    _connection_in_progress = false;
                    _connections.erase("mouse");
                }
            }
        }

        for(auto i : _connections) {
            auto cur = i.first;
            _connections[cur]->draw();
        }
    }

    void connect(std::string source_name, std::string dest_name) {
        patch_destination* dest = _destinations[dest_name];

        if (dest->connected) {
            disconnect(dest_name);
        }

        dest->source = _sources[source_name];

        dest->connected = true;

        connection* c = new connection();
        c->_src_name = source_name;
        c->_src_coordinates = _sources[source_name]->gui._circle_position;
        c->_dest_name = dest_name;
        c->_dest_coordinates = _destinations[dest_name]->gui._circle_position;
        _connections.insert({dest_name, c});
    }

    void disconnect(std::string dest_name) {
        _destinations[dest_name]->source = nullptr;
        _destinations[dest_name]->connected = false;
        //need to delete pointer i.e. memory leak
        _connections.erase(dest_name);
    }

private:

    bool point_colliding_patch_source(Vector2 point, std::string* name) {
        for(auto i : _sources) {
            auto idx = i.first;
            if (CheckCollisionPointCircle(point, _sources[idx]->gui._circle_position, _sources[idx]->gui._radius)) {
                *name = idx;
                return true;
            }
        }
        return false;
    }

    bool point_colliding_patch_destination(Vector2 point, std::string* name) {
        for(auto i : _destinations) {
            auto idx = i.first;
            if (CheckCollisionPointCircle(point, _destinations[idx]->gui._circle_position, _destinations[idx]->gui._radius)) {
                *name = idx;
                return true;
            }
        }
        return false;
    }

    std::unordered_map<std::string, patch_source*> _sources;
    std::unordered_map<std::string, patch_destination*> _destinations;

    std::unordered_map<std::string, connection*> _connections;

    bool _connection_in_progress = false;
};

class oscillator {
public:
    oscillator() {}
    ~oscillator() {}

    void init(float sample_rate, patch_manager* patch_bay) {
        _patch_bay = patch_bay;

        _audio_frequency = 100;
        for (auto i : _audio_oscillators) {
            i->Init(sample_rate);
            i->SetFreq(_audio_frequency);
        }
        _audio_tri.SetWaveform(daisysp::Oscillator::WAVE_TRI);
        _audio_saw.SetWaveform(daisysp::Oscillator::WAVE_SAW);
        _audio_sqr.SetWaveform(daisysp::Oscillator::WAVE_SQUARE);

        _module_box.x = 30;
        _module_box.y = 45;
        _module_box.width = 255;
        _module_box.height = 135;

        _audio_frequency_mod.gui.init("Freq Mod", {_module_box.x+82.5f, _module_box.y+82.5f});

        _audio_tri_out.gui.init("Tri", {_module_box.x+202.5f, _module_box.y+22.5f});
        _audio_saw_out.gui.init("Saw", {_module_box.x+202.5f, _module_box.y+82.5f});
        _audio_sqr_out.gui.init("Sqr", {_module_box.x+142.5f, _module_box.y+82.5f});

        float freq_radius = 30;
        _frequency_knob = new knob({_module_box.x + freq_radius + 22.5f, _module_box.y + freq_radius + 22.5f},freq_radius, &_audio_frequency, 20, 500);
    }

    void draw() {
        
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        GuiGroupBox(_module_box, "VCO");

        //GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Frequency", "", &_audio_frequency, 20, 500);
        _frequency_knob->draw();
        y_index += 160;

        _audio_frequency_mod.gui.draw();

        for (auto i : _audio_outs) {
            i->gui.draw();
        }

        

        _frequency_knob->draw();
    }

    float process() {

        for (auto i : _audio_oscillators) {
            i->SetFreq(_audio_frequency + (100*_audio_frequency_mod.val()));
        }

        for (int i = 0; i < _audio_oscillators.size(); i++) {
            _audio_outs[i]->value = _audio_oscillators[i]->Process();
        }

    }

    // process calls the daisy process function, THEN sets patch output variables accordingly;

    //callback will call each module process one at a time

    //draw will draw the GUI AND update module params accordingly
    patch_source _audio_tri_out;
    patch_source _audio_saw_out;
    patch_source _audio_sqr_out;
    std::vector<patch_source*> _audio_outs = {&_audio_tri_out, &_audio_saw_out, &_audio_sqr_out};

    patch_destination _audio_frequency_mod;

private:

    daisysp::Oscillator _audio_tri;
    daisysp::Oscillator _audio_saw;
    daisysp::Oscillator _audio_sqr;
    std::vector<daisysp::Oscillator*> _audio_oscillators = {&_audio_tri, &_audio_saw, &_audio_sqr};
    float _audio_frequency;
    //int _wavetype;

    

    //int _out_counter;
    

    //patch_destinations* _patch_destinations;

    patch_manager* _patch_bay;

    Rectangle _module_box;

    knob* _frequency_knob;
};

class lfo {
public:
    lfo() {}
    ~lfo() {}

    void init(float sample_rate, patch_manager* patch_bay) {
        _patch_bay = patch_bay;

        _module_box = {30, 187.5, 180, 112.5};

        _lfo_frequency = 0.5;
        for (auto i : _lfo_oscillators) {
            i->Init(sample_rate);
            i->SetFreq(_lfo_frequency);
        }
        _lfo_tri.SetWaveform(daisysp::Oscillator::WAVE_TRI);
        _lfo_saw.SetWaveform(daisysp::Oscillator::WAVE_SAW);
        _lfo_sqr.SetWaveform(daisysp::Oscillator::WAVE_SQUARE);

        _lfo_tri_out.gui.init("Tri", {_module_box.x+135, _module_box.y+15});
        _lfo_saw_out.gui.init("Saw", {_module_box.x+135, _module_box.y+67.f});
        _lfo_sqr_out.gui.init("Sqr", {_module_box.x+75, _module_box.y+67.5f});

        float radius = 22.5f;
        _frequency_knob  = new knob({_module_box.x+67.5f+radius, _module_box.y+7.5f+radius}, radius, &_lfo_frequency, 0.01, 50);
    }

    void draw() {

        GuiGroupBox(_module_box, "LFO");

        _frequency_knob->draw();

        for (auto i : _lfo_outs) {
            i->gui.draw();
        }
    }

    float process() {
        for (auto i : _lfo_oscillators) {
            i->SetFreq(_lfo_frequency);
        }

        for (int i = 0; i < _lfo_oscillators.size(); i++) {
            _lfo_outs[i]->value = _lfo_oscillators[i]->Process();
        }
    }

    patch_source _lfo_tri_out;
    patch_source _lfo_saw_out;
    patch_source _lfo_sqr_out;
    std::vector<patch_source*> _lfo_outs = {&_lfo_tri_out, &_lfo_saw_out, &_lfo_sqr_out};

private:

    daisysp::Oscillator _lfo_tri;
    daisysp::Oscillator _lfo_saw;
    daisysp::Oscillator _lfo_sqr;
    std::vector<daisysp::Oscillator*> _lfo_oscillators = {&_lfo_tri, &_lfo_saw, &_lfo_sqr};
    float _lfo_frequency;

    patch_manager* _patch_bay;

    Rectangle _module_box;

    knob* _frequency_knob;
};

class filter {
public:
    filter() {}
    ~filter() {}

    void init(float sample_rate, patch_manager* patch_bay){
        _patch_bay = patch_bay;

        _filter.Init(sample_rate);
        _frequency = 1000;
        _resonance = 0.5;
        _filter.SetFreq(_frequency);

        _module_box = {292.5f, 45, 195, 135};

        _in.gui.init("input", {_module_box.x+22.5f, _module_box.y+82.5f});
        _out.gui.init("output", {_module_box.x+142.5f, _module_box.y+82.5f});

        float radius = 22.5f;
        _cutoff_knob = new knob({_module_box.x+45+radius, _module_box.y+22.5f+radius}, radius, &_frequency, 20, 15000);

        _resonance_knob = new knob({_module_box.x+105+radius, _module_box.y+22.5f+radius}, radius, &_resonance, 0.0, 1.5f);

    }

    float process() {

        _filter.SetFreq(_frequency);
        _filter.SetRes(_resonance);

        _filter.Process(_in.val());

        _out.value = _filter.Low();
    }

    void draw() {
        
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        GuiGroupBox(_module_box, "VCF");

        // if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Frequency", "", &_frequency, 20, 15000)) {
        //     _filter.SetFreq(_frequency);
        // }
        y_index += y_pad;

        // if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Resonance", "", &_resonance, 0.0, 2.0)) {
        //     _filter.SetRes(_resonance);
        // }
        y_index += y_pad;

        _in.gui.draw();
        _out.gui.draw();
        _cutoff_knob->draw();
        _resonance_knob->draw();
    }

    patch_destination _in;
    patch_source _out;

private:
    daisysp::Svf _filter;
    float _frequency;
    float _resonance;

    patch_manager* _patch_bay;

    Rectangle _module_box;

    knob* _cutoff_knob;
    knob* _resonance_knob;
};

class envelope_generator {
public:
    envelope_generator() {}
    ~envelope_generator() {}

    void init(float sample_rate) {
        _env.Init(sample_rate);

        _module_box = {330,187.5, 157.5, 112.5};

        _attack = 0.1;
        _decay = 0.5;
        _last_trig_value = 0.0;

        _trigger.gui.init("trigger", {_module_box.x+15, _module_box.y+67.5f});
        _output.gui.init("env out", {_module_box.x+112.5f, _module_box.y+41.25f});

        float radius = 22.5f;
        _attack_knob = new knob({_module_box.x+60+radius, _module_box.y+7.5f+radius}, radius, &_attack, 0.01, 1.0);
        _decay_knob = new knob({_module_box.x+60+radius, _module_box.y+60+radius}, radius, &_decay, 0.01, 2.0);
    }

    void process() {

        _env.SetTime(daisysp::ADENV_SEG_ATTACK, _attack);
        _env.SetTime(daisysp::ADENV_SEG_DECAY, _decay);

        if(_trigger.val() > _last_trig_value) {
            _env.Trigger();
        }
        _last_trig_value = _trigger.val();
        _output.value = _env.Process();
    }

    void draw() {

        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        GuiGroupBox(_module_box, "Envelope");

        // if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Attack", "", &_attack, 0.01, 2.0)) {
        //     _env.SetTime(daisysp::ADENV_SEG_ATTACK, _attack);
        // }
        y_index += y_pad;

        // if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Decay", "", &_decay, 0.01, 2.0)) {
        //     _env.SetTime(daisysp::ADENV_SEG_DECAY, _decay);
        // }
        y_index += y_pad;

        if (GuiButton((Rectangle){_module_box.x+15, _module_box.y+15, 30, 30}, "Trigger")) {
            _env.Trigger();
        }

        _trigger.gui.draw();
        _output.gui.draw();
        _attack_knob->draw();
        _decay_knob->draw();
    }

    patch_destination _trigger;
    patch_source _output;

private:
    daisysp::AdEnv _env;
    float _attack;
    float _decay;

    float _last_trig_value;

    Rectangle _module_box;

    knob* _attack_knob;
    knob* _decay_knob;
};

class mult {
public:
    mult() {}
    ~mult(){}

    void init() {
        _module_box = {217.5, 187.5, 105, 112.5};

        _in.gui.init("in", {_module_box.x+15, _module_box.y+15});
        _out1.gui.init("out", {_module_box.x+60, _module_box.y+15});
        _out2.gui.init("out", {_module_box.x+15, _module_box.y+67.5f});
        _out3.gui.init("out", {_module_box.x+60, _module_box.y+67.5f});
    }

    void draw() {

        GuiGroupBox(_module_box, "Mult");

        _in.gui.draw();
        _out1.gui.draw();
        _out2.gui.draw();
        _out3.gui.draw();
    }

    void process() {
        _out1.value = _in.val();
        _out2.value = _in.val();
        _out3.value = _in.val();
    }

    patch_destination _in;
    patch_source _out1;
    patch_source _out2;
    patch_source _out3;

private:
    Rectangle _module_box;
};

class vca {
public:
    vca() {}
    ~vca() {}

    void init() {
        _module_box = {495, 45, 97.5f, 255};
        _in_a1.gui.init("ina1", {_module_box.x+15, _module_box.y+22.5f});
        _in_a2.gui.init("ina2", {_module_box.x+15, _module_box.y+82.5f});
        _out_a1.gui.init("outa1", {_module_box.x+52.5f, _module_box.y+52.5f});

        _in_b1.gui.init("inb1", {_module_box.x+15, _module_box.y+142.5f});
        _in_b2.gui.init("inb2", {_module_box.x+15, _module_box.y+202.5f});
        _out_b1.gui.init("outb1", {_module_box.x+52.5f, _module_box.y+172.5f});
    }

    void process() {
        _out_a1.value = _in_a1.val() * _in_a2.val();
        _out_b1.value = _in_b1.val() * _in_b2.val();
    }

    void draw() {
        GuiGroupBox(_module_box, "VCA");
        _in_a1.gui.draw();
        _in_a2.gui.draw();
        _out_a1.gui.draw();
        _in_b1.gui.draw();
        _in_b2.gui.draw();
        _out_b1.gui.draw();
    }

    patch_destination _in_a1;
    patch_destination _in_a2;
    patch_source _out_a1;

    patch_destination _in_b1;
    patch_destination _in_b2;
    patch_source _out_b1;

private:
    Rectangle _module_box;
};

class sequencer {
public:
    sequencer() {}
    ~sequencer() {}

    void init(float sample_rate, float tempo) {

        _module_box = {30, 307.5f, 562.5f, 97.5f};
        _cv.gui.init("CV Out", {_module_box.x + 82.5f, _module_box.y + 52.5f});
        _trig.gui.init("Gate Out", {_module_box.x + 82.5f, _module_box.y + 15});
        _tempo = tempo;

        _metronome.Init(_tempo/60.0f*_num_steps, sample_rate); //multiplying by _num_steps should make it pulse once per step
        _step = 0;

        float radius = 22.5f;
        _tempo_knob = new knob({_module_box.x+15+radius, _module_box.y+26.25f+radius},radius, &_tempo, 30, 160);

        radius = 18.75;
        float gap = 15;
        for (int i = 0; i < _num_steps; i++) {
            _step_knobs[i] = new knob({_module_box.x+146.25f+radius+(i*(radius+radius+gap)), _module_box.y+52.5f+radius}, radius, &_cv_pattern[i], -1.0f, 1.0f);
        }
    }

    void process() {

        _metronome.SetFreq(_tempo/60.0f*_num_steps);

        int new_step = _metronome.Process();
        _trig.value = _trig_pattern[_step] * new_step;

        if(new_step) {
            
            _cv.value = _cv_pattern[_step];

            _step++;
            if (_step == _num_steps) {
                _step = 0;
            }
        }
    }

    void draw() {
        GuiGroupBox(_module_box, "Sequencer");

        for(int i = 0; i < _num_steps; i++) {
            Rectangle bounds = {_module_box.x+150+(i*52.5f), _module_box.y + 7.5f, 30, 30};
            GuiCheckBox(bounds, "", &_trig_pattern[i]);
            bounds.y += 30;
            bounds.width = 40;
            //GuiSlider(bounds, "", "", &_cv_pattern[i], -1.0f, 1.0f);

        }

        if(_trig.value) {
            DrawCircle(_module_box.x+10, _module_box.y+10, 5, RED);
        }

        _cv.gui.draw();
        _trig.gui.draw();
        _tempo_knob->draw();
        for (int i= 0; i < _num_steps; i++){
            _step_knobs[i]->draw();
        }
    }

    void set_trig(int step, bool value) {
        _trig_pattern[step] = value;
    }

    void set_cv(int step, float value) {
        _cv_pattern[step] = value;
    }

    patch_source _trig;
    patch_source _cv;
private:
    daisysp::Metro _metronome;

    static const int _num_steps = 8;
    int _step;
    bool _trig_pattern[_num_steps] = {0};
    float _cv_pattern[_num_steps] = {0.0f};
    float _tempo;

    Rectangle _module_box;

    knob* _tempo_knob;
    knob* _step_knobs[_num_steps];
};

class mixer {
public:
    mixer() {}
    ~mixer() {}

    void init() {
        _gain_1= 0.5;
        _gain_2 = 0.5;

        _module_box = {600, 45, 90, 360};

        _in_1.gui.init("Audio 1", {_module_box.x+7.5f, _module_box.y+22.5f});

        _in_2.gui.init("Audio 2", {_module_box.x+7.5f, _module_box.y+112.5f});

        float radius = 22.5;
        _gain_1_knob = new knob({_module_box.x+37.5f+radius, _module_box.y+45+radius}, radius, &_gain_1, 0.0, 1.0);
        _gain_2_knob = new knob({_module_box.x+37.5f+radius, _module_box.y+135+radius}, radius, &_gain_2, 0.0, 1.0);
    }

    float process() {
        float sample = (_in_1.val() * _gain_1) + (_in_2.val() * _gain_2);
        return sample;
    }

    float draw() {
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        GuiGroupBox(_module_box, "Mixer");

        // if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Gain 1", "", &_gain_1, 0.0, 1.0)) {}
        // y_index += y_pad;

        // if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Gain 2", "", &_gain_2, 0.0, 1.0)) {}
        // y_index += y_pad;

        _in_1.gui.draw();
        _in_2.gui.draw();
        _gain_1_knob->draw();
        _gain_2_knob->draw();
    }

    patch_destination _in_1;
    patch_destination _in_2;

private:
    float _gain_1;
    float _gain_2;

    Rectangle _module_box;

    knob* _gain_1_knob;
    knob* _gain_2_knob;
};