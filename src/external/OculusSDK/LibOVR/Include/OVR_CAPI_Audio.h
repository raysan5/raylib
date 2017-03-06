/********************************************************************************//**
\file      OVR_CAPI_Audio.h
\brief     CAPI audio functions.
\copyright Copyright 2015 Oculus VR, LLC. All Rights reserved.
************************************************************************************/


#ifndef OVR_CAPI_Audio_h
#define OVR_CAPI_Audio_h

#ifdef _WIN32
// Prevents <Windows.h> from defining min() and max() macro symbols.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "OVR_CAPI.h"
#define OVR_AUDIO_MAX_DEVICE_STR_SIZE 128

#if !defined(OVR_EXPORTING_CAPI)

/// Gets the ID of the preferred VR audio output device.
///
/// \param[out] deviceOutId The ID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be WAVE_MAPPER.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetAudioDeviceOutWaveId(UINT* deviceOutId);

/// Gets the ID of the preferred VR audio input device.
///
/// \param[out] deviceInId The ID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be WAVE_MAPPER.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetAudioDeviceInWaveId(UINT* deviceInId);


/// Gets the GUID of the preferred VR audio device as a string.
///
/// \param[out] deviceOutStrBuffer A buffer where the GUID string for the device will copied to.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetAudioDeviceOutGuidStr(WCHAR deviceOutStrBuffer[OVR_AUDIO_MAX_DEVICE_STR_SIZE]);


/// Gets the GUID of the preferred VR audio device.
///
/// \param[out] deviceOutGuid The GUID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be NULL.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetAudioDeviceOutGuid(GUID* deviceOutGuid);


/// Gets the GUID of the preferred VR microphone device as a string.
///
/// \param[out] deviceInStrBuffer A buffer where the GUID string for the device will copied to.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetAudioDeviceInGuidStr(WCHAR deviceInStrBuffer[OVR_AUDIO_MAX_DEVICE_STR_SIZE]);


/// Gets the GUID of the preferred VR microphone device.
///
/// \param[out] deviceInGuid The GUID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be NULL.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetAudioDeviceInGuid(GUID* deviceInGuid);

#endif // !defined(OVR_EXPORTING_CAPI)

#endif //OVR_OS_MS

#endif    // OVR_CAPI_Audio_h
