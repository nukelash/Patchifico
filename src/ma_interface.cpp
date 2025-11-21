#include "ma_interface.h"

ma_interface::ma_interface(void* user_data, void (*callback) (ma_device *, void *, const void *, unsigned int)){

    if (ma_context_init(NULL, 0, NULL, &(_context)) != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize MiniAudio context.");
    }

    ma_waveform_config config = ma_waveform_config_init(
                                ma_format_f32,
                                1,
                                44100,
                                ma_waveform_type_sine,
                                0.2,
                                440);

    ma_result result = ma_waveform_init(&config, &(_waveform));
    if (result != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize MiniAudio waveform.");
    }

    _device_config = ma_device_config_init(ma_device_type_duplex);
    _device_config.dataCallback      = callback;
    _device_config.playback.format   = ma_format_f32;
    _device_config.playback.channels = 1;
    _device_config.pUserData         = user_data;
    _device_config.capture.format    = ma_format_f32;
    _device_config.periodSizeInFrames= 64;

}

ma_interface::~ma_interface(){
    ma_context_uninit(&_context);
    ma_device_uninit(&_device);
}

void ma_interface::start(){

    
    _result = ma_device_init(NULL, &_device_config, &_device);
    if (_result != MA_SUCCESS) {
        throw std::runtime_error("Failed to open playback device.");
    }

    if (ma_device_start(&_device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&_device);
    }
}

void ma_interface::stop(){
    ma_device_stop(&_device);
    ma_device_uninit(&_device);
}

void ma_interface::set_device(ma_device_id* id) {
    _device_config.playback.pDeviceID = id;
}

void ma_interface::get_device_info(ma_device_info*** pinfo, ma_uint32* count) {

    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;

    ma_result result = ma_context_get_devices(&_context, *pinfo, count, &pCaptureInfos, &captureCount);
    if (result != MA_SUCCESS) {
            printf("error getting devices: %d", result);
            return;
        }
}

 std::string ma_interface::current_device_name() {
    ma_device_info* pPlaybackInfos;
    ma_uint32 playback_id;
    ma_device_info* pCaptureInfos;
    int capture_id;
    
    ma_uint32 playbackCount;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&_context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        printf("error getting devices");
        return std::string("bad!");
    }

    // Loop over each device info
    for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice += 1) {
        if (&pPlaybackInfos[iDevice].id == _device_config.playback.pDeviceID) {
            return std::string(pPlaybackInfos[iDevice].name);
        }

    }

    // We don't know the current device
    return std::string("Select Audio Device...");
}