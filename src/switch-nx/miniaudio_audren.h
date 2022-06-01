/******************************************************************************

Audren Backend

******************************************************************************/
#ifndef miniaudio_audren_h
#define miniaudio_audren_h

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <switch.h>

#define LIBNX_AUDREN_BUFFER_COUNT 5

static const AudioRendererConfig audio_renderer_config =
{
    .output_rate     = AudioRendererOutputRate_48kHz,
    .num_voices      = 24,
    .num_effects     = 0,
    .num_sinks       = 1,
    .num_mix_objs    = 1,
    .num_mix_buffers = 2,
};

static const int sample_rate           = 48000;
static const int num_channels          = 2;
static const uint8_t sink_channels[]   = { 0, 1 };

typedef struct
{
    AudioDriver drv;
    char* mempool;
    AudioDriverWaveBuf wavebufs[LIBNX_AUDREN_BUFFER_COUNT];
    AudioDriverWaveBuf* current_wavebuf;
    char* current_pool_ptr;
    size_t current_size;
    size_t buffer_size;
    size_t samples;
    Mutex update_lock;
} libnx_audren_t;

static ssize_t libnx_audren_audio_get_free_wavebuf_idx(libnx_audren_t* aud)
{
    unsigned i;

    for (i = 0; i < LIBNX_AUDREN_BUFFER_COUNT; i++)
    {
        if (aud->wavebufs[i].state == AudioDriverWaveBufState_Free || aud->wavebufs[i].state == AudioDriverWaveBufState_Done)
            return i;
    }

    return -1;
}

static size_t libnx_audren_audio_append(libnx_audren_t* aud, const char *buf, size_t size)
{
    void *dstbuf = NULL;
    ssize_t free_idx = -1;

    if (!aud->current_wavebuf)
    {
        free_idx = libnx_audren_audio_get_free_wavebuf_idx(aud);
        if (free_idx == -1)
            return 0;

        aud->current_wavebuf = &aud->wavebufs[free_idx];
        aud->current_pool_ptr = aud->mempool + (free_idx * aud->buffer_size);
        aud->current_size = 0;
    }

    if (size > aud->buffer_size - aud->current_size)
        size = aud->buffer_size - aud->current_size;

    dstbuf = aud->current_pool_ptr + aud->current_size;
    memcpy(dstbuf, buf, size);
    armDCacheFlush(dstbuf, size);

    aud->current_size += size;

    if (aud->current_size == aud->buffer_size)
    {
        audrvVoiceAddWaveBuf(&aud->drv, 0, aud->current_wavebuf);

        mutexLock(&aud->update_lock);
        audrvUpdate(&aud->drv);
        mutexUnlock(&aud->update_lock);

        if (!audrvVoiceIsPlaying(&aud->drv, 0))
        {
            audrvVoiceStart(&aud->drv, 0);
        }

        aud->current_wavebuf = NULL;
    }

    return size;
}

static ma_result ma_device_write__audren(ma_device* pDevice, const void* pFrames, ma_uint32 frameCount, ma_uint32* pFramesWritten)
{
    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pFrames != NULL);
    MA_ASSERT(pDevice->pContext->pUserData != NULL);
    libnx_audren_t *aud = (libnx_audren_t*)pDevice->pContext->pUserData;

    if (pFramesWritten != NULL)
        *pFramesWritten = 0;

    if (ma_device_get_state(pDevice) != ma_device_state_started)
        return MA_DEVICE_NOT_INITIALIZED;

    char *buf = (char*)pFrames;
    size_t size = frameCount * ma_get_bytes_per_frame(pDevice->playback.internalFormat, pDevice->playback.internalChannels);
    size_t written = 0;

    while (written < size)
    {
        written += libnx_audren_audio_append(aud, buf + written, size - written);
        if (written != size)
        {
            mutexLock(&aud->update_lock);
            audrvUpdate(&aud->drv);
            mutexUnlock(&aud->update_lock);
            audrenWaitFrame();
        }
    }

    if (written == 0)
        return MA_IO_ERROR;

    *pFramesWritten = frameCount;

    return MA_SUCCESS;
}

static ma_result ma_device_init__audren(ma_device* pDevice, const ma_device_config* pConfig, ma_device_descriptor* pDescriptorPlayback, ma_device_descriptor* pDescriptorCapture)
{
    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pConfig != NULL);
    MA_ASSERT(pDevice->pContext->pUserData != NULL);
    libnx_audren_t *aud = (libnx_audren_t*)pDevice->pContext->pUserData;

    if (pConfig->deviceType == ma_device_type_loopback) {
        return MA_DEVICE_TYPE_NOT_SUPPORTED;
    }

    pDescriptorPlayback->format = ma_format_s16;
    pDescriptorPlayback->channels = num_channels;
    pDescriptorPlayback->sampleRate = sample_rate;
    ma_channel_map_init_standard(ma_standard_channel_map_default, pDescriptorPlayback->channelMap, ma_countof(pDescriptorPlayback->channelMap), pDescriptorPlayback->channels);
    pDescriptorPlayback->periodSizeInFrames = ma_calculate_buffer_size_in_frames_from_descriptor(pDescriptorPlayback, pDescriptorPlayback->sampleRate, pConfig->performanceProfile);

    aud->buffer_size = pDescriptorPlayback->periodSizeInFrames * ma_get_bytes_per_frame(pDescriptorPlayback->format, pDescriptorPlayback->channels);
    aud->samples = (aud->buffer_size / num_channels / sizeof(int16_t));
    aud->current_size = 0;

    size_t mempool_size      = (aud->buffer_size * LIBNX_AUDREN_BUFFER_COUNT + (AUDREN_MEMPOOL_ALIGNMENT-1)) &~ (AUDREN_MEMPOOL_ALIGNMENT-1);
    aud->mempool      = memalign(AUDREN_MEMPOOL_ALIGNMENT, mempool_size);

    audrenInitialize(&audio_renderer_config);
    audrvCreate(&aud->drv, &audio_renderer_config, num_channels);

    unsigned i;
    for(i = 0; i < LIBNX_AUDREN_BUFFER_COUNT; i++)
    {
        aud->wavebufs[i].data_raw = aud->mempool;
        aud->wavebufs[i].size = mempool_size;
        aud->wavebufs[i].start_sample_offset = i * aud->samples;
        aud->wavebufs[i].end_sample_offset = aud->wavebufs[i].start_sample_offset + aud->samples;
    }
    aud->current_wavebuf = NULL;

    int mpid = audrvMemPoolAdd(&aud->drv, aud->mempool, mempool_size);
    audrvMemPoolAttach(&aud->drv, mpid);
    audrvDeviceSinkAdd(&aud->drv, AUDREN_DEFAULT_DEVICE_NAME, num_channels, sink_channels);
    audrenStartAudioRenderer();
    audrvVoiceInit(&aud->drv, 0, num_channels, PcmFormat_Int16, sample_rate);
    audrvVoiceSetDestinationMix(&aud->drv, 0, AUDREN_FINAL_MIX_ID);

    unsigned j;
    for(i = 0; i < num_channels; i++)
        for(j = 0; j < num_channels; j++)
            audrvVoiceSetMixFactor(&aud->drv, 0, i == j ? 1.0f : 0.0f, i, j);

    mutexInit(&aud->update_lock);

    return MA_SUCCESS;
}

static ma_result ma_context_get_device_info__audren(ma_context* pContext, ma_device_type deviceType, const ma_device_id* pDeviceID, ma_device_info* pDeviceInfo)
{
    MA_ASSERT(pContext != NULL);
    MA_ASSERT(deviceType == ma_device_type_playback);

    pDeviceInfo->isDefault = MA_TRUE;
    pDeviceInfo->nativeDataFormatCount = 0;

    ma_device_info_add_native_data_format(pDeviceInfo, ma_format_s16, num_channels, sample_rate, 0);

    return MA_SUCCESS;
}

static ma_result ma_device_start__audren(ma_device* pDevice)
{
    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pDevice->pContext->pUserData != NULL);
    libnx_audren_t *aud = (libnx_audren_t*)pDevice->pContext->pUserData;

    audrvVoiceStart(&aud->drv, 0);

    return MA_SUCCESS;
}

static ma_result ma_device_stop__audren(ma_device* pDevice)
{
    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pDevice->pContext->pUserData != NULL);
    libnx_audren_t *aud = (libnx_audren_t*)pDevice->pContext->pUserData;

    audrvVoiceStop(&aud->drv, 0);

    return MA_SUCCESS;
}

static ma_result ma_device_uninit__audren(ma_device* pDevice)
{
    MA_ASSERT(pDevice != NULL);
    MA_ASSERT(pDevice->pContext->pUserData != NULL);
    libnx_audren_t *aud = (libnx_audren_t*)pDevice->pContext->pUserData;

    audrvVoiceStop(&aud->drv, 0);
    audrvClose(&aud->drv);
    audrenExit();

    if (aud->mempool)
        free(aud->mempool);
    free(aud);

    return MA_SUCCESS;
}

static ma_result ma_context_init__audren(ma_context* pContext, const ma_context_config* pConfig, ma_backend_callbacks* pCallbacks)
{
    MA_ASSERT(pContext != NULL);

    pContext->pUserData = (libnx_audren_t*)calloc(1, sizeof(libnx_audren_t));

    (void)pConfig; /* Unused. */

    pCallbacks->onContextInit             = ma_context_init__audren;
    pCallbacks->onContextUninit           = NULL;
    pCallbacks->onContextEnumerateDevices = NULL;
    pCallbacks->onContextGetDeviceInfo    = ma_context_get_device_info__audren;
    pCallbacks->onDeviceInit              = ma_device_init__audren;
    pCallbacks->onDeviceUninit            = ma_device_uninit__audren;
    pCallbacks->onDeviceStart             = ma_device_start__audren;
    pCallbacks->onDeviceStop              = ma_device_stop__audren;
    pCallbacks->onDeviceRead              = NULL;
    pCallbacks->onDeviceWrite             = ma_device_write__audren;
    pCallbacks->onDeviceDataLoop          = NULL;

    return MA_SUCCESS;
}

#endif
