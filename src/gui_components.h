#include "raylib.h"

class knob {
public:
    knob() {
        _position = {100, 100};
        _radius = 60;
        _drag_in_progress = false;
        _knob_angle = 180;
        _knob_augmentation = 0;
    }
    ~knob() {}

    void draw() {

        Vector2 mouse_location = GetMousePosition();

        if(!_drag_in_progress && (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(mouse_location, _position, _radius))) {
            _drag_in_progress = true;
            _mouse_click_y = mouse_location.y;
        }

        if(_drag_in_progress && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            _knob_angle = _knob_angle + _knob_augmentation;
            _knob_augmentation = 0;
        }

        if(_drag_in_progress && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            _knob_augmentation = _mouse_click_y - mouse_location.y;
        }
        else {
            _drag_in_progress = false;
            _knob_augmentation = 0;
        }
        /*
        get mouse position

        2 states, drag in process or not
        if not &  mouse collides with knob & mouse down: 
            drag in process

        if drag in process & mouse down:
            compare mouse position to last mouse position:
                change knob angle based on change in Y of mouse position
        else
            drag not in process
        */


        DrawCircle(_position.x, _position.y, _radius, RED);

        int segment = 5;
        float true_knob_angle = _knob_angle + _knob_augmentation;

        //limit to range
        if (true_knob_angle < 120.0) {
            true_knob_angle = 120.0;
        }
        if (true_knob_angle > 420.0) {
            true_knob_angle = 420.0;
        }

        DrawCircleSector(_position, 40, true_knob_angle-segment, true_knob_angle+segment, 2, BLACK);

        //_last_click_y = mouse_location.y;
    }

private:
    Vector2 _position;
    int _radius;

    bool _drag_in_progress;
    float _mouse_click_y;

    float _knob_angle;
    float _knob_augmentation;
    // raylib convention is 3 o'clock is 0, 6 o'clock is 90, 9 is 180, etc.

    // so ideal knob range will go from like 120 to 60

};