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

    select_devices();
    
    _result = ma_device_init(NULL, &_device_config, &_device);
    if (_result != MA_SUCCESS) {
        throw std::runtime_error("Failed to open playback device.");
    }
}

ma_interface::~ma_interface(){
    ma_context_uninit(&_context);
    ma_device_uninit(&_device);
}

int ma_interface::start(){
    if (ma_device_start(&_device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&_device);
        return 1;
    }
    return 0;
}

int ma_interface::select_devices() {

    ma_device_info* pPlaybackInfos;
    ma_uint32 playback_id;
    ma_device_info* pCaptureInfos;
    int capture_id;
    
    ma_uint32 playbackCount;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&_context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        printf("error getting devices");
        return 1;
    }

    // Loop over each device info
    for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice += 1) {
        printf("%d - %s\n", iDevice, pPlaybackInfos[iDevice].name);
    }

    printf("Select playback device: ");
    playback_id = getchar() - '0';
    while (getchar() != '\n')
        continue;

    for (ma_uint32 iDevice = 0; iDevice < captureCount; iDevice += 1) {
        printf("%d - %s\n", iDevice, pCaptureInfos[iDevice].name);
    }

    printf("Select capture device: ");
    capture_id = getchar() - '0';
    while (getchar() != '\n')
        continue;
    printf("%d %d\n", playback_id, capture_id);

    _device_config.playback.pDeviceID = &pPlaybackInfos[playback_id].id;

    //ma_context_uninit(&_context);
    
    return 0;
}

void ma_interface::get_device_info(ma_device_info*** pinfo, ma_uint32* count) {

    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;

    ma_result result = ma_context_get_devices(&_context, *pinfo, count, &pCaptureInfos, &captureCount);
    if (result != MA_SUCCESS) {
            printf("error getting devices: %d", result);
            return;
        }
    printf("success\n");
    printf("%s\n", (*pinfo)[0]->name);
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
        printf("%d - %s\n", iDevice, pPlaybackInfos[iDevice].name);
        if (&pPlaybackInfos[iDevice].id == _device_config.playback.pDeviceID) {
            printf("returning %s", pPlaybackInfos[iDevice].name);
            return std::string(pPlaybackInfos[iDevice].name);
        }

    }
    printf("didn't find current device\n");
}