
//modules: oscillator, lfo, vca, env, filter, sequencer(?)

#include "daisysp.h"
#include "raygui.h"

#include <unordered_map>

typedef float patch_source;

struct patch_destination {
    float* source = nullptr;
    bool connected = false;

    float val() {
        if (connected) {
            return *source;
        }
        else {
            return 0.0f;
        }
    }
};

// class patch_bay {
// public:
//     patch_bay() {}
//     ~patch_bay() {}

//     void add_input(std::string key, patch_input* in) {
//         _inputs.insert({key, in});
//     }

//     patch_input* get_input(std::string key) {
//         return _inputs[key];
//     }

// private:
//     std::unordered_map<std::string, patch_input*> _inputs;
// };

//typedef std::unordered_map<std::string, patch_input*> patch_destinations;

class patch_manager {
public:
    patch_manager() {}
    ~patch_manager() {}

    void add(std::string name, patch_source* source) {
        _sources.insert({name, source});
    }

    void add(std::string name, patch_destination* dest) {
        _destinations.insert({name, dest});
    }

    void connect(std::string source_name, std::string dest_name) {
        _destinations[dest_name]->source = _sources[source_name];

        _destinations[dest_name]->connected = true;
    }

    void disconnect(std::string dest_name) {
        _destinations[dest_name]->source = nullptr;
        _destinations[dest_name]->connected = false;
    }

private:
    std::unordered_map<std::string, patch_source*> _sources;
    std::unordered_map<std::string, patch_destination*> _destinations;
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

        _module_box.x = 20;
        _module_box.y = 20;
        _module_box.width = 150;
        _module_box.height = 300;

        
    }

    // void connect() {
    //     (*_patch_destinations)["filter_in"]->connect(_out);
    // }

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

        // if (GuiComboBox((Rectangle) {x_index, y_index, _module_box.width - (2*x_pad), 20}, "Filter;Mixer", &_out_counter)) {
        //     if (_out_counter = 0) {
        //         (*_patch_destinations)["filter_in"]->connect(_out);
        //     }
        //     else {
        //         (*_patch_destinations)["mixer_in_1"]->connect(_out);
        //     }
        // }
    }

    float process() {
        _out = _osc.Process();
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

    }

    // void connect() {
    //     (*_patch_destinations)["mixer_in_1"]->connect(_out);
    // }

    float process() {

        _filter.Process(_in.val());

        _out = _filter.Low();
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
    }

    patch_destination _in;
    patch_source _out;

private:
    daisysp::Svf _filter;
    float _frequency;
    float _resonance;

    

    //patch_destinations* _patch_destinations;

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
    }

    patch_destination _in_1;

private:
    float _gain_1;
    float _gain_2;

    Rectangle _module_box;
};