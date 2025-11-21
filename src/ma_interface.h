#pragma once

#include <stdio.h>
#include <string>
#include <stdexcept>
#include "miniaudio.h"

class ma_interface {
public:
    ma_interface(void* user_data, void (*callback) (ma_device *, void *, const void *, unsigned int));
    ~ma_interface();

    void start();
    void stop();
    void set_device(ma_device_id* id);
    void get_device_info(ma_device_info*** info, ma_uint32* count);
    std::string current_device_name();

private:
    ma_context _context;
    ma_device_config _device_config;
    ma_device _device;
    ma_result _result;
    ma_waveform _waveform;
};
