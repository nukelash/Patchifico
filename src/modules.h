
//modules: oscillator, lfo, vca, env, filter, sequencer(?)

#include "daisysp.h"
#include "raygui.h"
#include "raylib.h"

#include <unordered_map>

class patch_point_gui {
public:
    patch_point_gui() {}
    ~patch_point_gui() {}

    void init(std::string label, Vector2 position) {

        float width = 40;
        float height = 100;

        float x = position.x + (0.5*width);
        float y = position.y + (0.7*height);
        _circle_position = {x, y};
        _radius = 20;

        y = position.y + (0.3*height);
        _label_position = {position.x, y};
        _label = label; 
    }

    void draw() {
        DrawCircleV(_circle_position, _radius, RED);
        DrawCircleV(_circle_position, _radius-3, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        DrawText(_label.c_str(), _label_position.x, _label_position.y, 16, BLACK);
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

        _lfo_frequency = 0.5;
        for (auto i : _lfo_oscillators) {
            i->Init(sample_rate);
            i->SetFreq(_lfo_frequency);
        }
        _lfo_tri.SetWaveform(daisysp::Oscillator::WAVE_TRI);
        _lfo_saw.SetWaveform(daisysp::Oscillator::WAVE_SAW);
        _lfo_sqr.SetWaveform(daisysp::Oscillator::WAVE_SQUARE);

        _module_box.x = 20;
        _module_box.y = 20;
        _module_box.width = 150;
        _module_box.height = 300;

        _audio_frequency_mod.gui.init("Freq Mod", {_module_box.x+5, _module_box.y+30});

        _audio_tri_out.gui.init("Tri", {_module_box.x+5, _module_box.y+80});
        _audio_saw_out.gui.init("Saw", {_module_box.x+50, _module_box.y+80});
        _audio_sqr_out.gui.init("Sqr", {_module_box.x+95, _module_box.y+80});

        _lfo_tri_out.gui.init("Tri", {_module_box.x+5, _module_box.y+180});
        _lfo_saw_out.gui.init("Saw", {_module_box.x+50, _module_box.y+180});
        _lfo_sqr_out.gui.init("Sqr", {_module_box.x+95, _module_box.y+180});

        
    }

    void draw() {
        
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        GuiGroupBox(_module_box, "VCO");

        GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Frequency", "", &_audio_frequency, 20, 500);
        y_index += 160;

        GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "LFO Freq", "", &_lfo_frequency, 0.1, 50);
        y_index += y_pad;

        _audio_frequency_mod.gui.draw();

        for (auto i : _audio_outs) {
            i->gui.draw();
        }

        for (auto i : _lfo_outs) {
            i->gui.draw();
        }
    }

    float process() {

        for (auto i : _audio_oscillators) {
            i->SetFreq(_audio_frequency + (100*_audio_frequency_mod.val()));
        }

        for (auto i : _lfo_oscillators) {
            i->SetFreq(_lfo_frequency);
        }

        for (int i = 0; i < _audio_oscillators.size(); i++) {
            _audio_outs[i]->value = _audio_oscillators[i]->Process();
            _lfo_outs[i]->value = _lfo_oscillators[i]->Process();
        }

    }

    // process calls the daisy process function, THEN sets patch output variables accordingly;

    //callback will call each module process one at a time

    //draw will draw the GUI AND update module params accordingly
    patch_source _audio_tri_out;
    patch_source _audio_saw_out;
    patch_source _audio_sqr_out;
    std::vector<patch_source*> _audio_outs = {&_audio_tri_out, &_audio_saw_out, &_audio_sqr_out};

    patch_source _lfo_tri_out;
    patch_source _lfo_saw_out;
    patch_source _lfo_sqr_out;
    std::vector<patch_source*> _lfo_outs = {&_lfo_tri_out, &_lfo_saw_out, &_lfo_sqr_out};

    patch_destination _audio_frequency_mod;

private:

    daisysp::Oscillator _audio_tri;
    daisysp::Oscillator _audio_saw;
    daisysp::Oscillator _audio_sqr;
    std::vector<daisysp::Oscillator*> _audio_oscillators = {&_audio_tri, &_audio_saw, &_audio_sqr};
    float _audio_frequency;
    //int _wavetype;

    daisysp::Oscillator _lfo_tri;
    daisysp::Oscillator _lfo_saw;
    daisysp::Oscillator _lfo_sqr;
    std::vector<daisysp::Oscillator*> _lfo_oscillators = {&_lfo_tri, &_lfo_saw, &_lfo_sqr};
    float _lfo_frequency;

    //int _out_counter;
    

    //patch_destinations* _patch_destinations;

    patch_manager* _patch_bay;

    Rectangle _module_box;
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

        _module_box = {190, 20, 150, 300};

        _in.gui.init("input", {_module_box.x+5, _module_box.y+60});
        _out.gui.init("output", {_module_box.x+60, _module_box.y+60});

    }

    float process() {

        _filter.Process(_in.val());

        _out.value = _filter.Low();
    }

    void draw() {
        
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        GuiGroupBox(_module_box, "VCF");

        if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Frequency", "", &_frequency, 20, 15000)) {
            _filter.SetFreq(_frequency);
        }
        y_index += y_pad;

        if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Resonance", "", &_resonance, 0.0, 2.0)) {
            _filter.SetRes(_resonance);
        }
        y_index += y_pad;

        _in.gui.draw();
        _out.gui.draw();
    }

    patch_destination _in;
    patch_source _out;

private:
    daisysp::Svf _filter;
    float _frequency;
    float _resonance;

    patch_manager* _patch_bay;

    Rectangle _module_box;
};

class envelope_generator {
public:
    envelope_generator() {}
    ~envelope_generator() {}

    void init(float sample_rate) {
        _env.Init(sample_rate);

        _module_box = {380, 20, 150, 300};

        _attack = 0.1;
        _decay = 0.5;
        _last_trig_value = 0.0;

        _trigger.gui.init("trigger", {_module_box.x+5, _module_box.y+200});
        _output.gui.init("env out", {_module_box.x+50, _module_box.y+200});
    }

    void process() {
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

        if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Attack", "", &_attack, 0.01, 2.0)) {
            _env.SetTime(daisysp::ADENV_SEG_ATTACK, _attack);
        }
        y_index += y_pad;

        if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Decay", "", &_decay, 0.01, 2.0)) {
            _env.SetTime(daisysp::ADENV_SEG_DECAY, _decay);
        }
        y_index += y_pad;

        if (GuiButton((Rectangle){x_index, y_index, _module_box.width - (2*x_pad), 20}, "Trigger")) {
            _env.Trigger();
        }

        _trigger.gui.draw();
        _output.gui.draw();
        //std::cout << _env.GetCurrentSegment() <<std::endl;
    }

    patch_destination _trigger;
    patch_source _output;

private:
    daisysp::AdEnv _env;
    float _attack;
    float _decay;

    float _last_trig_value;

    Rectangle _module_box;

};

class vca {
public:
    vca() {}
    ~vca() {}

    void init() {
        _module_box = {600, 20, 100, 300};
        _in_1.gui.init("in1", {_module_box.x+5, _module_box.y+30});
        _in_2.gui.init("in2", {_module_box.x+5, _module_box.y+80});
        _out_1.gui.init("out1", {_module_box.x+5, _module_box.y+130});
    }

    void process() {
        _out_1.value = _in_1.val() * _in_2.val();
    }

    void draw() {
        GuiGroupBox(_module_box, "VCA");
        _in_1.gui.draw();
        _in_2.gui.draw();
        _out_1.gui.draw();
    }

    patch_destination _in_1;
    patch_destination _in_2;
    patch_source _out_1;

private:
    Rectangle _module_box;
};

class mixer {
public:
    mixer() {}
    ~mixer() {}

    void init() {
        _gain_1= 0.5;
        _gain_2 = 0.5;

        _module_box = {700, 20, 150, 300};

        _in_1.gui.init("Audio 1", {_module_box.x+5, _module_box.y+200});

        _in_2.gui.init("Audio 2", {_module_box.x+60, _module_box.y+200});
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

        if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Gain 1", "", &_gain_1, 0.0, 1.0)) {}
        y_index += y_pad;

        if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Gain 2", "", &_gain_2, 0.0, 1.0)) {}
        y_index += y_pad;

        _in_1.gui.draw();
        _in_2.gui.draw();
    }

    patch_destination _in_1;
    patch_destination _in_2;

private:
    float _gain_1;
    float _gain_2;

    Rectangle _module_box;
};