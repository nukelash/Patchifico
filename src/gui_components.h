#pragma once

#include <math.h>

#include "raylib.h"
#include "raymath.h"

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
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(mouse_position, _position, _radius)) {
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

        DrawCircle(_position.x, _position.y, _radius, BLACK);

        Vector2 notch = {_radius-8.0f, 0};
        notch = Vector2Rotate(notch, _knob_angle * M_PI / 180.0f);
        notch = notch + _position;
        DrawCircleV(notch, 5, WHITE);

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