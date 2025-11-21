#pragma once

#include <math.h>

#include "raylib.h"
#include "raymath.h"

#include "visual_config.h"

float BASE_UNIT = 1.0f;

class parameter_map {
public:
    parameter_map(float min, float mid, float max) {
        _b = min;
        _c = (2*mid - min - max)/(max - mid);
        _a = max - min + (_c*max);
    }

    float process(float input) {
        return (_a*input + _b) / (_c*input + 1);
    }

private:
    float _a, _b, _c;
};

class knob {
public:
    knob(Vector2 position, int radius, float* parameter, float min_val, float max_val) {
        _position = position;
        _radius = radius;
        _is_dragging = false;
        _knob_angle = 270;
        _last_knob_angle = _knob_angle;
        _knob_angle_change = 0;
        _parameter = parameter;

        _min_parameter_value = min_val;
        _max_parameter_value = max_val;
    }
    ~knob() {}

    void draw() {

        Vector2 mouse_position = GetMousePosition();

        // If we click a knob, set the flag and note the current mouse y position
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(mouse_position, _position*BASE_UNIT, _radius*BASE_UNIT)) {
            _is_dragging = true;
            _mouse_click_y = mouse_position.y;
        }

        // If the dragging flag is set and mouse is still held, update the change
        if(_is_dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            _knob_angle_change = _mouse_click_y - mouse_position.y;
        }

        // Whenever the mouse is released, we know we are no longer dragging and changes should reset
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            _last_knob_angle = _knob_angle;
            _knob_angle_change = 0;
            _is_dragging = false;
        }

        // Apply any change
        _knob_angle = _last_knob_angle + _knob_angle_change;

        // Limit to range
        if (_knob_angle < _min_angle) {
            _knob_angle = _min_angle;
        }
        if (_knob_angle > _max_angle) {
            _knob_angle = _max_angle;
        }

        DrawCircle(_position.x*BASE_UNIT, _position.y*BASE_UNIT, _radius*BASE_UNIT, PACIFICO_BLACK);

        Vector2 notch = {_radius-8.0f, 0};
        notch = Vector2Rotate(notch, _knob_angle * M_PI / 180.0f);
        notch = notch + _position;
        DrawCircleV(notch*BASE_UNIT, 5*BASE_UNIT, WHITE);

        *_parameter = (_knob_angle - _min_angle) / (_max_angle - _min_angle); //0-1
        *_parameter = (*_parameter * (_max_parameter_value - _min_parameter_value)) + _min_parameter_value; //min_val - max_val

    }

    Vector2 _position;
    int _radius;

private:

    bool _is_dragging;
    float _mouse_click_y;

    float _last_knob_angle;
    float _knob_angle;
    float _knob_angle_change;

    float _min_angle = 120;
    float _max_angle = 420;

    float _min_parameter_value;
    float _max_parameter_value;

    float* _parameter;
    // raylib convention is 3 o'clock is 0, 6 o'clock is 90, 9 is 180, etc.

    // so ideal knob range will go from like 120 to 60

};

class toggle_switch{
public:
    toggle_switch(Vector2 position, bool* parameter) {
        _position = position;
        _parameter = parameter;
    }
    ~toggle_switch() {}

    void draw() {
        //Color filled = PACIFICO_GOLD;

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), {_position.x*BASE_UNIT, _position.y*BASE_UNIT, _side_length*BASE_UNIT, _side_length*BASE_UNIT})) {
            _toggled = !_toggled;
            *_parameter = _toggled;
        }

        DrawRectangleRoundedLinesEx((Rectangle){_position.x*BASE_UNIT, _position.y*BASE_UNIT, (_side_length)*BASE_UNIT, (_side_length)*BASE_UNIT}, 0.2, 8, 2*BASE_UNIT, PACIFICO_BROWN);

        if (_toggled) {
            DrawRectangleRounded((Rectangle){_position.x*BASE_UNIT, _position.y*BASE_UNIT, _side_length*BASE_UNIT, _side_length*BASE_UNIT}, 0.2, 8, PACIFICO_BROWN);
        }
        
    }
private:
    Vector2 _position;
    float _side_length = 25;

    bool _toggled = 0;
    bool* _parameter;
};

class push_button{
public:
    push_button(Vector2 position, bool* parameter) {
        _position = position;
        _parameter = parameter;
    }
    ~push_button() {}

    void draw() {

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), {_position.x*BASE_UNIT, _position.y*BASE_UNIT, _side_length*BASE_UNIT, _side_length*BASE_UNIT})) {
            if(!_toggled) {
                *_parameter = true;
            }
            _toggled = true;
            counter = 0;

        }
        else {
            _toggled = false;
            *_parameter = false;
            if(counter < pushed_time_frames) {
                counter++;
            }
        }

        DrawRectangleRoundedLinesEx((Rectangle){_position.x*BASE_UNIT, _position.y*BASE_UNIT, (_side_length)*BASE_UNIT, (_side_length)*BASE_UNIT}, 0.2, 8, 2*BASE_UNIT, PACIFICO_BROWN);

        if (counter < pushed_time_frames) {
            DrawRectangleRounded((Rectangle){_position.x*BASE_UNIT, _position.y*BASE_UNIT, _side_length*BASE_UNIT, _side_length*BASE_UNIT}, 0.2, 8, PACIFICO_BROWN);
        }
        
    }
private:
    Vector2 _position;
    float _side_length = 19.5f;

    bool _toggled = 0;
    bool* _parameter;

    int pushed_time_frames = 7;
    int counter = 7;
};

class help_button{
public:
    help_button() {}
    ~help_button() {}

    void init(ma_interface* ma){
        _ma = ma;
        Vector2 text_size = MeasureTextEx(PANEL_FONT, "Help/Settings", 10, PANEL_TITLE_FONT_SPACING);
        float horizontal_buffer = (_help_button_module_box.width - text_size.x)/ 2.0f;
        float vertical_buffer =(_help_button_module_box.height - text_size.y)/2.0f;
        _text_position = {_help_button_module_box.x+horizontal_buffer, _help_button_module_box.y+vertical_buffer};

        _patch_image = LoadTexture("/Users/lukenash/Documents/Github/synth/instructions1.png");
        _knob_image = LoadTexture("/Users/lukenash/Documents/Github/synth/instructions2.png");

        _ma->get_device_info(&_p_device_info, &_device_count);

    }

    void draw() {

        float roundness = calculate_roundness(_help_button_module_box, 3);

        // == shadow ==
        DrawRectangleRounded((Rectangle){_help_button_module_box.x - 3, _help_button_module_box.y + 3, _help_button_module_box.width, _help_button_module_box.height}*BASE_UNIT, roundness, 8, BLACK);

        // == front ==
        DrawRectangleRounded(_help_button_module_box*BASE_UNIT, roundness, 8, WHITE);
        DrawRectangleRoundedLinesEx(_help_button_module_box*BASE_UNIT, roundness, 8, 1.5*BASE_UNIT, PACIFICO_BLACK);
        DrawTextEx(PANEL_FONT, "Help/Settings", _text_position*BASE_UNIT, 10*BASE_UNIT, PANEL_TITLE_FONT_SPACING*BASE_UNIT, PACIFICO_BLACK);

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), _help_button_module_box*BASE_UNIT)) {
            _menu_open = !_menu_open;
        }

        if (_menu_open) {
            draw_menu();
        }
         
    }

    void draw_menu() {
        
        // == background ==
        float top_bar_height = 20;
        float roundness = calculate_roundness(_menu_module_box, 5);
        DrawRectangleRounded(_menu_module_box*BASE_UNIT, roundness, 8, WHITE);
        Rectangle top_bar = {_menu_module_box.x, _menu_module_box.y, _menu_module_box.width, 40};
        Rectangle cover = {_menu_module_box.x, _menu_module_box.y + top_bar_height, _menu_module_box.width, 40};
        roundness = calculate_roundness(top_bar, 5);
        DrawRectangleRounded(top_bar, roundness, 8, PACIFICO_BROWN);
        DrawRectanglePro(cover, {0,0}, 0, WHITE);

        roundness = calculate_roundness(_menu_module_box, 5);
        DrawRectangleRoundedLinesEx(_menu_module_box*BASE_UNIT, roundness, 8, 1.5*BASE_UNIT, PACIFICO_BLACK);
        DrawLineEx({_menu_module_box.x, _menu_module_box.y+top_bar_height}, {_menu_module_box.x+_menu_module_box.width, _menu_module_box.y+top_bar_height}, 1.5*BASE_UNIT, PACIFICO_BLACK);


        Vector2 line_start, line_end;

        // == instructions ==
        line_start = {_menu_module_box.x + (_menu_module_box.width*0.5f), _menu_module_box.y + top_bar_height + 30};
        line_end = {_menu_module_box.x + (_menu_module_box.width*0.5f), line_start.y + 55};
        DrawLineEx(line_start, line_end, 3*BASE_UNIT, PACIFICO_BLACK);
        float image_scale = 0.3f;
        DrawTextureEx(_patch_image, {line_start.x - (_patch_image.width*image_scale) - 40, _menu_module_box.y + top_bar_height + 20}, 0, image_scale, WHITE);
        DrawTextureEx(_knob_image, {line_start.x + 40, _menu_module_box.y + top_bar_height + 20}, 0, image_scale, WHITE);

        Vector2 text_size = MeasureTextEx(PANEL_FONT, _instructions_line_1, 12, PANEL_FONT_SPACING);
        Vector2 text_position = {_menu_module_box.x + (_menu_module_box.width*0.5f) - (text_size.x*0.5f), _menu_module_box.y + 120};
        DrawTextEx(PANEL_FONT, _instructions_line_1, text_position, 12, PANEL_FONT_SPACING, PACIFICO_BLACK);

        text_size = MeasureTextEx(PANEL_FONT, _instructions_line_2, 12, PANEL_FONT_SPACING);
        text_position = {_menu_module_box.x + (_menu_module_box.width*0.5f) - (text_size.x*0.5f), _menu_module_box.y + 135};
        DrawTextEx(PANEL_FONT, _instructions_line_2, text_position, 12, PANEL_FONT_SPACING, PACIFICO_BLACK);


        // == divider line ==
        float buffer = 10;
        line_start = {_menu_module_box.x + buffer, _menu_module_box.y + (_menu_module_box.height * 0.75f)};
        line_end = {_menu_module_box.x + _menu_module_box.width - buffer, _menu_module_box.y + (_menu_module_box.height * 0.75f)};
        DrawLineEx(line_start*BASE_UNIT, line_end*BASE_UNIT, 3*BASE_UNIT, PACIFICO_BLACK);
        

        // == audio device selection ==
        DrawTextEx(PANEL_FONT,"Audio Device:", {line_start.x + buffer, line_start.y + 10}, 12, PANEL_FONT_SPACING, PACIFICO_BLACK);
        text_size = MeasureTextEx(PANEL_FONT,"Audio Device:", 12, PANEL_FONT_SPACING);
        Rectangle device_box = {line_start.x + buffer + text_size.x + 5, line_start.y + 10, line_end.x - (line_start.x + (2*buffer) + text_size.x + 5), text_size.y};
        DrawRectangleRounded(device_box, 0.1, 8, WHITE);
        DrawRectangleRoundedLinesEx(device_box, 0.1, 8, 1.5, PACIFICO_BLACK);
        DrawTextEx(PANEL_FONT, _ma->current_device_name().c_str(), {device_box.x+5, device_box.y-1}, 14, PANEL_FONT_SPACING, BLACK);

        if(_dropdown_open) {
            draw_dropdown(device_box);
        }

        Vector2 mouse_position = GetMousePosition();
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse_position, device_box)) {
            _dropdown_open = !_dropdown_open;
            _ma->get_device_info(&_p_device_info, &_device_count);
        }

        // == done button ==
        text_size = MeasureTextEx(PANEL_FONT, "Done", 12, PANEL_FONT_SPACING);
        float height = text_size.y + 2;
        float width = text_size.x + 6;
        Rectangle done_button = {_menu_module_box.x + (2*buffer), _menu_module_box.y + _menu_module_box.height - buffer - height, width, height};
        roundness = calculate_roundness(done_button, 3);
        DrawRectangleRounded(done_button, roundness, 8, PACIFICO_BROWN);
        DrawRectangleRoundedLinesEx(done_button, roundness, 8, 1.1f*BASE_UNIT, PACIFICO_BLACK);
        DrawTextEx(PANEL_FONT, "Done", {done_button.x + 3, done_button.y + 1}, 12, PANEL_FONT_SPACING, PACIFICO_BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse_position, done_button)) {
            _menu_open = false;
        }
    }

    void draw_dropdown(Rectangle device_box) {
        
        float line_thickness = 1.5;
        for(ma_uint32 i = 0; i < _device_count; i++) {
            device_box.y += device_box.height + line_thickness;
            DrawRectangleRec(device_box, WHITE);
            DrawRectangleRoundedLinesEx(device_box, 0.1, 8, line_thickness, PACIFICO_BLACK);
            DrawTextEx(PANEL_FONT, _device_info[i].name, {device_box.x+5, device_box.y}, 12, PANEL_FONT_SPACING, BLACK);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), device_box)) {
                _ma->stop();
                _ma->set_device(&_device_info[i].id);
                _ma->start();
            }
        }

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            _dropdown_open = false;
        }
    }

    bool _menu_open = 0; // will need to poll this before checking collisions on other modules
    bool _dropdown_open =  0;

private:

    float calculate_roundness(Rectangle rec, float radius) {
        // roundness in raylib changes based on rectangle dimensions, this will solve for that based on rounded corner pixel radius

        // Calculate corner radius
        //float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;

        float roundness = (rec.width > rec.height)? (2.0f*radius)/rec.height : (2.0f*radius)/rec.width;
        return roundness;
    }

    ma_interface* _ma;
    ma_device_info* _device_info;
    ma_device_info** _p_device_info = &_device_info;
    ma_uint32 _device_count;

    Rectangle _help_button_module_box = {595, 5, 75, 15};
    Rectangle _menu_module_box = {165, 120, 350, 210};
    Vector2 _text_position;

    Texture2D _patch_image;
    Texture2D _knob_image;

    // click and drag to place patch cables connecting inputs and outputs. Also click and drag to turn knobs.
    const char* _instructions_line_1 = "Click and drag to place patch cables connecting inputs ";
    const char* _instructions_line_2 = "and outputs. Also click and drag to turn knobs.";

    std::string _current_device_name;
    int _current_device_id;

};

bool DrawDropDown(Rectangle rect, bool* open, ma_device_info* device_info, int device_count) {

}

class group {
public:
    group(Rectangle border, std::string title) {
        //_border = border;
        init(border);
        _radius = 8;

        _label = true;
        
        _title = title;
        Vector2 title_size = MeasureTextEx(PANEL_FONT, title.c_str(), PANEL_TITLE_FONT_SIZE, PANEL_TITLE_FONT_SPACING);
        _title_width = title_size.x;
        float label_width = _title_width + 10;
        float label_pos_x = (_main_rec.width/2.0f) - (label_width/2.0f);
        _label_rectangle = {_main_rec.x+label_pos_x, _main_rec.y-5, label_width, 10};
    }

    group(Rectangle border) {
        _offset = {3, -3};
        init(border);
        _label = false;
        _radius = 2;
    }

    ~group() {}

    void init(Rectangle border) {
        _main_rec = {border.x + _offset.x, border.y, border.width - _offset.x, border.height + _offset.y};

        _shadow = {border.x, border.y - _offset.y, border.width - _offset.x, border.height + _offset.y};

    }

    void draw() {
        
        // == Shadow rectangle ==
        float roundness = calculate_roundness(_shadow, _radius);
        DrawRectangleRounded(_shadow*BASE_UNIT, roundness, 8, PACIFICO_BLACK);

        // == Main rectangle  ==
        roundness = calculate_roundness(_main_rec, _radius);
        DrawRectangleRounded(_main_rec*BASE_UNIT, roundness, 8, PACIFICO_GOLD);
        DrawRectangleRoundedLinesEx(_main_rec*BASE_UNIT, roundness, 8, 1.5*BASE_UNIT, PACIFICO_BLACK);

        // == Label ==
        if (_label) {
            roundness = calculate_roundness(_label_rectangle, 2);
            DrawRectangleRounded(_label_rectangle*BASE_UNIT, roundness, 8, WHITE);
            DrawRectangleRoundedLinesEx(_label_rectangle*BASE_UNIT, roundness, 8, 1.5*BASE_UNIT, PACIFICO_BLACK);
            DrawTextEx(PANEL_FONT, _title.c_str(), {(_label_rectangle.x + 5)*BASE_UNIT, (_label_rectangle.y-2.0f)*BASE_UNIT}, PANEL_TITLE_FONT_SIZE*BASE_UNIT, PANEL_TITLE_FONT_SPACING*BASE_UNIT, PACIFICO_BLACK); 
        }
        
        
    }

    Vector2 _offset = {5, -5}; //changes how the rectangles are positioned on top of each other

private:

    float calculate_roundness(Rectangle rec, float radius) {
        // roundness in raylib changes based on rectangle dimensions, this will solve for that based on rounded corner pixel radius

        // Calculate corner radius
        //float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;

        float roundness = (rec.width > rec.height)? (2.0f*radius)/rec.height : (2.0f*radius)/rec.width;
        return roundness;
    }

    Rectangle _shadow;
    Rectangle _main_rec;
    Rectangle _label_rectangle;
    std::string _title;
    float _title_width;
    float _radius;
    bool _label;
};

class light {
public:
    light(Vector2 position, float radius, Color color) {
        _position = position;
        _radius = radius;
        _color = color;
        _brightness = 0.0f;
    }
    ~light() {}

    void draw() {
        DrawCircleV(_position*BASE_UNIT, _radius*BASE_UNIT, PACIFICO_BLACK);
        Color light = _color;
        light.a *= _brightness;
        DrawCircleV(_position*BASE_UNIT, _radius*BASE_UNIT, light);
    }

    void set_brightness(float brightness) {
        if(brightness < 0.0f) {
            _brightness = 0.0f;
        } else if(brightness > 1.0f) {
            _brightness = 1.0f;
        } else {
            _brightness = brightness;
        }
    }

private:

    Color _color;
    float _radius;
    Vector2 _position;
    float _brightness;
};

class volume_meter {
public:
    volume_meter(Rectangle rectangle) {
        for (int i = 0;  i < _num_lights; i++) {
            float width = rectangle.width;
            float height = (rectangle.height / _num_lights) - _buffer;
            float x = rectangle.x;
            float y = rectangle.y + i * (height + _buffer);
            _lights[i] = {x, y, width, height};

            _map = new parameter_map(0, 11.8, 12);
        }
    }
    ~volume_meter() {}

    void process(float sample) {
        _average = (_f*_average) + ((1-_f)*abs(sample));
    }

    void draw() {
        Color c;
        int num_illuminated_lights = _map->process(_average);//_average * _num_lights;
        for (int i = _num_lights-1; i >= 0; i--) {
            c = PACIFICO_BLUE;
            if (i < 6) c = PACIFICO_GREEN;
            if (i < 3) c = PACIFICO_RED;

            if (i < (_num_lights - num_illuminated_lights)) {
                c.a *= 0.2;
            }
            
            DrawRectangleRounded(_lights[i]*BASE_UNIT, 0.5, 8, c);
        }
    }
private:
    static const int _num_lights = 12;
    Rectangle _lights[_num_lights];
    float _buffer = 5;

    float _average;
    float _f = 0.999;

    parameter_map* _map;

};

