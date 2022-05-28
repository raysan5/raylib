/******************************************************************************

SDL Backend

******************************************************************************/

struct
{
    ma_uint32 deviceID;
} device_sdl;

struct
{
    ma_handle hSDL;
    ma_proc SDL_InitSubSystem;
    ma_proc SDL_QuitSubSystem;
    ma_proc SDL_GetNumAudioDevices;
    ma_proc SDL_GetAudioDeviceName;
    ma_proc SDL_CloseAudioDevice;
    ma_proc SDL_OpenAudioDevice;
    ma_proc SDL_PauseAudioDevice;
} context_sdl;


#define MA_SDL_INIT_AUDIO                      0x00000010
#define MA_AUDIO_U8                            0x0008
#define MA_AUDIO_S16                           0x8010
#define MA_AUDIO_S32                           0x8020
#define MA_AUDIO_F32                           0x8120
#define MA_SDL_AUDIO_ALLOW_FREQUENCY_CHANGE    0x00000001
#define MA_SDL_AUDIO_ALLOW_FORMAT_CHANGE       0x00000002
#define MA_SDL_AUDIO_ALLOW_CHANNELS_CHANGE     0x00000004
#define MA_SDL_AUDIO_ALLOW_ANY_CHANGE          (MA_SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | MA_SDL_AUDIO_ALLOW_FORMAT_CHANGE | MA_SDL_AUDIO_ALLOW_CHANNELS_CHANGE)

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

typedef int                   (* MA_PFN_SDL_InitSubSystem)(ma_uint32 flags);
typedef void                  (* MA_PFN_SDL_QuitSubSystem)(ma_uint32 flags);
typedef int                   (* MA_PFN_SDL_GetNumAudioDevices)(int iscapture);
typedef const char*           (* MA_PFN_SDL_GetAudioDeviceName)(int index, int iscapture);
typedef void                  (* MA_PFN_SDL_CloseAudioDevice)(SDL_AudioDeviceID dev);
typedef SDL_AudioDeviceID     (* MA_PFN_SDL_OpenAudioDevice)(const char* device, int iscapture, const SDL_AudioSpec* desired, SDL_AudioSpec* obtained, int allowed_changes);
typedef void                  (* MA_PFN_SDL_PauseAudioDevice)(SDL_AudioDeviceID dev, int pause_on);

static SDL_AudioFormat ma_format_to_sdl(ma_format format)
{
    switch (format)
    {
        case ma_format_unknown: return 0;
        case ma_format_u8:      return MA_AUDIO_U8;
        case ma_format_s16:     return MA_AUDIO_S16;
        case ma_format_s24:     return MA_AUDIO_S32;  // Closest match.
        case ma_format_s32:     return MA_AUDIO_S32;
        case ma_format_f32:     return MA_AUDIO_F32;
        default:                return 0;
    }
}

static ma_format ma_format_from_sdl(SDL_AudioFormat format)
{
    switch (format)
    {
        case MA_AUDIO_U8:  return ma_format_u8;
        case MA_AUDIO_S16: return ma_format_s16;
        case MA_AUDIO_S32: return ma_format_s32;
        case MA_AUDIO_F32: return ma_format_f32;
        default:           return ma_format_unknown;
    }
}

static ma_result ma_context_get_device_info__sdl(ma_context* pContext, ma_device_type deviceType, const ma_device_id* pDeviceID, ma_device_info* pDeviceInfo)
{
    MA_ASSERT(pContext != NULL);
    MA_ASSERT(deviceType == ma_device_type_playback);

    pDeviceInfo->isDefault = MA_TRUE;

    SDL_AudioSpec desiredSpec, obtainedSpec;
    MA_ZERO_OBJECT(&desiredSpec);

    SDL_AudioDeviceID tempDeviceID = ((MA_PFN_SDL_OpenAudioDevice)context_sdl.SDL_OpenAudioDevice)(NULL, 0, &desiredSpec, &obtainedSpec, MA_SDL_AUDIO_ALLOW_ANY_CHANGE);
    ((MA_PFN_SDL_CloseAudioDevice)context_sdl.SDL_CloseAudioDevice)(tempDeviceID);

    ma_format format = ma_format_from_sdl(obtainedSpec.format);
    if (format == ma_format_unknown) {
        format = ma_format_f32;
    }

    pDeviceInfo->nativeDataFormatCount = 0;
    ma_device_info_add_native_data_format(pDeviceInfo, format, obtainedSpec.channels, obtainedSpec.freq, 0);

    return MA_SUCCESS;
}


static ma_result ma_device_uninit__sdl(ma_device* pDevice)
{
    MA_ASSERT(pDevice != NULL);

    ((MA_PFN_SDL_CloseAudioDevice)context_sdl.SDL_CloseAudioDevice)(device_sdl.deviceID);

    return MA_SUCCESS;
}


static void ma_audio_callback__sdl(void* pUserData, ma_uint8* pBuffer, int bufferSizeInBytes)
{
    ma_device* pDevice = (ma_device*)pUserData;
    MA_ASSERT(pDevice != NULL);

    ma_uint32 bytesPerFrame = ma_get_bytes_per_frame(pDevice->playback.internalFormat, pDevice->playback.internalChannels);
    ma_uint32 frameCount = (ma_uint32)bufferSizeInBytes / bytesPerFrame;

    ma_device_handle_backend_data_callback(pDevice, pBuffer, NULL, frameCount);
}

static ma_result ma_device_init__sdl(ma_device* pDevice, const ma_device_config* pConfig, ma_device_descriptor* pDescriptorPlayback, ma_device_descriptor* pDescriptorCapture)
{
    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pConfig != NULL);
    MA_ASSERT(pConfig->deviceType == ma_device_type_playback);

    if (pDescriptorPlayback->sampleRate == 0) {
        pDescriptorPlayback->sampleRate = MA_DEFAULT_SAMPLE_RATE;
    }

    pDescriptorPlayback->periodSizeInFrames = pDescriptorPlayback->periodCount * ma_calculate_buffer_size_in_frames_from_descriptor(pDescriptorPlayback, pDescriptorPlayback->sampleRate, pConfig->performanceProfile);

    /* SDL wants the buffer size to be a power of 2 for some reason. */
    if (pDescriptorPlayback->periodSizeInFrames > 32768) {
        pDescriptorPlayback->periodSizeInFrames = 32768;
    } else {
        pDescriptorPlayback->periodSizeInFrames = ma_next_power_of_2(pDescriptorPlayback->periodSizeInFrames);
    }

    MA_ASSERT(pDescriptorPlayback->periodSizeInFrames <= 32768);

    /* We now have enough information to set up the device. */
    SDL_AudioSpec desiredSpec, obtainedSpec;
    MA_ZERO_OBJECT(&desiredSpec);
    desiredSpec.freq     = (int)pDescriptorPlayback->sampleRate;
    desiredSpec.format   = ma_format_to_sdl(pDescriptorPlayback->format);
    desiredSpec.channels = (ma_uint8)pDescriptorPlayback->channels;
    desiredSpec.samples  = (ma_uint16)pDescriptorPlayback->periodSizeInFrames;
    desiredSpec.callback = ma_audio_callback__sdl;
    desiredSpec.userdata = pDevice;

    /* We'll fall back to f32 if we don't have an appropriate mapping between SDL and miniaudio. */
    if (desiredSpec.format == ma_format_unknown) {
        desiredSpec.format = MA_AUDIO_F32;
    }

    device_sdl.deviceID = ((MA_PFN_SDL_OpenAudioDevice)context_sdl.SDL_OpenAudioDevice)(NULL, 0, &desiredSpec, &obtainedSpec, MA_SDL_AUDIO_ALLOW_ANY_CHANGE);

    /* The descriptor needs to be updated with our actual settings. */
    pDescriptorPlayback->format             = ma_format_from_sdl(obtainedSpec.format);
    pDescriptorPlayback->channels           = obtainedSpec.channels;
    pDescriptorPlayback->sampleRate         = (ma_uint32)obtainedSpec.freq;
    ma_channel_map_init_standard(ma_standard_channel_map_default, pDescriptorPlayback->channelMap, ma_countof(pDescriptorPlayback->channelMap), pDescriptorPlayback->channels);
    pDescriptorPlayback->periodSizeInFrames = obtainedSpec.samples;
    pDescriptorPlayback->periodCount        = 1;    /* SDL doesn't use the notion of period counts, so just set to 1. */

    return MA_SUCCESS;
}

static ma_result ma_device_start__sdl(ma_device* pDevice)
{
    MA_ASSERT(pDevice != NULL);

    ((MA_PFN_SDL_PauseAudioDevice)context_sdl.SDL_PauseAudioDevice)(device_sdl.deviceID, 0);

    return MA_SUCCESS;
}

static ma_result ma_device_stop__sdl(ma_device* pDevice)
{
    MA_ASSERT(pDevice != NULL);

    ((MA_PFN_SDL_PauseAudioDevice)context_sdl.SDL_PauseAudioDevice)(device_sdl.deviceID, 1);

    ma_device__set_state(pDevice, ma_device_state_stopped);
    ma_stop_proc onStop = pDevice->onStop;
    if (onStop) {
        onStop(pDevice);
    }

    return MA_SUCCESS;
}


static ma_result ma_context_uninit__sdl(ma_context* pContext)
{
    MA_ASSERT(pContext != NULL);

    ((MA_PFN_SDL_QuitSubSystem)context_sdl.SDL_QuitSubSystem)(MA_SDL_INIT_AUDIO);

    return MA_SUCCESS;
}

static ma_result ma_context_init__sdl(ma_context* pContext, const ma_context_config* pConfig, ma_backend_callbacks* pCallbacks)
{
    MA_ASSERT(pContext != NULL);

    (void)pConfig; /* Unused. */

    context_sdl.SDL_InitSubSystem      = (ma_proc)SDL_InitSubSystem;
    context_sdl.SDL_QuitSubSystem      = (ma_proc)SDL_QuitSubSystem;
    context_sdl.SDL_GetNumAudioDevices = (ma_proc)SDL_GetNumAudioDevices;
    context_sdl.SDL_GetAudioDeviceName = (ma_proc)SDL_GetAudioDeviceName;
    context_sdl.SDL_CloseAudioDevice   = (ma_proc)SDL_CloseAudioDevice;
    context_sdl.SDL_OpenAudioDevice    = (ma_proc)SDL_OpenAudioDevice;
    context_sdl.SDL_PauseAudioDevice   = (ma_proc)SDL_PauseAudioDevice;

    int resultSDL = ((MA_PFN_SDL_InitSubSystem)context_sdl.SDL_InitSubSystem)(MA_SDL_INIT_AUDIO);
    if (resultSDL != 0) {
        return MA_ERROR;
    }

    pCallbacks->onContextInit             = ma_context_init__sdl;
    pCallbacks->onContextUninit           = ma_context_uninit__sdl;
    pCallbacks->onContextEnumerateDevices = NULL;
    pCallbacks->onContextGetDeviceInfo    = ma_context_get_device_info__sdl;
    pCallbacks->onDeviceInit              = ma_device_init__sdl;
    pCallbacks->onDeviceUninit            = ma_device_uninit__sdl;
    pCallbacks->onDeviceStart             = ma_device_start__sdl;
    pCallbacks->onDeviceStop              = ma_device_stop__sdl;
    pCallbacks->onDeviceRead              = NULL;
    pCallbacks->onDeviceWrite             = NULL;
    pCallbacks->onDeviceDataLoop          = NULL;

    return MA_SUCCESS;
}
