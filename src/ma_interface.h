#pragma once

#include <stdio.h>
#include <stdexcept>
#include "miniaudio.h"

class ma_interface {
public:
    ma_interface(void* user_data, void (*callback) (ma_device *, void *, const void *, unsigned int));
    ~ma_interface();

    int start();
    int select_devices();

private:
    ma_context _context;
    ma_device_config _device_config;
    ma_device _device;
    ma_result _result;
    ma_waveform _waveform;
};
