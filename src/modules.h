
//modules: oscillator, lfo, vca, env, filter, sequencer(?)

#include "daisysp.h"
#include "raygui.h"
#include "raylib.h"

#include "gui_components.h"
#include "visual_config.h"

#include <unordered_map>
#include <algorithm>


Color accent1 = {100, 49, 115, 255};
Color accent2 = {134, 165, 156, 255};
Color accent3 = {125, 91, 166, 255};
Color accent4 = {137, 206, 148, 255};

#define NUM_PATCH_COLORS 3
Color patch_colors[NUM_PATCH_COLORS] = {PACIFICO_GREEN, PACIFICO_BLUE, PACIFICO_RED};

class patch_point_gui {
public:
    patch_point_gui() {}
    ~patch_point_gui() {}

    void init(std::string label, Vector2 position, bool is_source) {

        _radius = 15;
        _circle_position = {position.x+_radius, position.y+_radius};
        _label_position = {position.x, position.y+_radius+20};
        _label = label; 
        _color = (is_source ? PACIFICO_BROWN : BLACK);

    }

    void draw() {
        DrawCircleV(_circle_position*BASE_UNIT, _radius*BASE_UNIT, _color);
        DrawCircleV(_circle_position*BASE_UNIT, (_radius-3)*BASE_UNIT, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        DrawTextEx(PANEL_FONT, _label.c_str(), {_label_position.x*BASE_UNIT, _label_position.y*BASE_UNIT}, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING, BLACK);
    }

    std::string _label;
    Vector2 _label_position;
    Vector2 _circle_position;
    float _radius;
    Color _color;
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

    struct patch_cable {

        patch_cable() {
            //_color = patch_colors[rand() % 4];
        }

        patch_cable(Color color){
            _color = color;
        }

        std::string _src_name;
        Vector2 _src_coordinates;
        std::string _dest_name;
        Vector2 _dest_coordinates;
        Color _color;

        void draw() {
            DrawCircleV(_src_coordinates*BASE_UNIT, 10*BASE_UNIT, _color);
            DrawCircleV(_dest_coordinates*BASE_UNIT, 10*BASE_UNIT, _color);

            float max_x = fmax(_dest_coordinates.x,_src_coordinates.x);
            float min_x = fmin(_dest_coordinates.x, _src_coordinates.x);
            Vector2 midpoint = {((max_x-min_x)/2.0f)+min_x, fmax(_src_coordinates.y, _dest_coordinates.y)+20};

            Vector2 points[5]  = {_src_coordinates*BASE_UNIT, _src_coordinates*BASE_UNIT, midpoint*BASE_UNIT, _dest_coordinates*BASE_UNIT,  _dest_coordinates*BASE_UNIT};
            DrawSplineCatmullRom(points, 5, 5*BASE_UNIT, _color);
        }
    };

    void add(std::string name, patch_source* source) {
        _sources.insert({name, source});
    }

    void add(std::string name, patch_destination* dest) {
        _destinations.insert({name, dest});
    }
    
    void draw() {
        for(auto i : _new_patch_cables) {
            i->draw();
        }
        //TODO: Connecting and unconnecting a lot leads to segfault occasionally?
        Vector2 mouse_location = GetMousePosition();
        std::string name;
        Vector2 patch_coords;

        if(!_connection_in_progress) {
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && point_colliding_patch_point(mouse_location, &name, &patch_coords)) {
                _connection_in_progress = true;

                bool is_source;
                patch_cable* cable;
                
                if(patch_point_occupied(name, &cable, &is_source)){
                    // find connection occupying this patch point
                    if(is_source) {
                        _in_progress_cable._src_name = "mouse";
                        _in_progress_cable._src_coordinates = mouse_location;
                        _in_progress_cable._dest_coordinates = cable->_dest_coordinates;
                        _in_progress_cable._dest_name = cable->_dest_name;
                    }
                    else {
                        _in_progress_cable._dest_name = "mouse";
                        _in_progress_cable._dest_coordinates = mouse_location;
                        _in_progress_cable._src_coordinates = cable->_src_coordinates;
                        _in_progress_cable._src_name = cable->_src_name;
                    }
                    _in_progress_cable._color = cable->_color;

                    // see erase-remove idiom
                    _new_patch_cables.erase(std::remove(_new_patch_cables.begin(), _new_patch_cables.end(), cable), _new_patch_cables.end());
                    disconnect(cable->_dest_name);
                    
                }
                else {
                    if(is_source) {
                        _in_progress_cable._src_name = name;
                        _in_progress_cable._src_coordinates = patch_coords;
                        _in_progress_cable._dest_name = "mouse";
                        _in_progress_cable._dest_coordinates = mouse_location;
                    }
                    else {
                        _in_progress_cable._src_name = "mouse";
                        _in_progress_cable._src_coordinates = mouse_location;
                        _in_progress_cable._dest_name = name;
                        _in_progress_cable._dest_coordinates = patch_coords;
                    }
                    _in_progress_cable._color = patch_colors[rand() % NUM_PATCH_COLORS];
                }
            }
        }

        if(_connection_in_progress) {
            std::string in_progress_patch_name;
            if(_in_progress_cable._src_name == "mouse") {
                // Since patch cables multiply their coordinates by BASE_UNIT when drawing, we divide the mouse coordinates by BASE_UNIT here to draw the true mouse location
                _in_progress_cable._src_coordinates = mouse_location/BASE_UNIT;
                in_progress_patch_name = _in_progress_cable._dest_name;
            }
            else if(_in_progress_cable._dest_name == "mouse") {
                _in_progress_cable._dest_coordinates = mouse_location/BASE_UNIT;
                in_progress_patch_name = _in_progress_cable._src_name;
            }
            _in_progress_cable.draw();

            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if(point_colliding_patch_point(mouse_location, &name, &patch_coords)) {
                    std::string source_name, destination_name;
                    if(check_valid_connection(in_progress_patch_name, name, &source_name, &destination_name)) {
                        connect(source_name, destination_name, _in_progress_cable._color);
                    }
                }

                _connection_in_progress = false;
            }
        }
    }

    void connect(std::string source_name, std::string dest_name, Color color) {
        patch_destination* dest = _destinations[dest_name];

        if (dest->connected) {
            disconnect(dest_name);
        }

        dest->source = _sources[source_name];

        dest->connected = true;

        // TODO: I don't really delete these anywhere, should fix
        patch_cable* c = new patch_cable(color);
        c->_src_name = source_name;
        c->_src_coordinates = _sources[source_name]->gui._circle_position;
        c->_dest_name = dest_name;
        c->_dest_coordinates = _destinations[dest_name]->gui._circle_position;
        _new_patch_cables.push_back(c);
    }

    void disconnect(std::string dest_name) {
        _destinations[dest_name]->source = nullptr;
        _destinations[dest_name]->connected = false;
        //need to delete pointer i.e. memory leak
        for(int i = 0; i < _new_patch_cables.size(); i++) {
            if(_new_patch_cables[i]->_dest_name == dest_name) {
                _new_patch_cables.erase(_new_patch_cables.begin() + i);
            }
            
        }
        
    }

private:

    bool point_colliding_patch_source(Vector2 point, std::string* name) {
        for(auto i : _sources) {
            auto idx = i.first;
            if (CheckCollisionPointCircle(point, (_sources[idx]->gui._circle_position)*BASE_UNIT, 
            (_sources[idx]->gui._radius))*BASE_UNIT) {
                *name = idx;
                return true;
            }
        }
        return false;
    }

    bool point_colliding_patch_destination(Vector2 point, std::string* name) {
        for(auto i : _destinations) {
            auto idx = i.first;
            if (CheckCollisionPointCircle(point, (_destinations[idx]->gui._circle_position)*BASE_UNIT, (_destinations[idx]->gui._radius)*BASE_UNIT)) {
                *name = idx;
                return true;
            }
        }
        return false;
    }

    bool point_colliding_patch_point(Vector2 point, std::string* name, Vector2* patch_coordinates) {
        for(auto i : _sources) {
            auto idx = i.first;
            if (CheckCollisionPointCircle(point, (_sources[idx]->gui._circle_position)*BASE_UNIT, (_sources[idx]->gui._radius)*BASE_UNIT)) {
                *name = idx;
                *patch_coordinates = _sources[idx]->gui._circle_position;
                return true;
            }
        }
        for(auto i : _destinations) {
            auto idx = i.first;
            if (CheckCollisionPointCircle(point, (_destinations[idx]->gui._circle_position)*BASE_UNIT, (_destinations[idx]->gui._radius)*BASE_UNIT)) {
                *name = idx;
                *patch_coordinates = _destinations[idx]->gui._circle_position;
                return true;
            }
        }
        return false;
    }

    bool patch_point_occupied(std::string name, patch_cable** cable, bool* is_source) {
        for (auto c : _new_patch_cables) {
            if (c->_src_name == name) {
                *cable = c;
                *is_source = true;
                return true;
            }
            if (c->_dest_name == name) {
                *cable = c;
                *is_source = false;
                return true;
            }
        }

        return false;
    }

    bool check_valid_connection(std::string name1, std::string name2, std::string* source_name, std::string* destination_name) {
        //check if name1 and name2 represent 1 source and 1 destination
        int source_hits, destination_hits = 0;
        for(auto src : _sources) {
            std::string src_name = src.first;
            if(src_name == name1) {
                *source_name = name1;
                source_hits++;
            }
            if(src_name == name2) {
                *source_name = name2;
                source_hits++;
            }
        }
        for(auto dest : _destinations) {
            std::string dest_name = dest.first;
            if(dest_name == name1) {
                *destination_name = name1;
                destination_hits++;
            }
            if(dest_name == name2) {
                *destination_name = name2;
                destination_hits++;
            }
        }

        if((destination_hits == 1) || (source_hits == 1)) {
            return true;
        }

        return false;
    }

    std::unordered_map<std::string, patch_source*> _sources;
    std::unordered_map<std::string, patch_destination*> _destinations;

    std::vector<patch_cable*> _new_patch_cables;
    patch_cable _in_progress_cable;

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

        _module_box.x = 10;
        _module_box.y = 85;
        _module_box.width = 210;
        _module_box.height = 142.5;

        _audio_frequency_mod.gui.init("", {_module_box.x+37.5f, _module_box.y+82.5f}, false);
        _pulse_width.gui.init("", {_module_box.x+97.5f, _module_box.y+82.5f}, false);

        _audio_tri_out.gui.init("", {_module_box.x+157.5f, _module_box.y+37.5f}, true);
        _audio_saw_out.gui.init("", {_module_box.x+157.5f, _module_box.y+82.5f}, true);
        _audio_sqr_out.gui.init("", {_module_box.x+97.5f, _module_box.y+37.5f}, true);

        float freq_radius = 30;
        _frequency_knob = new knob({_module_box.x + freq_radius + 22.5f, _module_box.y + freq_radius + 15.0f},freq_radius, &_audio_frequency, 0.0, 1.0);

        _group_box = new group(_module_box, "Oscillator");

        _map = new parameter_map(60, 260, 880);

        _saw_wave = LoadTexture("/Users/lukenash/Documents/Github/synth/saw_wave.png");
        _tri_wave = LoadTexture("/Users/lukenash/Documents/Github/synth/tri_wave_12.png");
        _sqr_wave = LoadTexture("/Users/lukenash/Documents/Github/synth/sqr_wave.png");
    }

    void draw() {
        
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        _group_box->draw();

        _frequency_knob->draw();
        y_index += 160;

        _audio_frequency_mod.gui.draw();
        _pulse_width.gui.draw();

        for (auto i : _audio_outs) {
            i->gui.draw();
        }

        _frequency_knob->draw();

        float symbol_scale = 0.06;

        Vector2 tri_wave_position = {_audio_tri_out.gui._circle_position.x-(_tri_wave.width*symbol_scale)/2.0f, _audio_tri_out.gui._circle_position.y-_audio_tri_out.gui._radius-(_tri_wave.height*symbol_scale)-5.0f};
        DrawTextureEx(_tri_wave, tri_wave_position*BASE_UNIT, 0,  symbol_scale*BASE_UNIT, WHITE);

        Vector2 sqr_wave_position = {_audio_sqr_out.gui._circle_position.x-(_sqr_wave.width*symbol_scale)/2.0f, _audio_sqr_out.gui._circle_position.y-_audio_sqr_out.gui._radius-(_sqr_wave.height*symbol_scale)-5.0f};
        DrawTextureEx(_sqr_wave, sqr_wave_position*BASE_UNIT, 0, symbol_scale*BASE_UNIT, WHITE);

        Vector2 saw_wave_position = {_audio_saw_out.gui._circle_position.x-(_saw_wave.width*symbol_scale)/2.0f, _audio_saw_out.gui._circle_position.y+_audio_saw_out.gui._radius+5.0f};
        DrawTextureEx(_saw_wave, saw_wave_position*BASE_UNIT, 0, symbol_scale*BASE_UNIT, WHITE);

        Vector2 pw_label_position = {_pulse_width.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "PW", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _pulse_width.gui._circle_position.y + _pulse_width.gui._radius + 5};
        DrawTextEx(PANEL_FONT, "PW", pw_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 freq_label_position = {_audio_frequency_mod.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "Freq", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _audio_frequency_mod.gui._circle_position.y + _audio_frequency_mod.gui._radius + 5};
        DrawTextEx(PANEL_FONT, "Freq", freq_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);
    }

    float process() {

        for (auto i : _audio_oscillators) {
            i->SetFreq(_map->process(_audio_frequency + (0.2*_audio_frequency_mod.val())));
            i->SetPw((_pulse_width.val()+1.0f)/2.0f);
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
    patch_destination _pulse_width;

private:

    daisysp::Oscillator _audio_tri;
    daisysp::Oscillator _audio_saw;
    daisysp::Oscillator _audio_sqr;
    std::vector<daisysp::Oscillator*> _audio_oscillators = {&_audio_tri, &_audio_saw, &_audio_sqr};
    float _audio_frequency;

    patch_manager* _patch_bay;

    Rectangle _module_box;

    knob* _frequency_knob;
    group* _group_box;

    parameter_map* _map;

    Texture2D _tri_wave;
    Texture2D _saw_wave;
    Texture2D _sqr_wave;
};

class lfo {
public:
    lfo() {}
    ~lfo() {}

    void init(float sample_rate, patch_manager* patch_bay) {
        _patch_bay = patch_bay;

        _module_box = {10, 237.5, 150, 142.5};
        _map = new parameter_map(0.01, 10, 60);

        _lfo_frequency = 0.5;
        for (auto i : _lfo_oscillators) {
            i->Init(sample_rate);
            i->SetFreq(_map->process(_lfo_frequency));
        }
        _lfo_tri.SetWaveform(daisysp::Oscillator::WAVE_TRI);
        _lfo_saw.SetWaveform(daisysp::Oscillator::WAVE_SAW);
        _lfo_sqr.SetWaveform(daisysp::Oscillator::WAVE_SQUARE);

        _lfo_tri_out.gui.init("", {_module_box.x+60, _module_box.y+75}, true);
        _lfo_saw_out.gui.init("", {_module_box.x+105, _module_box.y+75}, true);
        _lfo_sqr_out.gui.init("", {_module_box.x+15.0f, _module_box.y+75.0f}, true);

        _pulse_width.gui.init("", {_module_box.x+15, _module_box.y+67.5f}, false);
        _retrig.gui.init("", {_module_box.x+82.5f, _module_box.y+30}, false);

        float radius = 22.5f;
        _frequency_knob  = new knob({_module_box.x+30.0f+radius, _module_box.y+22.5f+radius}, radius, &_lfo_frequency, 0.0, 1.0f);

        _group_box = new group(_module_box, "LFO");

        _saw_wave = LoadTexture("/Users/lukenash/Documents/Github/synth/saw_wave.png");
        _tri_wave = LoadTexture("/Users/lukenash/Documents/Github/synth/tri_wave_12.png");
        _sqr_wave = LoadTexture("/Users/lukenash/Documents/Github/synth/sqr_wave.png");
    }

    void draw() {

        _group_box->draw();

        _frequency_knob->draw();

        for (auto i : _lfo_outs) {
            i->gui.draw();
        }

        //_pulse_width.gui.draw();
        _retrig.gui.draw();

        float symbol_scale = 0.06;

        Vector2 tri_wave_position = {_lfo_tri_out.gui._circle_position.x-(_tri_wave.width*symbol_scale)/2.0f, _lfo_tri_out.gui._circle_position.y+_lfo_tri_out.gui._radius+5.0f};
        DrawTextureEx(_tri_wave, tri_wave_position*BASE_UNIT, 0,  symbol_scale*BASE_UNIT, WHITE);

        Vector2 sqr_wave_position = {_lfo_sqr_out.gui._circle_position.x-(_sqr_wave.width*symbol_scale)/2.0f, _lfo_sqr_out.gui._circle_position.y+_lfo_sqr_out.gui._radius+5.0f};
        DrawTextureEx(_sqr_wave, sqr_wave_position*BASE_UNIT, 0, symbol_scale*BASE_UNIT, WHITE);

        Vector2 saw_wave_position = {_lfo_saw_out.gui._circle_position.x-(_saw_wave.width*symbol_scale)/2.0f, _lfo_saw_out.gui._circle_position.y+_lfo_saw_out.gui._radius+5.0f};
        DrawTextureEx(_saw_wave, saw_wave_position*BASE_UNIT, 0, symbol_scale*BASE_UNIT, WHITE);

        Vector2 reset_label_position = {_retrig.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "Reset", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _retrig.gui._circle_position.y - _retrig.gui._radius - 12.5f};
        DrawTextEx(PANEL_FONT, "Reset", reset_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);
    }

    float process() {
        for (auto i : _lfo_oscillators) {
            i->SetFreq(_map->process(_lfo_frequency));
            i->SetPw((_pulse_width.val()+1.0f)/2.0f);
        }

        if((_last_retrig_value <= 0.5f) && (_retrig.val() >0.5f)) {
            for (auto i : _lfo_oscillators) {
                i->Reset();
            }
        }

        for (int i = 0; i < _lfo_oscillators.size(); i++) {
            _lfo_outs[i]->value = _lfo_oscillators[i]->Process();
        }
    }

    patch_source _lfo_tri_out;
    patch_source _lfo_saw_out;
    patch_source _lfo_sqr_out;
    std::vector<patch_source*> _lfo_outs = {&_lfo_tri_out, &_lfo_saw_out, &_lfo_sqr_out};

    patch_destination _pulse_width;
    patch_destination _retrig;

private:

    daisysp::Oscillator _lfo_tri;
    daisysp::Oscillator _lfo_saw;
    daisysp::Oscillator _lfo_sqr;
    std::vector<daisysp::Oscillator*> _lfo_oscillators = {&_lfo_tri, &_lfo_saw, &_lfo_sqr};
    float _lfo_frequency;

    patch_manager* _patch_bay;

    Rectangle _module_box;

    knob* _frequency_knob;
    group* _group_box;
    parameter_map* _map;

    float _last_retrig_value = 0.0f;

    Texture2D _tri_wave;
    Texture2D _saw_wave;
    Texture2D _sqr_wave;
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

        _module_box = {167.5, 237.5, 150, 142.5};

        _in.gui.init("", {_module_box.x+15.0f, _module_box.y+15.0f}, false);
        _out.gui.init("", {_module_box.x+105.0f, _module_box.y+15.0f}, true);
        _cutoff_mod.gui.init("", {_module_box.x+60.0f, _module_box.y+15.0f}, false);

        float radius = 22.5f;
        _cutoff_knob = new knob({_module_box.x+22.5f+radius, _module_box.y+67.5f+radius}, radius, &_frequency, 0.0, 1.0);

        _resonance_knob = new knob({_module_box.x+82.5f+radius, _module_box.y+67.5f+radius}, radius, &_resonance, 0.0, 1.5f);

        _group_box = new group(_module_box, "Filter");

        _map = new parameter_map(20, 2000, 15000);

    }

    float process() {

        _filter.SetFreq(_map->process(_frequency+(0.2*_cutoff_mod.val())));
        _filter.SetRes(_resonance);

        _filter.Process(_in.val());

        _out.value = _filter.Low();
    }

    void draw() {
        
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;

        _group_box->draw();
        //DrawLineEx(_cutoff_knob->_position, _cutoff_mod.gui._circle_position, 3, BLACK);
        _in.gui.draw();
        _out.gui.draw();
        _cutoff_mod.gui.draw();
        _cutoff_knob->draw();
        _resonance_knob->draw();

        Vector2 cutoff_label_position = {_cutoff_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "Cutoff", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _cutoff_knob->_position.y + _cutoff_knob->_radius + 5};
        DrawTextEx(PANEL_FONT, "Cutoff", cutoff_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 resonance_label_position = {_resonance_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "Resonance", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _resonance_knob->_position.y + _resonance_knob->_radius + 5};
        DrawTextEx(PANEL_FONT, "Resonance", resonance_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 in_label_position = {_in.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in.gui._circle_position.y + _in.gui._radius + 5};
        DrawTextEx(PANEL_FONT, "in", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 cutoff_label_position_2 = {_cutoff_mod.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "cutoff", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _cutoff_mod.gui._circle_position.y + _cutoff_mod.gui._radius + 5};
        DrawTextEx(PANEL_FONT, "cutoff", cutoff_label_position_2*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);
        
        Vector2 out_label_position = {_out.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "out", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _out.gui._circle_position.y + _out.gui._radius + 5};
        DrawTextEx(PANEL_FONT, "out", out_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);
    }

    patch_destination _in;
    patch_destination _cutoff_mod;
    patch_source _out;

private:
    daisysp::Svf _filter;
    float _frequency;
    float _resonance;

    patch_manager* _patch_bay;

    Rectangle _module_box;

    knob* _cutoff_knob;
    knob* _resonance_knob;
    group* _group_box;
    parameter_map* _map;
};

class envelope_generator {
public:
    envelope_generator() {}
    ~envelope_generator() {}

    void init(float sample_rate) {
        _env.Init(sample_rate);

        _module_box = {325,237.5, 142.5, 142.5};

        _attack = 0.1;
        _decay = 0.5;
        _last_trig_value = 0.0;

        _trigger.gui.init("", {_module_box.x+37.5f, _module_box.y+15.0f}, false);
        _output.gui.init("", {_module_box.x+82.5f, _module_box.y+15.0f}, true);

        float radius = 22.5f;
        _attack_knob = new knob({_module_box.x+15+radius, _module_box.y+67.5f+radius}, radius, &_attack, 0.0, 1.0);
        _decay_knob = new knob({_module_box.x+82.5f+radius, _module_box.y+67.5f+radius}, radius, &_decay, 0.0, 1.0);

        _group_box = new group(_module_box, "Envelope");

        _trig_button = new push_button({_module_box.x+12.0f, _module_box.y+20.25f}, &_trig_button_was_pushed);

        _light = new light({_module_box.x+120.0f+5, _module_box.y+26.25f+5}, 5, PACIFICO_RED);

        _map = new parameter_map(0.01, 0.2, 1.5);
    }

    void process() {

        _env.SetTime(daisysp::ADENV_SEG_ATTACK, _map->process(_attack));
        _env.SetTime(daisysp::ADENV_SEG_DECAY, _map->process(_decay));

        if(_trigger.val() > _last_trig_value) {
            _env.Trigger();
        }
        _last_trig_value = _trigger.val();
        _output.value = _env.Process();
    }

    void draw() {

        _light->set_brightness(_output.value);
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        _group_box->draw();

        if (_trig_button_was_pushed){
            _env.Trigger();
        }

        _trigger.gui.draw();
        _output.gui.draw();
        _attack_knob->draw();
        _decay_knob->draw();
        _trig_button->draw();
        _light->draw();

        Vector2 trigger_label_position = {_attack_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "trigger", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _trigger.gui._circle_position.y + _trigger.gui._radius + 3.0f};
        DrawTextEx(PANEL_FONT, "trigger", trigger_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 out_label_position = {_decay_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "out", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _trigger.gui._circle_position.y + _trigger.gui._radius + 3.0f};
        DrawTextEx(PANEL_FONT, "out", out_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 attack_label_position = {_attack_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "Attack", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _attack_knob->_position.y + _attack_knob->_radius + 5.0f};
        DrawTextEx(PANEL_FONT, "Attack", attack_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 decay_label_position = {_decay_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "Decay", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _decay_knob->_position.y + _decay_knob->_radius + 5.0f};
        DrawTextEx(PANEL_FONT, "Decay", decay_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);
    }

    patch_destination _trigger;
    patch_source _output;

private:
    daisysp::AdEnv _env;
    float _attack;
    float _decay;

    float _last_trig_value;
    bool _trig_button_was_pushed;

    Rectangle _module_box;

    knob* _attack_knob;
    knob* _decay_knob;
    group* _group_box;
    push_button* _trig_button;
    light* _light;

    parameter_map* _map;
};

class mult {
public:
    mult() {}
    ~mult(){}

    void init() {
        _module_box = {227.5f, 85, 240, 142.5};

        _in.gui.init("", {_module_box.x+22.5f, _module_box.y+60}, false);
        _out1.gui.init("", {_module_box.x+82.5f, _module_box.y+92.5f}, true);
        _out2.gui.init("", {_module_box.x+135, _module_box.y+92.5f}, true);
        _out3.gui.init("", {_module_box.x+187.5f, _module_box.y+92.5f}, true);

        _group_box = new group(_module_box, "Mult");

        float radius = 18.75f;
        _knob_1 = new knob({_module_box.x+78.75f+radius, _module_box.y+30.0f+radius}, radius, &_gain_1, -1.0, 1.0);
        _knob_2 = new knob({_module_box.x+131.25f+radius, _module_box.y+30.0f+radius}, radius, &_gain_2, -1.0, 1.0);
        _knob_3 = new knob({_module_box.x+183.75f+radius, _module_box.y+30.0f+radius}, radius, &_gain_3, -1.0, 1.0);
    }

    void draw() {

        _group_box->draw();
        _in.gui.draw();
        _out1.gui.draw();
        _out2.gui.draw();
        _out3.gui.draw();

        _knob_1->draw();
        _knob_2->draw();
        _knob_3->draw();

        int line_thickness = 3; 
        Vector2 top_left = {_in.gui._circle_position.x+_in.gui._radius, _in.gui._circle_position.y+_in.gui._radius};
        Vector2 top_right = {_out1.gui._circle_position.x-_in.gui._radius, _out1.gui._circle_position.y+_out1.gui._radius};
        Vector2 bottom_left = {_out2.gui._circle_position.x+_in.gui._radius, _out2.gui._circle_position.y-_out2.gui._radius};
        Vector2 bottom_right = {_out3.gui._circle_position.x-_in.gui._radius, _out3.gui._circle_position.y-_out3.gui._radius};

        Vector2 top_1 = {_in.gui._circle_position.x, _module_box.y + 15.0f};
        Vector2 top_2 = {_out1.gui._circle_position.x, _module_box.y + 15.0f};
        Vector2 top_3 = {_out2.gui._circle_position.x, _module_box.y + 15.0f};
        Vector2 top_4 = {_out3.gui._circle_position.x, _module_box.y + 15.0f};


        float spacing = 5.0f;
        DrawLineEx((Vector2){_in.gui._circle_position.x, _in.gui._circle_position.y - _in.gui._radius - spacing}*BASE_UNIT, top_1*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){_knob_1->_position.x, _knob_1->_position.y - _knob_1->_radius - spacing}*BASE_UNIT, top_2*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){_knob_2->_position.x, _knob_2->_position.y - _knob_2->_radius - spacing}*BASE_UNIT, top_3*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){_knob_3->_position.x, _knob_3->_position.y - _knob_3->_radius - spacing}*BASE_UNIT, top_4*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){top_1.x, top_1.y+(line_thickness*0.5f)}*BASE_UNIT, (Vector2){top_4.x, top_4.y+(line_thickness*0.5f)}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);

        Vector2 triangle_tip_1 = {_out1.gui._circle_position.x, _out1.gui._circle_position.y - _out1.gui._radius - spacing};
        Vector2 triangle_tip_2 = {_out2.gui._circle_position.x, _out2.gui._circle_position.y - _out2.gui._radius - spacing};
        Vector2 triangle_tip_3 = {_out3.gui._circle_position.x, _out3.gui._circle_position.y - _out3.gui._radius - spacing};

        DrawLineEx((Vector2){_knob_1->_position.x, _knob_1->_position.y + _knob_1->_radius + spacing}*BASE_UNIT, (Vector2){triangle_tip_1.x, triangle_tip_1.y-5}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){_knob_2->_position.x, _knob_2->_position.y + _knob_2->_radius + spacing}*BASE_UNIT, (Vector2){triangle_tip_2.x, triangle_tip_2.y-5}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){_knob_3->_position.x, _knob_3->_position.y + _knob_3->_radius + spacing}*BASE_UNIT, (Vector2){triangle_tip_3.x, triangle_tip_3.y-5}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);

        DrawTriangle((Vector2){triangle_tip_1}*BASE_UNIT, (Vector2){triangle_tip_1.x+10.0f, triangle_tip_1.y-10.0f}*BASE_UNIT, (Vector2){triangle_tip_1.x-10.0f, triangle_tip_1.y-10.0f}*BASE_UNIT, BLACK);
        DrawTriangle((Vector2){triangle_tip_2}*BASE_UNIT, (Vector2){triangle_tip_2.x+10.0f, triangle_tip_2.y-10.0f}*BASE_UNIT, (Vector2){triangle_tip_2.x-10.0f, triangle_tip_2.y-10.0f}*BASE_UNIT, BLACK);
        DrawTriangle((Vector2){triangle_tip_3}*BASE_UNIT, (Vector2){triangle_tip_3.x+10.0f, triangle_tip_3.y-10.0f}*BASE_UNIT, (Vector2){triangle_tip_3.x-10.0f, triangle_tip_3.y-10.0f}*BASE_UNIT, BLACK);

        Vector2 in_label_position = {_in.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in.gui._circle_position.y + _in.gui._radius + 3.0f};
        DrawTextEx(PANEL_FONT, "in", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 out_label_position = {_out1.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "out", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _out1.gui._circle_position.y + _out1.gui._radius + 2.0f};
        DrawTextEx(PANEL_FONT, "out", out_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        out_label_position = {_out2.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "out", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _out2.gui._circle_position.y + _out2.gui._radius + 2.0f};
        DrawTextEx(PANEL_FONT, "out", out_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        out_label_position = {_out3.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "out", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _out3.gui._circle_position.y + _out3.gui._radius + 2.0f};
        DrawTextEx(PANEL_FONT, "out", out_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);


    }

    void process() {
        _out1.value = _in.val() * _gain_1;
        _out2.value = _in.val() * _gain_2;
        _out3.value = _in.val() * _gain_3;
    }

    patch_destination _in;
    patch_source _out1;
    patch_source _out2;
    patch_source _out3;

private:
    Rectangle _module_box;
    group* _group_box;

    knob* _knob_1;
    knob* _knob_2;
    knob* _knob_3;

    float _gain_1;
    float _gain_2;
    float _gain_3;
};

class vca {
public:
    vca() {}
    ~vca() {}

    void init() {
        _module_box = {475, 85, 97.5f, 295};
        _in_a1.gui.init("", {_module_box.x+15, _module_box.y+30.0f}, false);
        _in_a2.gui.init("", {_module_box.x+55, _module_box.y+30.0f}, false);
        _out_a1.gui.init("", {_module_box.x+55, _module_box.y+95.0f}, true);

        _in_b1.gui.init("", {_module_box.x+15, _module_box.y+167.5f}, false);
        _in_b2.gui.init("", {_module_box.x+55, _module_box.y+167.5f}, false);
        _out_b1.gui.init("", {_module_box.x+55, _module_box.y+232.5f}, true);

        _group_box = new group(_module_box, "VCA");
    }

    void process() {
        _out_a1.value = _in_a1.val() * _in_a2.val();
        _out_b1.value = _in_b1.val() * _in_b2.val();
    }

    void draw() {
        _group_box->draw();
        _in_a1.gui.draw();
        _in_a2.gui.draw();
        _out_a1.gui.draw();
        _in_b1.gui.draw();
        _in_b2.gui.draw();
        _out_b1.gui.draw();

        int line_thickness = 3;

        // Top arrow
        Vector2 in_a1_center_bottom = {_in_a1.gui._circle_position.x, _in_a1.gui._circle_position.y+_in_a1.gui._radius};
        Vector2 in_a2_center_bottom = {_in_a2.gui._circle_position.x, _in_a2.gui._circle_position.y+_in_a2.gui._radius};
        Vector2 out_a_center_top = {_out_a1.gui._circle_position.x, _out_a1.gui._circle_position.y-_out_a1.gui._radius};
        Vector2 a_midpoint = {in_a1_center_bottom.x+(in_a2_center_bottom.x-in_a1_center_bottom.x)/2.0f, in_a1_center_bottom.y+(out_a_center_top.y-in_a1_center_bottom.y)/3.0f};

        DrawLineEx((Vector2){in_a1_center_bottom.x, in_a1_center_bottom.y+2.5f}*BASE_UNIT, (Vector2){in_a1_center_bottom.x, a_midpoint.y}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){in_a2_center_bottom.x, in_a2_center_bottom.y+2.5f}*BASE_UNIT, (Vector2){out_a_center_top.x, out_a_center_top.y-5.0f}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){in_a1_center_bottom.x-(line_thickness/2.0f), a_midpoint.y}*BASE_UNIT, (Vector2){out_a_center_top.x, a_midpoint.y}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        
        DrawTriangle((Vector2){out_a_center_top.x, out_a_center_top.y-2.5f}*BASE_UNIT, (Vector2){out_a_center_top.x+10.0f, out_a_center_top.y-12.5f}*BASE_UNIT, (Vector2){out_a_center_top.x-10.0f, out_a_center_top.y-12.5f}*BASE_UNIT, BLACK);

        // Bottom arrow
        Vector2 in_b1_center_bottom = {_in_b1.gui._circle_position.x, _in_b1.gui._circle_position.y+_in_b1.gui._radius};
        Vector2 in_b2_center_bottom = {_in_b2.gui._circle_position.x, _in_b2.gui._circle_position.y+_in_b2.gui._radius};
        Vector2 out_b_center_top = {_out_b1.gui._circle_position.x, _out_b1.gui._circle_position.y-_out_b1.gui._radius};
        Vector2 b_midpoint = {in_b1_center_bottom.x+(in_b2_center_bottom.x-in_b1_center_bottom.x)/2.0f, in_b1_center_bottom.y+(out_b_center_top.y-in_b1_center_bottom.y)/3.0f};

        DrawLineEx((Vector2){in_b1_center_bottom.x, in_b1_center_bottom.y+2.5f}*BASE_UNIT, (Vector2){in_b1_center_bottom.x, b_midpoint.y}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){in_b2_center_bottom.x, in_b2_center_bottom.y+2.5f}*BASE_UNIT, (Vector2){out_b_center_top.x, out_b_center_top.y-5.0f}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        DrawLineEx((Vector2){in_b1_center_bottom.x-(line_thickness/2.0f), b_midpoint.y}*BASE_UNIT, (Vector2){out_b_center_top.x, b_midpoint.y}*BASE_UNIT, line_thickness*BASE_UNIT, BLACK);
        
        DrawTriangle((Vector2){out_b_center_top.x, out_b_center_top.y-2.5f}*BASE_UNIT, (Vector2){out_b_center_top.x+10.0f, out_b_center_top.y-12.5f}*BASE_UNIT, (Vector2){out_b_center_top.x-10.0f, out_b_center_top.y-12.5f}*BASE_UNIT, BLACK);
        
        Vector2 in_label_position = {_in_a1.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in_a1.gui._circle_position.y - _in_a1.gui._radius - 12.5f};
        DrawTextEx(PANEL_FONT, "in", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        in_label_position = {_in_a2.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in_a2.gui._circle_position.y - _in_a2.gui._radius - 12.5f};
        DrawTextEx(PANEL_FONT, "in", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        in_label_position = {_in_b1.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in_b1.gui._circle_position.y - _in_b1.gui._radius - 12.5f};
        DrawTextEx(PANEL_FONT, "in", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        in_label_position = {_in_b2.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in_b2.gui._circle_position.y - _in_b2.gui._radius - 12.5f};
        DrawTextEx(PANEL_FONT, "in", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 out_label_position = {_out_a1.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "out", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _out_a1.gui._circle_position.y + _out_a1.gui._radius + 3.0f};
        DrawTextEx(PANEL_FONT, "out", out_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        out_label_position = {_out_b1.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "out", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _out_b1.gui._circle_position.y + _out_b1.gui._radius + 3.0f};
        DrawTextEx(PANEL_FONT, "out", out_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        
    }

    patch_destination _in_a1;
    patch_destination _in_a2;
    patch_source _out_a1;

    patch_destination _in_b1;
    patch_destination _in_b2;
    patch_source _out_b1;

private:
    Rectangle _module_box;
    group* _group_box;
};

class sequencer {
public:
    sequencer() {}
    ~sequencer() {}

    void init(float sample_rate, float tempo) {

        _module_box = {10, 390, 562.5f, 97.5f};
        _cv.gui.init("", {_module_box.x + 92.5f, _module_box.y + 52.5f}, true);
        _trig.gui.init("", {_module_box.x + 92.5f, _module_box.y + 10}, true);
        _tempo = tempo;

        _metronome.Init(_tempo/60.0f*_num_steps, sample_rate); //multiplying by _num_steps should make it pulse once per step
        _step = 0;

        float radius = 22.5f;
        _tempo_knob = new knob({_module_box.x+15+radius, _module_box.y+22.5f+radius},radius, &_tempo, 0.0, 1.0);

        radius = 18.75;
        float gap = 15;
        for (int i = 0; i < _num_steps; i++) {
            _step_knobs[i] = new knob({_module_box.x+146.25f+radius+(i*(radius+radius+gap)), _module_box.y+50+radius}, radius, &_cv_pattern[i], -1.0f, 1.0f);
            _step_switches[i] = new toggle_switch({_module_box.x+152.5f+(i*52.5f), _module_box.y + 12.5f}, &_trig_pattern[i]);
            _lights[i] = new light({_module_box.x+146.25f+radius+(i*(radius+radius+gap)+12.5f), _module_box.y+50.0f}, 5, PACIFICO_RED);
            _cursor_positions[i] = {_module_box.x+150+(i*52.5f), _module_box.y + 10.0f};
        }

        _cv_light = new light({_cv.gui._circle_position.x + _cv.gui._radius + 12.0f, _cv.gui._circle_position.y}, 5, PACIFICO_RED);
        _trig_light = new light({_trig.gui._circle_position.x + _trig.gui._radius + 12.0f, _trig.gui._circle_position.y}, 5, PACIFICO_RED);

        _group_box = new group(_module_box, "Sequencer");

        _map = new parameter_map(10, 95, 400);

    }

    void process() {

        _metronome.SetFreq(_map->process(_tempo)/60.0f*_num_steps);

        int new_step = _metronome.Process();
        
        // This, along with setting _trig.value false below, forces trig.value to be false
        // for a single sample between steps, so that the envelope is properly triggered.
        if (_trig_reset) {
            _trig.value = _trig_pattern[_step];
            _trig_reset = false;
        }

        if(new_step) {
            _lights[_step]->set_brightness(0.2);
            
            _step++;
            if (_step == _num_steps) {
                _step = 0;
            }

            _trig_reset = true;
            _trig.value = false;
            _cv.value = _cv_pattern[_step];

            _trig_light->set_brightness(_trig.value);
            _cv_light->set_brightness((_cv.value+1.0f)/2.0f);

            
            _lights[_step]->set_brightness(1.0);
        }
    }

    void draw() {
        _group_box->draw();

        for(int i = 0; i < _num_steps; i++) {
            // Rectangle bounds = {_module_box.x+150+(i*52.5f), _module_box.y + 7.5f, 30, 30};
            // GuiCheckBox(bounds, "", &_trig_pattern[i]);
            // bounds.y += 30;
            // bounds.width = 40;
            
        }

        _cv.gui.draw();
        _trig.gui.draw();
        _tempo_knob->draw();
        for (int i= 0; i < _num_steps; i++){
            _step_knobs[i]->draw();
            _step_switches[i]->draw();
            //_lights[i]->draw();
        }
        _cv_light->draw();
        _trig_light->draw();

        DrawRectangleRounded((Rectangle){_cursor_positions[_step].x+2.5f, _cursor_positions[_step].y+32, 25, 5}*BASE_UNIT, 0.5f, 8, PACIFICO_BROWN);

        Vector2 tempo_label_position = {_tempo_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "Tempo", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _tempo_knob->_position.y + _tempo_knob->_radius + 5.0f};
        DrawTextEx(PANEL_FONT, "Tempo", tempo_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 text_size = MeasureTextEx(PANEL_FONT, "trig", PANEL_FONT_SIZE, PANEL_FONT_SPACING);
        Vector2 trig_label_position = {_trig.gui._circle_position.x - _trig.gui._radius - text_size.x - 5, _trig.gui._circle_position.y - (text_size.y*0.5f)};
        DrawTextEx(PANEL_FONT, "trig", trig_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        text_size = MeasureTextEx(PANEL_FONT, "CV", PANEL_FONT_SIZE, PANEL_FONT_SPACING);
        Vector2 cv_label_position = {_cv.gui._circle_position.x - _cv.gui._radius - text_size.x - 5, _cv.gui._circle_position.y - (text_size.y*0.5f)};
        DrawTextEx(PANEL_FONT, "CV", cv_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);
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
    bool _trig_reset = false;

    Rectangle _module_box;

    knob* _tempo_knob;
    knob* _step_knobs[_num_steps];
    group* _group_box;
    toggle_switch* _step_switches[_num_steps];
    light*  _lights[_num_steps];
    light* _cv_light;
    light* _trig_light;
    parameter_map* _map;
    Vector2 _cursor_positions[_num_steps];
};

class mixer {
public:
    mixer() {}
    ~mixer() {}

    void init() {
        _gain_1= 0.2;
        _gain_2 = 0.2;

        _module_box = {580, 85, 90, 402.5f};

        _group_box = new group(_module_box, "Mixer");

        _in_1.gui.init("", {_module_box.x+45.0f-15+2.5f, _module_box.y+30.0f}, false);

        _in_2.gui.init("", {_module_box.x+45.0f-15+2.5f, _module_box.y+142.5f}, false);

        float radius = 22.5;
        _gain_1_knob = new knob({_module_box.x+45.0f+2.5f, _module_box.y+75.0f+radius}, radius, &_gain_1, 0.0, 0.5);
        _gain_2_knob = new knob({_module_box.x+45.0f+2.5f, _module_box.y+187.5f+radius}, radius, &_gain_2, 0.0, 0.5);

        _meter = new volume_meter({_module_box.x+20, _module_box.y+250, 55.0f, 140.0f});
    }

    float process() {
        float sample = (_in_1.val() * _gain_1) + (_in_2.val() * _gain_2);
        _meter->process(sample);
        return sample;
    }

    float draw() {
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        //GuiGroupBox(_module_box, "Mixer");
        _group_box->draw();

        _in_1.gui.draw();
        _in_2.gui.draw();
        _gain_1_knob->draw();
        _gain_2_knob->draw();
        _meter->draw();

        Vector2 in_label_position = {_in_1.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in 1", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in_1.gui._circle_position.y - _in_1.gui._radius - 12.5f};
        DrawTextEx(PANEL_FONT, "in 1", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        in_label_position = {_in_2.gui._circle_position.x - ((MeasureTextEx(PANEL_FONT, "in 2", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _in_2.gui._circle_position.y - _in_2.gui._radius - 12.5f};
        DrawTextEx(PANEL_FONT, "in 2", in_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        Vector2 gain_label_position = {_gain_1_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "Gain", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _gain_1_knob->_position.y - _gain_1_knob->_radius - 12.0f};
        DrawTextEx(PANEL_FONT, "Gain", gain_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);

        gain_label_position = {_gain_2_knob->_position.x - ((MeasureTextEx(PANEL_FONT, "Gain", PANEL_FONT_SIZE, PANEL_FONT_SPACING)).x*0.5f), _gain_2_knob->_position.y - _gain_2_knob->_radius - 12.0f};
        DrawTextEx(PANEL_FONT, "Gain", gain_label_position*BASE_UNIT, PANEL_FONT_SIZE*BASE_UNIT, PANEL_FONT_SPACING*BASE_UNIT, BLACK);
    }

    patch_destination _in_1;
    patch_destination _in_2;

private:
    float _gain_1;
    float _gain_2;

    Rectangle _module_box;

    knob* _gain_1_knob;
    knob* _gain_2_knob;

    group* _group_box;

    volume_meter* _meter;
};