#include "miniaudio.h"

/* Support SDL on everything. */
#define MA_SUPPORT_SDL

/*
Only enable SDL if it's hasn't been explicitly disabled (MA_NO_SDL) or enabled (MA_ENABLE_SDL with
MA_ENABLE_ONLY_SPECIFIC_BACKENDS) and it's supported at compile time (MA_SUPPORT_SDL).
*/
#if defined(MA_SUPPORT_SDL) && !defined(MA_NO_SDL) && (!defined(MA_ENABLE_ONLY_SPECIFIC_BACKENDS) || defined(MA_ENABLE_SDL))
    #define MA_HAS_SDL
#endif


typedef struct
{
    ma_context context; /* Make this the first member so we can cast between ma_context and ma_context_ex. */
#if defined(MA_SUPPORT_SDL)
    struct
    {
        ma_proc SDL_InitSubSystem;
        ma_proc SDL_QuitSubSystem;
        ma_proc SDL_GetNumAudioDevices;
        ma_proc SDL_GetAudioDeviceName;
        ma_proc SDL_CloseAudioDevice;
        ma_proc SDL_OpenAudioDevice;
        ma_proc SDL_PauseAudioDevice;
    } sdl;
#endif
} ma_context_ex;

typedef struct
{
    ma_device device;   /* Make this the first member so we can cast between ma_device and ma_device_ex. */
#if defined(MA_SUPPORT_SDL)
    struct
    {
        int deviceIDPlayback;
        int deviceIDCapture;
    } sdl;
#endif
} ma_device_ex;


#if defined(MA_HAS_SDL)

#define MA_SDL_INIT_AUDIO                       0x00000010
#define MA_AUDIO_U8                             0x0008
#define MA_AUDIO_S16                            0x8010
#define MA_AUDIO_S32                            0x8020
#define MA_AUDIO_F32                            0x8120
#define MA_SDL_AUDIO_ALLOW_FREQUENCY_CHANGE     0x00000001
#define MA_SDL_AUDIO_ALLOW_FORMAT_CHANGE        0x00000002
#define MA_SDL_AUDIO_ALLOW_CHANNELS_CHANGE      0x00000004
#define MA_SDL_AUDIO_ALLOW_ANY_CHANGE           (MA_SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | MA_SDL_AUDIO_ALLOW_FORMAT_CHANGE | MA_SDL_AUDIO_ALLOW_CHANNELS_CHANGE)

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

typedef SDL_AudioCallback   MA_SDL_AudioCallback;
typedef SDL_AudioSpec       MA_SDL_AudioSpec;
typedef SDL_AudioFormat     MA_SDL_AudioFormat;
typedef SDL_AudioDeviceID   MA_SDL_AudioDeviceID;


typedef int                  (* MA_PFN_SDL_InitSubSystem)(ma_uint32 flags);
typedef void                 (* MA_PFN_SDL_QuitSubSystem)(ma_uint32 flags);
typedef int                  (* MA_PFN_SDL_GetNumAudioDevices)(int iscapture);
typedef const char*          (* MA_PFN_SDL_GetAudioDeviceName)(int index, int iscapture);
typedef void                 (* MA_PFN_SDL_CloseAudioDevice)(MA_SDL_AudioDeviceID dev);
typedef MA_SDL_AudioDeviceID (* MA_PFN_SDL_OpenAudioDevice)(const char* device, int iscapture, const MA_SDL_AudioSpec* desired, MA_SDL_AudioSpec* obtained, int allowed_changes);
typedef void                 (* MA_PFN_SDL_PauseAudioDevice)(MA_SDL_AudioDeviceID dev, int pause_on);

MA_SDL_AudioFormat ma_format_to_sdl(ma_format format)
{
    switch (format)
    {
    case ma_format_unknown: return 0;
    case ma_format_u8:      return MA_AUDIO_U8;
    case ma_format_s16:     return MA_AUDIO_S16;
    case ma_format_s24:     return MA_AUDIO_S32;  /* Closest match. */
    case ma_format_s32:     return MA_AUDIO_S32;
    case ma_format_f32:     return MA_AUDIO_F32;
    default:                return 0;
    }
}

ma_format ma_format_from_sdl(MA_SDL_AudioFormat format)
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

static ma_result ma_context_enumerate_devices__sdl(ma_context* pContext, ma_enum_devices_callback_proc callback, void* pUserData)
{
    ma_context_ex* pContextEx = (ma_context_ex*)pContext;
    ma_bool32 isTerminated = MA_FALSE;
    ma_bool32 cbResult;
    int iDevice;

    MA_ASSERT(pContext != NULL);
    MA_ASSERT(callback != NULL);

    /* Playback */
    if (!isTerminated) {
        int deviceCount = ((MA_PFN_SDL_GetNumAudioDevices)pContextEx->sdl.SDL_GetNumAudioDevices)(0);
        for (iDevice = 0; iDevice < deviceCount; ++iDevice) {
            ma_device_info deviceInfo;
            MA_ZERO_OBJECT(&deviceInfo);

            deviceInfo.id.custom.i = iDevice;
            ma_strncpy_s(deviceInfo.name, sizeof(deviceInfo.name), ((MA_PFN_SDL_GetAudioDeviceName)pContextEx->sdl.SDL_GetAudioDeviceName)(iDevice, 0), (size_t)-1);

            if (iDevice == 0) {
                deviceInfo.isDefault = MA_TRUE;
            }

            cbResult = callback(pContext, ma_device_type_playback, &deviceInfo, pUserData);
            if (cbResult == MA_FALSE) {
                isTerminated = MA_TRUE;
                break;
            }
        }
    }

    /* Capture */
    if (!isTerminated) {
        int deviceCount = ((MA_PFN_SDL_GetNumAudioDevices)pContextEx->sdl.SDL_GetNumAudioDevices)(1);
        for (iDevice = 0; iDevice < deviceCount; ++iDevice) {
            ma_device_info deviceInfo;
            MA_ZERO_OBJECT(&deviceInfo);

            deviceInfo.id.custom.i = iDevice;
            ma_strncpy_s(deviceInfo.name, sizeof(deviceInfo.name), ((MA_PFN_SDL_GetAudioDeviceName)pContextEx->sdl.SDL_GetAudioDeviceName)(iDevice, 1), (size_t)-1);

            if (iDevice == 0) {
                deviceInfo.isDefault = MA_TRUE;
            }

            cbResult = callback(pContext, ma_device_type_capture, &deviceInfo, pUserData);
            if (cbResult == MA_FALSE) {
                isTerminated = MA_TRUE;
                break;
            }
        }
    }

    return MA_SUCCESS;
}

static ma_result ma_context_get_device_info__sdl(ma_context* pContext, ma_device_type deviceType, const ma_device_id* pDeviceID, ma_device_info* pDeviceInfo)
{
    ma_context_ex* pContextEx = (ma_context_ex*)pContext;

    MA_SDL_AudioSpec desiredSpec;
    MA_SDL_AudioSpec obtainedSpec;
    MA_SDL_AudioDeviceID tempDeviceID;
    const char* pDeviceName;

    MA_ASSERT(pContext != NULL);

    if (pDeviceID == NULL) {
        if (deviceType == ma_device_type_playback) {
            pDeviceInfo->id.custom.i = 0;
            ma_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), MA_DEFAULT_PLAYBACK_DEVICE_NAME, (size_t)-1);
        } else {
            pDeviceInfo->id.custom.i = 0;
            ma_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), MA_DEFAULT_CAPTURE_DEVICE_NAME, (size_t)-1);
        }
    } else {
        pDeviceInfo->id.custom.i = pDeviceID->custom.i;
        ma_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), ((MA_PFN_SDL_GetAudioDeviceName)pContextEx->sdl.SDL_GetAudioDeviceName)(pDeviceID->custom.i, (deviceType == ma_device_type_playback) ? 0 : 1), (size_t)-1);
    }

    if (pDeviceInfo->id.custom.i == 0) {
        pDeviceInfo->isDefault = MA_TRUE;
    }

    /*
    To get an accurate idea on the backend's native format we need to open the device. Not ideal, but it's the only way. An
    alternative to this is to report all channel counts, sample rates and formats, but that doesn't offer a good representation
    of the device's _actual_ ideal format.

    */

    MA_ZERO_MEMORY(&desiredSpec, sizeof(desiredSpec));

    pDeviceName = NULL;
    if (pDeviceID != NULL) {
        pDeviceName = ((MA_PFN_SDL_GetAudioDeviceName)pContextEx->sdl.SDL_GetAudioDeviceName)(pDeviceID->custom.i, (deviceType == ma_device_type_playback) ? 0 : 1);
    }

    tempDeviceID = ((MA_PFN_SDL_OpenAudioDevice)pContextEx->sdl.SDL_OpenAudioDevice)(pDeviceName, (deviceType == ma_device_type_playback) ? 0 : 1, &desiredSpec, &obtainedSpec, MA_SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (tempDeviceID == 0) {
        ma_log_postf(ma_context_get_log(pContext), MA_LOG_LEVEL_ERROR, "Failed to open SDL device.");
        return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
    }

    ((MA_PFN_SDL_CloseAudioDevice)pContextEx->sdl.SDL_CloseAudioDevice)(tempDeviceID);

    /* Only reporting a single native data format. It'll be whatever SDL decides is the best. */
    pDeviceInfo->nativeDataFormatCount = 1;
    pDeviceInfo->nativeDataFormats[0].format     = ma_format_from_sdl(obtainedSpec.format);
    pDeviceInfo->nativeDataFormats[0].channels   = obtainedSpec.channels;
    pDeviceInfo->nativeDataFormats[0].sampleRate = obtainedSpec.freq;
    pDeviceInfo->nativeDataFormats[0].flags      = 0;

    /* If miniaudio does not support the format, just use f32 as the native format (SDL will do the necessary conversions for us). */
    if (pDeviceInfo->nativeDataFormats[0].format == ma_format_unknown) {
        pDeviceInfo->nativeDataFormats[0].format = ma_format_f32;
    }

    return MA_SUCCESS;
}


void ma_audio_callback_capture__sdl(void* pUserData, ma_uint8* pBuffer, int bufferSizeInBytes)
{
    ma_device_ex* pDeviceEx = (ma_device_ex*)pUserData;

    MA_ASSERT(pDeviceEx != NULL);

    ma_uint32 bytes_per_frame = ma_get_bytes_per_frame(pDeviceEx->device.capture.internalFormat, pDeviceEx->device.capture.internalChannels);

    if(bytes_per_frame == 0) {
        return;
    }

    ma_device_handle_backend_data_callback((ma_device*)pDeviceEx, NULL, pBuffer, (ma_uint32)bufferSizeInBytes / bytes_per_frame);
}

void ma_audio_callback_playback__sdl(void* pUserData, ma_uint8* pBuffer, int bufferSizeInBytes)
{
    ma_device_ex* pDeviceEx = (ma_device_ex*)pUserData;

    MA_ASSERT(pDeviceEx != NULL);

    ma_uint32 bytes_per_frame = ma_get_bytes_per_frame(pDeviceEx->device.playback.internalFormat, pDeviceEx->device.playback.internalChannels);

    if(bytes_per_frame == 0) {
        return;
    }

    ma_device_handle_backend_data_callback((ma_device*)pDeviceEx, pBuffer, NULL, (ma_uint32)bufferSizeInBytes / bytes_per_frame);
}

static ma_result ma_device_init_internal__sdl(ma_device_ex* pDeviceEx, const ma_device_config* pConfig, ma_device_descriptor* pDescriptor)
{
    ma_context_ex* pContextEx;
    MA_SDL_AudioSpec desiredSpec;
    MA_SDL_AudioSpec obtainedSpec;
    const char* pDeviceName;
    int deviceID;

    MA_ASSERT(pDeviceEx != NULL);
    MA_ASSERT(pConfig != NULL);
    MA_ASSERT(pDescriptor != NULL);

    pContextEx = (ma_context_ex*)pDeviceEx->device.pContext;

    MA_ASSERT(pContextEx  != NULL);

    /*
	SDL is a little bit awkward with specifying the buffer size, You need to specify the size of the buffer in frames, but since we may
	have requested a period size in milliseconds we'll need to convert, which depends on the sample rate. But there's a possibility that
	the sample rate just set to 0, which indicates that the native sample rate should be used. There's no practical way to calculate this
	that I can think of right now so I'm just using MA_DEFAULT_SAMPLE_RATE.
    */
    if (pDescriptor->sampleRate == 0) {
        pDescriptor->sampleRate = MA_DEFAULT_SAMPLE_RATE;
    }

    /*
    When determining the period size, you need to take defaults into account. This is how the size of the period should be determined.

        1) If periodSizeInFrames is not 0, use periodSizeInFrames; else
        2) If periodSizeInMilliseconds is not 0, use periodSizeInMilliseconds; else
        3) If both periodSizeInFrames and periodSizeInMilliseconds is 0, use the backend's default. If the backend does not allow a default
           buffer size, use a default value of MA_DEFAULT_PERIOD_SIZE_IN_MILLISECONDS_LOW_LATENCY or 
           MA_DEFAULT_PERIOD_SIZE_IN_MILLISECONDS_CONSERVATIVE depending on the value of pConfig->performanceProfile.

    Note that options 2 and 3 require knowledge of the sample rate in order to convert it to a frame count. You should try to keep the
    calculation of the period size as accurate as possible, but sometimes it's just not practical so just use whatever you can.

    A helper function called ma_calculate_buffer_size_in_frames_from_descriptor() is available to do all of this for you which is what
    we'll be using here.
    */
    pDescriptor->periodSizeInFrames = ma_calculate_buffer_size_in_frames_from_descriptor(pDescriptor, pDescriptor->sampleRate, pConfig->performanceProfile);

    /* SDL wants the buffer size to be a power of 2 for some reason. */
    if (pDescriptor->periodSizeInFrames > 32768) {
        pDescriptor->periodSizeInFrames = 32768;
    } else {
        pDescriptor->periodSizeInFrames = ma_next_power_of_2(pDescriptor->periodSizeInFrames);
    }


    /* We now have enough information to set up the device. */
    MA_ZERO_OBJECT(&desiredSpec);
    desiredSpec.freq     = (int)pDescriptor->sampleRate;
    desiredSpec.format   = ma_format_to_sdl(pDescriptor->format);
    desiredSpec.channels = (ma_uint8)pDescriptor->channels;
    desiredSpec.samples  = (ma_uint16)pDescriptor->periodSizeInFrames;
    desiredSpec.callback = (pConfig->deviceType == ma_device_type_capture) ? ma_audio_callback_capture__sdl : ma_audio_callback_playback__sdl;
    desiredSpec.userdata = pDeviceEx;

    /* We'll fall back to f32 if we don't have an appropriate mapping between SDL and miniaudio. */
    if (desiredSpec.format == 0) {
        desiredSpec.format = MA_AUDIO_F32;
    }

    pDeviceName = NULL;
    if (pDescriptor->pDeviceID != NULL) {
        pDeviceName = ((MA_PFN_SDL_GetAudioDeviceName)pContextEx->sdl.SDL_GetAudioDeviceName)(pDescriptor->pDeviceID->custom.i, (pConfig->deviceType == ma_device_type_playback) ? 0 : 1);
    }

    deviceID = ((MA_PFN_SDL_OpenAudioDevice)pContextEx->sdl.SDL_OpenAudioDevice)(pDeviceName, (pConfig->deviceType == ma_device_type_playback) ? 0 : 1, &desiredSpec, &obtainedSpec, MA_SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (deviceID == 0) {
        ma_log_postf(ma_device_get_log((ma_device*)pDeviceEx), MA_LOG_LEVEL_ERROR, "Failed to open SDL2 device.");
        return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
    }

    if (pConfig->deviceType == ma_device_type_playback) {
        pDeviceEx->sdl.deviceIDPlayback = deviceID;
    } else {
        pDeviceEx->sdl.deviceIDCapture = deviceID;
    }

    /* The descriptor needs to be updated with our actual settings. */
    pDescriptor->format             = ma_format_from_sdl(obtainedSpec.format);
    pDescriptor->channels           = obtainedSpec.channels;
    pDescriptor->sampleRate         = (ma_uint32)obtainedSpec.freq;
    ma_channel_map_init_standard(ma_standard_channel_map_default, pDescriptor->channelMap, ma_countof(pDescriptor->channelMap), pDescriptor->channels);
    pDescriptor->periodSizeInFrames = obtainedSpec.samples;
    pDescriptor->periodCount        = 1;    /* SDL doesn't use the notion of period counts, so just set to 1. */

    return MA_SUCCESS;
}

static ma_result ma_device_init__sdl(ma_device* pDevice, const ma_device_config* pConfig, ma_device_descriptor* pDescriptorPlayback, ma_device_descriptor* pDescriptorCapture)
{
    ma_device_ex* pDeviceEx = (ma_device_ex*)pDevice;
    ma_context_ex* pContextEx = (ma_context_ex*)pDevice->pContext;
    ma_result result;

    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pContextEx != NULL);
    
    /* SDL does not support loopback mode, so must return MA_DEVICE_TYPE_NOT_SUPPORTED if it's requested. */
    if (pConfig->deviceType == ma_device_type_loopback) {
        return MA_DEVICE_TYPE_NOT_SUPPORTED;
    }

    if (pConfig->deviceType == ma_device_type_capture || pConfig->deviceType == ma_device_type_duplex) {
        result = ma_device_init_internal__sdl(pDeviceEx, pConfig, pDescriptorCapture);
        if (result != MA_SUCCESS) {
            return result;
        }
    }

    if (pConfig->deviceType == ma_device_type_playback || pConfig->deviceType == ma_device_type_duplex) {
        result = ma_device_init_internal__sdl(pDeviceEx, pConfig, pDescriptorPlayback);
        if (result != MA_SUCCESS) {
            if (pConfig->deviceType == ma_device_type_duplex) {
                ((MA_PFN_SDL_CloseAudioDevice)pContextEx->sdl.SDL_CloseAudioDevice)(pDeviceEx->sdl.deviceIDCapture);
            }

            return result;
        }
    }

    return MA_SUCCESS;
}

static ma_result ma_device_uninit__sdl(ma_device* pDevice)
{
    ma_device_ex* pDeviceEx = (ma_device_ex*)pDevice;
    ma_context_ex* pContextEx = (ma_context_ex*)pDevice->pContext;

    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pContextEx != NULL);

    if (pDevice->type == ma_device_type_capture || pDevice->type == ma_device_type_duplex) {
        ((MA_PFN_SDL_CloseAudioDevice)pContextEx->sdl.SDL_CloseAudioDevice)(pDeviceEx->sdl.deviceIDCapture);
    }

    if (pDevice->type == ma_device_type_playback || pDevice->type == ma_device_type_duplex) {
        ((MA_PFN_SDL_CloseAudioDevice)pContextEx->sdl.SDL_CloseAudioDevice)(pDeviceEx->sdl.deviceIDCapture);
    }

    return MA_SUCCESS;
}

static ma_result ma_device_start__sdl(ma_device* pDevice)
{
    ma_device_ex* pDeviceEx = (ma_device_ex*)pDevice;
    ma_context_ex* pContextEx = (ma_context_ex*)pDevice->pContext;

    MA_ASSERT(pDevice != NULL);

    if (pDevice->type == ma_device_type_capture || pDevice->type == ma_device_type_duplex) {
        ((MA_PFN_SDL_PauseAudioDevice)pContextEx->sdl.SDL_PauseAudioDevice)(pDeviceEx->sdl.deviceIDCapture, 0);
    }

    if (pDevice->type == ma_device_type_playback || pDevice->type == ma_device_type_duplex) {
        ((MA_PFN_SDL_PauseAudioDevice)pContextEx->sdl.SDL_PauseAudioDevice)(pDeviceEx->sdl.deviceIDPlayback, 0);
    }

    return MA_SUCCESS;
}

static ma_result ma_device_stop__sdl(ma_device* pDevice)
{
    ma_device_ex* pDeviceEx = (ma_device_ex*)pDevice;
    ma_context_ex* pContextEx = (ma_context_ex*)pDevice->pContext;

    MA_ASSERT(pDevice != NULL);

    if (pDevice->type == ma_device_type_capture || pDevice->type == ma_device_type_duplex) {
        ((MA_PFN_SDL_PauseAudioDevice)pContextEx->sdl.SDL_PauseAudioDevice)(pDeviceEx->sdl.deviceIDCapture, 1);
    }

    if (pDevice->type == ma_device_type_playback || pDevice->type == ma_device_type_duplex) {
        ((MA_PFN_SDL_PauseAudioDevice)pContextEx->sdl.SDL_PauseAudioDevice)(pDeviceEx->sdl.deviceIDPlayback, 1);
    }

    return MA_SUCCESS;
}

static ma_result ma_context_uninit__sdl(ma_context* pContext)
{
    ma_context_ex* pContextEx = (ma_context_ex*)pContext;

    MA_ASSERT(pContext != NULL);

    ((MA_PFN_SDL_QuitSubSystem)pContextEx->sdl.SDL_QuitSubSystem)(MA_SDL_INIT_AUDIO);

    return MA_SUCCESS;
}

static ma_result ma_context_init__sdl(ma_context* pContext, const ma_context_config* pConfig, ma_backend_callbacks* pCallbacks)
{
    ma_context_ex* pContextEx = (ma_context_ex*)pContext;
    int resultSDL;
    
    pContextEx->sdl.SDL_InitSubSystem      = (ma_proc)SDL_InitSubSystem;
    pContextEx->sdl.SDL_QuitSubSystem      = (ma_proc)SDL_QuitSubSystem;
    pContextEx->sdl.SDL_GetNumAudioDevices = (ma_proc)SDL_GetNumAudioDevices;
    pContextEx->sdl.SDL_GetAudioDeviceName = (ma_proc)SDL_GetAudioDeviceName;
    pContextEx->sdl.SDL_CloseAudioDevice   = (ma_proc)SDL_CloseAudioDevice;
    pContextEx->sdl.SDL_OpenAudioDevice    = (ma_proc)SDL_OpenAudioDevice;
    pContextEx->sdl.SDL_PauseAudioDevice   = (ma_proc)SDL_PauseAudioDevice;

    resultSDL = ((MA_PFN_SDL_InitSubSystem)pContextEx->sdl.SDL_InitSubSystem)(MA_SDL_INIT_AUDIO);
    if (resultSDL != 0) {
        return MA_ERROR;
    }

    /*
    The last step is to make sure the callbacks are set properly in pCallbacks. Internally, miniaudio will copy these callbacks into the
    context object and then use them for then on for calling into our custom backend.
    */
    pCallbacks->onContextInit             = ma_context_init__sdl;
    pCallbacks->onContextUninit           = ma_context_uninit__sdl;
    pCallbacks->onContextEnumerateDevices = ma_context_enumerate_devices__sdl;
    pCallbacks->onContextGetDeviceInfo    = ma_context_get_device_info__sdl;
    pCallbacks->onDeviceInit              = ma_device_init__sdl;
    pCallbacks->onDeviceUninit            = ma_device_uninit__sdl;
    pCallbacks->onDeviceStart             = ma_device_start__sdl;
    pCallbacks->onDeviceStop              = ma_device_stop__sdl;

    return MA_SUCCESS;
}
#endif  /* MA_HAS_SDL */


/*
This is our custom backend "loader". All this does is attempts to initialize our custom backends in the order they are listed. The first
one to successfully initialize is the one that's chosen. In this example we're just listing them statically, but you can use whatever logic
you want to handle backend selection.

This is used as the onContextInit() callback in the context config.
*/
static ma_result ma_context_init__custom_loader__sdl(ma_context* pContext, const ma_context_config* pConfig, ma_backend_callbacks* pCallbacks)
{
    ma_result result = MA_NO_BACKEND;

    /* Silence some unused parameter warnings just in case no custom backends are enabled. */
    (void)pContext;
    (void)pCallbacks;

    /* SDL. */
#if !defined(MA_NO_SDL)
    if (result != MA_SUCCESS) {
        result = ma_context_init__sdl(pContext, pConfig, pCallbacks);
    }
#endif

    /* ... plug in any other custom backends here ... */

    /* If we have a success result we have initialized a backend. Otherwise we need to tell miniaudio about the error so it can skip over our custom backends. */
    return result;
}
