
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
        _connections.erase(dest_name);
    }

private:
    std::unordered_map<std::string, patch_source*> _sources;
    std::unordered_map<std::string, patch_destination*> _destinations;

    std::unordered_map<std::string, connection*> _connections;
};

class oscillator {
public:
    oscillator() {}
    ~oscillator() {}

    void init(float sample_rate, patch_manager* patch_bay) {
        _patch_bay = patch_bay;
        
        _osc.Init(sample_rate);
        _frequency = 100;
        _osc.SetFreq(_frequency);

        _out_counter = 0;

        _module_box.x = 20;
        _module_box.y = 20;
        _module_box.width = 150;
        _module_box.height = 300;

         _out.gui.init("output", {_module_box.x+5, _module_box.y+80});

        
    }

    void draw() {
        
        int y_pad = 30;
        float y_index = _module_box.y + y_pad;

        int x_pad = 5;
        float x_index = _module_box.x + x_pad;
        GuiGroupBox(_module_box, "VCO");

        if (GuiSlider((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Frequency", "", &_frequency, 20, 500)) {
            _osc.SetFreq(_frequency);
        }
        y_index += y_pad;

        if (GuiComboBox((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Sine;Tri;Saw;Ramp;Square", &_wavetype)) {
            _osc.SetWaveform(_wavetype);
        }
        y_index += y_pad;

        if (GuiComboBox((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Filter;Mixer", &_out_counter)) {
            if (_out_counter == 0) {
                _patch_bay->connect("my_osc_out", "my_filt_in");
                _patch_bay->connect("my_filt_out", "my_mixer_in");
            }
            else {
                std::cout << _out_counter << std::endl;
                _patch_bay->disconnect("my_filt_in");
                _patch_bay->connect("my_osc_out", "my_mixer_in");
            }
        }

        _out.gui.draw();
    }

    float process() {
        _out.value = _osc.Process();
    }

    // process calls the daisy process function, THEN sets patch output variables accordingly;

    //callback will call each module process one at a time

    //draw will draw the GUI AND update module params accordingly
    patch_source _out;
private:

    daisysp::Oscillator _osc;
    float _frequency;
    int _wavetype;

    int _out_counter;
    

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

class mixer {
public:
    mixer() {}
    ~mixer() {}

    void init() {
        _gain_1= 0.5;
        _gain_2 = 0.5;

        _module_box = {400, 20, 150, 300};

        _in_1.gui.init("Audio 1", {_module_box.x+5, _module_box.y+200});

        _in_2.gui.init("Audio 2", {_module_box.x+60, _module_box.y+200});
    }

    float process() {
        float sample = (_in_1.val() * _gain_1) + (0.0f * _gain_2);
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