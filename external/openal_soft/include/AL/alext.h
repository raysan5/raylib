/**
 * OpenAL cross platform audio library
 * Copyright (C) 2008 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#ifndef AL_ALEXT_H
#define AL_ALEXT_H

#include <stddef.h>
/* Define int64_t and uint64_t types */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <inttypes.h>
#elif defined(_WIN32) && defined(__GNUC__)
#include <stdint.h>
#elif defined(_WIN32)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
/* Fallback if nothing above works */
#include <inttypes.h>
#endif

#include "alc.h"
#include "al.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AL_LOKI_IMA_ADPCM_format
#define AL_LOKI_IMA_ADPCM_format 1
#define AL_FORMAT_IMA_ADPCM_MONO16_EXT           0x10000
#define AL_FORMAT_IMA_ADPCM_STEREO16_EXT         0x10001
#endif

#ifndef AL_LOKI_WAVE_format
#define AL_LOKI_WAVE_format 1
#define AL_FORMAT_WAVE_EXT                       0x10002
#endif

#ifndef AL_EXT_vorbis
#define AL_EXT_vorbis 1
#define AL_FORMAT_VORBIS_EXT                     0x10003
#endif

#ifndef AL_LOKI_quadriphonic
#define AL_LOKI_quadriphonic 1
#define AL_FORMAT_QUAD8_LOKI                     0x10004
#define AL_FORMAT_QUAD16_LOKI                    0x10005
#endif

#ifndef AL_EXT_float32
#define AL_EXT_float32 1
#define AL_FORMAT_MONO_FLOAT32                   0x10010
#define AL_FORMAT_STEREO_FLOAT32                 0x10011
#endif

#ifndef AL_EXT_double
#define AL_EXT_double 1
#define AL_FORMAT_MONO_DOUBLE_EXT                0x10012
#define AL_FORMAT_STEREO_DOUBLE_EXT              0x10013
#endif

#ifndef AL_EXT_MULAW
#define AL_EXT_MULAW 1
#define AL_FORMAT_MONO_MULAW_EXT                 0x10014
#define AL_FORMAT_STEREO_MULAW_EXT               0x10015
#endif

#ifndef AL_EXT_ALAW
#define AL_EXT_ALAW 1
#define AL_FORMAT_MONO_ALAW_EXT                  0x10016
#define AL_FORMAT_STEREO_ALAW_EXT                0x10017
#endif

#ifndef ALC_LOKI_audio_channel
#define ALC_LOKI_audio_channel 1
#define ALC_CHAN_MAIN_LOKI                       0x500001
#define ALC_CHAN_PCM_LOKI                        0x500002
#define ALC_CHAN_CD_LOKI                         0x500003
#endif

#ifndef AL_EXT_MCFORMATS
#define AL_EXT_MCFORMATS 1
#define AL_FORMAT_QUAD8                          0x1204
#define AL_FORMAT_QUAD16                         0x1205
#define AL_FORMAT_QUAD32                         0x1206
#define AL_FORMAT_REAR8                          0x1207
#define AL_FORMAT_REAR16                         0x1208
#define AL_FORMAT_REAR32                         0x1209
#define AL_FORMAT_51CHN8                         0x120A
#define AL_FORMAT_51CHN16                        0x120B
#define AL_FORMAT_51CHN32                        0x120C
#define AL_FORMAT_61CHN8                         0x120D
#define AL_FORMAT_61CHN16                        0x120E
#define AL_FORMAT_61CHN32                        0x120F
#define AL_FORMAT_71CHN8                         0x1210
#define AL_FORMAT_71CHN16                        0x1211
#define AL_FORMAT_71CHN32                        0x1212
#endif

#ifndef AL_EXT_MULAW_MCFORMATS
#define AL_EXT_MULAW_MCFORMATS 1
#define AL_FORMAT_MONO_MULAW                     0x10014
#define AL_FORMAT_STEREO_MULAW                   0x10015
#define AL_FORMAT_QUAD_MULAW                     0x10021
#define AL_FORMAT_REAR_MULAW                     0x10022
#define AL_FORMAT_51CHN_MULAW                    0x10023
#define AL_FORMAT_61CHN_MULAW                    0x10024
#define AL_FORMAT_71CHN_MULAW                    0x10025
#endif

#ifndef AL_EXT_IMA4
#define AL_EXT_IMA4 1
#define AL_FORMAT_MONO_IMA4                      0x1300
#define AL_FORMAT_STEREO_IMA4                    0x1301
#endif

#ifndef AL_EXT_STATIC_BUFFER
#define AL_EXT_STATIC_BUFFER 1
typedef ALvoid (AL_APIENTRY*PFNALBUFFERDATASTATICPROC)(const ALint,ALenum,ALvoid*,ALsizei,ALsizei);
#ifdef AL_ALEXT_PROTOTYPES
AL_API ALvoid AL_APIENTRY alBufferDataStatic(const ALint buffer, ALenum format, ALvoid *data, ALsizei len, ALsizei freq);
#endif
#endif

#ifndef ALC_EXT_EFX
#define ALC_EXT_EFX 1
#include "efx.h"
#endif

#ifndef ALC_EXT_disconnect
#define ALC_EXT_disconnect 1
#define ALC_CONNECTED                            0x313
#endif

#ifndef ALC_EXT_thread_local_context
#define ALC_EXT_thread_local_context 1
typedef ALCboolean  (ALC_APIENTRY*PFNALCSETTHREADCONTEXTPROC)(ALCcontext *context);
typedef ALCcontext* (ALC_APIENTRY*PFNALCGETTHREADCONTEXTPROC)(void);
#ifdef AL_ALEXT_PROTOTYPES
ALC_API ALCboolean  ALC_APIENTRY alcSetThreadContext(ALCcontext *context);
ALC_API ALCcontext* ALC_APIENTRY alcGetThreadContext(void);
#endif
#endif

#ifndef AL_EXT_source_distance_model
#define AL_EXT_source_distance_model 1
#define AL_SOURCE_DISTANCE_MODEL                 0x200
#endif

#ifndef AL_SOFT_buffer_sub_data
#define AL_SOFT_buffer_sub_data 1
#define AL_BYTE_RW_OFFSETS_SOFT                  0x1031
#define AL_SAMPLE_RW_OFFSETS_SOFT                0x1032
typedef ALvoid (AL_APIENTRY*PFNALBUFFERSUBDATASOFTPROC)(ALuint,ALenum,const ALvoid*,ALsizei,ALsizei);
#ifdef AL_ALEXT_PROTOTYPES
AL_API ALvoid AL_APIENTRY alBufferSubDataSOFT(ALuint buffer,ALenum format,const ALvoid *data,ALsizei offset,ALsizei length);
#endif
#endif

#ifndef AL_SOFT_loop_points
#define AL_SOFT_loop_points 1
#define AL_LOOP_POINTS_SOFT                      0x2015
#endif

#ifndef AL_EXT_FOLDBACK
#define AL_EXT_FOLDBACK 1
#define AL_EXT_FOLDBACK_NAME                     "AL_EXT_FOLDBACK"
#define AL_FOLDBACK_EVENT_BLOCK                  0x4112
#define AL_FOLDBACK_EVENT_START                  0x4111
#define AL_FOLDBACK_EVENT_STOP                   0x4113
#define AL_FOLDBACK_MODE_MONO                    0x4101
#define AL_FOLDBACK_MODE_STEREO                  0x4102
typedef void (AL_APIENTRY*LPALFOLDBACKCALLBACK)(ALenum,ALsizei);
typedef void (AL_APIENTRY*LPALREQUESTFOLDBACKSTART)(ALenum,ALsizei,ALsizei,ALfloat*,LPALFOLDBACKCALLBACK);
typedef void (AL_APIENTRY*LPALREQUESTFOLDBACKSTOP)(void);
#ifdef AL_ALEXT_PROTOTYPES
AL_API void AL_APIENTRY alRequestFoldbackStart(ALenum mode,ALsizei count,ALsizei length,ALfloat *mem,LPALFOLDBACKCALLBACK callback);
AL_API void AL_APIENTRY alRequestFoldbackStop(void);
#endif
#endif

#ifndef ALC_EXT_DEDICATED
#define ALC_EXT_DEDICATED 1
#define AL_DEDICATED_GAIN                        0x0001
#define AL_EFFECT_DEDICATED_DIALOGUE             0x9001
#define AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT 0x9000
#endif

#ifndef AL_SOFT_buffer_samples
#define AL_SOFT_buffer_samples 1
/* Channel configurations */
#define AL_MONO_SOFT                             0x1500
#define AL_STEREO_SOFT                           0x1501
#define AL_REAR_SOFT                             0x1502
#define AL_QUAD_SOFT                             0x1503
#define AL_5POINT1_SOFT                          0x1504
#define AL_6POINT1_SOFT                          0x1505
#define AL_7POINT1_SOFT                          0x1506

/* Sample types */
#define AL_BYTE_SOFT                             0x1400
#define AL_UNSIGNED_BYTE_SOFT                    0x1401
#define AL_SHORT_SOFT                            0x1402
#define AL_UNSIGNED_SHORT_SOFT                   0x1403
#define AL_INT_SOFT                              0x1404
#define AL_UNSIGNED_INT_SOFT                     0x1405
#define AL_FLOAT_SOFT                            0x1406
#define AL_DOUBLE_SOFT                           0x1407
#define AL_BYTE3_SOFT                            0x1408
#define AL_UNSIGNED_BYTE3_SOFT                   0x1409

/* Storage formats */
#define AL_MONO8_SOFT                            0x1100
#define AL_MONO16_SOFT                           0x1101
#define AL_MONO32F_SOFT                          0x10010
#define AL_STEREO8_SOFT                          0x1102
#define AL_STEREO16_SOFT                         0x1103
#define AL_STEREO32F_SOFT                        0x10011
#define AL_QUAD8_SOFT                            0x1204
#define AL_QUAD16_SOFT                           0x1205
#define AL_QUAD32F_SOFT                          0x1206
#define AL_REAR8_SOFT                            0x1207
#define AL_REAR16_SOFT                           0x1208
#define AL_REAR32F_SOFT                          0x1209
#define AL_5POINT1_8_SOFT                        0x120A
#define AL_5POINT1_16_SOFT                       0x120B
#define AL_5POINT1_32F_SOFT                      0x120C
#define AL_6POINT1_8_SOFT                        0x120D
#define AL_6POINT1_16_SOFT                       0x120E
#define AL_6POINT1_32F_SOFT                      0x120F
#define AL_7POINT1_8_SOFT                        0x1210
#define AL_7POINT1_16_SOFT                       0x1211
#define AL_7POINT1_32F_SOFT                      0x1212

/* Buffer attributes */
#define AL_INTERNAL_FORMAT_SOFT                  0x2008
#define AL_BYTE_LENGTH_SOFT                      0x2009
#define AL_SAMPLE_LENGTH_SOFT                    0x200A
#define AL_SEC_LENGTH_SOFT                       0x200B

typedef void (AL_APIENTRY*LPALBUFFERSAMPLESSOFT)(ALuint,ALuint,ALenum,ALsizei,ALenum,ALenum,const ALvoid*);
typedef void (AL_APIENTRY*LPALBUFFERSUBSAMPLESSOFT)(ALuint,ALsizei,ALsizei,ALenum,ALenum,const ALvoid*);
typedef void (AL_APIENTRY*LPALGETBUFFERSAMPLESSOFT)(ALuint,ALsizei,ALsizei,ALenum,ALenum,ALvoid*);
typedef ALboolean (AL_APIENTRY*LPALISBUFFERFORMATSUPPORTEDSOFT)(ALenum);
#ifdef AL_ALEXT_PROTOTYPES
AL_API void AL_APIENTRY alBufferSamplesSOFT(ALuint buffer, ALuint samplerate, ALenum internalformat, ALsizei samples, ALenum channels, ALenum type, const ALvoid *data);
AL_API void AL_APIENTRY alBufferSubSamplesSOFT(ALuint buffer, ALsizei offset, ALsizei samples, ALenum channels, ALenum type, const ALvoid *data);
AL_API void AL_APIENTRY alGetBufferSamplesSOFT(ALuint buffer, ALsizei offset, ALsizei samples, ALenum channels, ALenum type, ALvoid *data);
AL_API ALboolean AL_APIENTRY alIsBufferFormatSupportedSOFT(ALenum format);
#endif
#endif

#ifndef AL_SOFT_direct_channels
#define AL_SOFT_direct_channels 1
#define AL_DIRECT_CHANNELS_SOFT                  0x1033
#endif

#ifndef ALC_SOFT_loopback
#define ALC_SOFT_loopback 1
#define ALC_FORMAT_CHANNELS_SOFT                 0x1990
#define ALC_FORMAT_TYPE_SOFT                     0x1991

/* Sample types */
#define ALC_BYTE_SOFT                            0x1400
#define ALC_UNSIGNED_BYTE_SOFT                   0x1401
#define ALC_SHORT_SOFT                           0x1402
#define ALC_UNSIGNED_SHORT_SOFT                  0x1403
#define ALC_INT_SOFT                             0x1404
#define ALC_UNSIGNED_INT_SOFT                    0x1405
#define ALC_FLOAT_SOFT                           0x1406

/* Channel configurations */
#define ALC_MONO_SOFT                            0x1500
#define ALC_STEREO_SOFT                          0x1501
#define ALC_QUAD_SOFT                            0x1503
#define ALC_5POINT1_SOFT                         0x1504
#define ALC_6POINT1_SOFT                         0x1505
#define ALC_7POINT1_SOFT                         0x1506

typedef ALCdevice* (ALC_APIENTRY*LPALCLOOPBACKOPENDEVICESOFT)(const ALCchar*);
typedef ALCboolean (ALC_APIENTRY*LPALCISRENDERFORMATSUPPORTEDSOFT)(ALCdevice*,ALCsizei,ALCenum,ALCenum);
typedef void (ALC_APIENTRY*LPALCRENDERSAMPLESSOFT)(ALCdevice*,ALCvoid*,ALCsizei);
#ifdef AL_ALEXT_PROTOTYPES
ALC_API ALCdevice* ALC_APIENTRY alcLoopbackOpenDeviceSOFT(const ALCchar *deviceName);
ALC_API ALCboolean ALC_APIENTRY alcIsRenderFormatSupportedSOFT(ALCdevice *device, ALCsizei freq, ALCenum channels, ALCenum type);
ALC_API void ALC_APIENTRY alcRenderSamplesSOFT(ALCdevice *device, ALCvoid *buffer, ALCsizei samples);
#endif
#endif

#ifndef AL_EXT_STEREO_ANGLES
#define AL_EXT_STEREO_ANGLES 1
#define AL_STEREO_ANGLES                         0x1030
#endif

#ifndef AL_EXT_SOURCE_RADIUS
#define AL_EXT_SOURCE_RADIUS 1
#define AL_SOURCE_RADIUS                         0x1031
#endif

#ifndef AL_SOFT_source_latency
#define AL_SOFT_source_latency 1
#define AL_SAMPLE_OFFSET_LATENCY_SOFT            0x1200
#define AL_SEC_OFFSET_LATENCY_SOFT               0x1201
typedef int64_t ALint64SOFT;
typedef uint64_t ALuint64SOFT;
typedef void (AL_APIENTRY*LPALSOURCEDSOFT)(ALuint,ALenum,ALdouble);
typedef void (AL_APIENTRY*LPALSOURCE3DSOFT)(ALuint,ALenum,ALdouble,ALdouble,ALdouble);
typedef void (AL_APIENTRY*LPALSOURCEDVSOFT)(ALuint,ALenum,const ALdouble*);
typedef void (AL_APIENTRY*LPALGETSOURCEDSOFT)(ALuint,ALenum,ALdouble*);
typedef void (AL_APIENTRY*LPALGETSOURCE3DSOFT)(ALuint,ALenum,ALdouble*,ALdouble*,ALdouble*);
typedef void (AL_APIENTRY*LPALGETSOURCEDVSOFT)(ALuint,ALenum,ALdouble*);
typedef void (AL_APIENTRY*LPALSOURCEI64SOFT)(ALuint,ALenum,ALint64SOFT);
typedef void (AL_APIENTRY*LPALSOURCE3I64SOFT)(ALuint,ALenum,ALint64SOFT,ALint64SOFT,ALint64SOFT);
typedef void (AL_APIENTRY*LPALSOURCEI64VSOFT)(ALuint,ALenum,const ALint64SOFT*);
typedef void (AL_APIENTRY*LPALGETSOURCEI64SOFT)(ALuint,ALenum,ALint64SOFT*);
typedef void (AL_APIENTRY*LPALGETSOURCE3I64SOFT)(ALuint,ALenum,ALint64SOFT*,ALint64SOFT*,ALint64SOFT*);
typedef void (AL_APIENTRY*LPALGETSOURCEI64VSOFT)(ALuint,ALenum,ALint64SOFT*);
#ifdef AL_ALEXT_PROTOTYPES
AL_API void AL_APIENTRY alSourcedSOFT(ALuint source, ALenum param, ALdouble value);
AL_API void AL_APIENTRY alSource3dSOFT(ALuint source, ALenum param, ALdouble value1, ALdouble value2, ALdouble value3);
AL_API void AL_APIENTRY alSourcedvSOFT(ALuint source, ALenum param, const ALdouble *values);
AL_API void AL_APIENTRY alGetSourcedSOFT(ALuint source, ALenum param, ALdouble *value);
AL_API void AL_APIENTRY alGetSource3dSOFT(ALuint source, ALenum param, ALdouble *value1, ALdouble *value2, ALdouble *value3);
AL_API void AL_APIENTRY alGetSourcedvSOFT(ALuint source, ALenum param, ALdouble *values);
AL_API void AL_APIENTRY alSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT value);
AL_API void AL_APIENTRY alSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT value1, ALint64SOFT value2, ALint64SOFT value3);
AL_API void AL_APIENTRY alSourcei64vSOFT(ALuint source, ALenum param, const ALint64SOFT *values);
AL_API void AL_APIENTRY alGetSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT *value);
AL_API void AL_APIENTRY alGetSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT *value1, ALint64SOFT *value2, ALint64SOFT *value3);
AL_API void AL_APIENTRY alGetSourcei64vSOFT(ALuint source, ALenum param, ALint64SOFT *values);
#endif
#endif

#ifndef ALC_EXT_DEFAULT_FILTER_ORDER
#define ALC_EXT_DEFAULT_FILTER_ORDER 1
#define ALC_DEFAULT_FILTER_ORDER                 0x1100
#endif

#ifndef AL_SOFT_deferred_updates
#define AL_SOFT_deferred_updates 1
#define AL_DEFERRED_UPDATES_SOFT                 0xC002
typedef ALvoid (AL_APIENTRY*LPALDEFERUPDATESSOFT)(void);
typedef ALvoid (AL_APIENTRY*LPALPROCESSUPDATESSOFT)(void);
#ifdef AL_ALEXT_PROTOTYPES
AL_API ALvoid AL_APIENTRY alDeferUpdatesSOFT(void);
AL_API ALvoid AL_APIENTRY alProcessUpdatesSOFT(void);
#endif
#endif

#ifndef AL_SOFT_block_alignment
#define AL_SOFT_block_alignment 1
#define AL_UNPACK_BLOCK_ALIGNMENT_SOFT           0x200C
#define AL_PACK_BLOCK_ALIGNMENT_SOFT             0x200D
#endif

#ifndef AL_SOFT_MSADPCM
#define AL_SOFT_MSADPCM 1
#define AL_FORMAT_MONO_MSADPCM_SOFT              0x1302
#define AL_FORMAT_STEREO_MSADPCM_SOFT            0x1303
#endif

#ifndef AL_SOFT_source_length
#define AL_SOFT_source_length 1
/*#define AL_BYTE_LENGTH_SOFT                      0x2009*/
/*#define AL_SAMPLE_LENGTH_SOFT                    0x200A*/
/*#define AL_SEC_LENGTH_SOFT                       0x200B*/
#endif

#ifndef ALC_SOFT_pause_device
#define ALC_SOFT_pause_device 1
typedef void (ALC_APIENTRY*LPALCDEVICEPAUSESOFT)(ALCdevice *device);
typedef void (ALC_APIENTRY*LPALCDEVICERESUMESOFT)(ALCdevice *device);
#ifdef AL_ALEXT_PROTOTYPES
ALC_API void ALC_APIENTRY alcDevicePauseSOFT(ALCdevice *device);
ALC_API void ALC_APIENTRY alcDeviceResumeSOFT(ALCdevice *device);
#endif
#endif

#ifndef AL_EXT_BFORMAT
#define AL_EXT_BFORMAT 1
#define AL_FORMAT_BFORMAT2D_8                    0x20021
#define AL_FORMAT_BFORMAT2D_16                   0x20022
#define AL_FORMAT_BFORMAT2D_FLOAT32              0x20023
#define AL_FORMAT_BFORMAT3D_8                    0x20031
#define AL_FORMAT_BFORMAT3D_16                   0x20032
#define AL_FORMAT_BFORMAT3D_FLOAT32              0x20033
#endif

#ifndef AL_EXT_MULAW_BFORMAT
#define AL_EXT_MULAW_BFORMAT 1
#define AL_FORMAT_BFORMAT2D_MULAW                0x10031
#define AL_FORMAT_BFORMAT3D_MULAW                0x10032
#endif

#ifndef ALC_SOFT_HRTF
#define ALC_SOFT_HRTF 1
#define ALC_HRTF_SOFT                            0x1992
#define ALC_DONT_CARE_SOFT                       0x0002
#define ALC_HRTF_STATUS_SOFT                     0x1993
#define ALC_HRTF_DISABLED_SOFT                   0x0000
#define ALC_HRTF_ENABLED_SOFT                    0x0001
#define ALC_HRTF_DENIED_SOFT                     0x0002
#define ALC_HRTF_REQUIRED_SOFT                   0x0003
#define ALC_HRTF_HEADPHONES_DETECTED_SOFT        0x0004
#define ALC_HRTF_UNSUPPORTED_FORMAT_SOFT         0x0005
#define ALC_NUM_HRTF_SPECIFIERS_SOFT             0x1994
#define ALC_HRTF_SPECIFIER_SOFT                  0x1995
#define ALC_HRTF_ID_SOFT                         0x1996
typedef const ALCchar* (ALC_APIENTRY*LPALCGETSTRINGISOFT)(ALCdevice *device, ALCenum paramName, ALCsizei index);
typedef ALCboolean (ALC_APIENTRY*LPALCRESETDEVICESOFT)(ALCdevice *device, const ALCint *attribs);
#ifdef AL_ALEXT_PROTOTYPES
ALC_API const ALCchar* ALC_APIENTRY alcGetStringiSOFT(ALCdevice *device, ALCenum paramName, ALCsizei index);
ALC_API ALCboolean ALC_APIENTRY alcResetDeviceSOFT(ALCdevice *device, const ALCint *attribs);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
