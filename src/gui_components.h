#pragma once

#include <math.h>

#include "raylib.h"
#include "raymath.h"

#define PACIFICO_GOLD CLITERAL(Color){254, 224, 33, 255}
#define PACIFICO_BROWN CLITERAL(Color){182, 126, 12, 255}
#define PACIFICO_RED CLITERAL(Color){248, 48, 23, 255}
#define PACIFICO_BLUE CLITERAL(Color){105, 175, 249, 255}
#define PACIFICO_GREEN CLITERAL(Color){73, 165, 11, 255}

float BASE_UNIT = 1.0f;

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

        DrawCircle(_position.x*BASE_UNIT, _position.y*BASE_UNIT, _radius*BASE_UNIT, BLACK);

        Vector2 notch = {_radius-8.0f, 0};
        notch = Vector2Rotate(notch, _knob_angle * M_PI / 180.0f);
        notch = notch + _position;
        DrawCircleV(notch*BASE_UNIT, 5*BASE_UNIT, WHITE);

        *_parameter = (_knob_angle - _min_angle) / (_max_angle - _min_angle); //0-1
        *_parameter = (*_parameter * (_max_parameter_value - _min_parameter_value)) + _min_parameter_value; //min_val - max_val

    }

private:
    Vector2 _position;
    int _radius;

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
    float _side_length = 30;

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
    float _side_length = 30;

    bool _toggled = 0;
    bool* _parameter;

    int pushed_time_frames = 7;
    int counter = 7;
};

class group {
public:
    group(Rectangle border, std::string title) {
        _border = border;
        float label_pos_x = (border.width/2.0f) - (((title.length()*10)+8)/2.0f);
        _label_rectangle = {_border.x+label_pos_x, _border.y-5, title.length()*10.0f+4.0f, 10};
        // _vertices[0] = {border.x, border.y};
        // _vertices[1] = {border.x+border.width, border.y};
        // _vertices[2] = {border.x+border.width, border.y+border.height};
        // _vertices[3] = {border.x, border.y+border.height};
        _title = title;
    }
    ~group() {}

    void draw() {
        
        DrawRectangleRounded(_border*BASE_UNIT, 0.3, 8, PACIFICO_GOLD);
        DrawRectangleRoundedLinesEx(_border*BASE_UNIT, 0.3, 8, 1.5, BLACK);
        DrawRectangleRec(_label_rectangle*BASE_UNIT, BLACK);
        
        // for(int i = 0; i < 4; i++) {
        //     int next = (i+1) % 4;
        //     DrawLineV(_vertices[i], _vertices[next], BLACK);
        // }

    
        
    }

private:

    Rectangle _border;
    Rectangle _label_rectangle;
    std::string _title;
};

