/********************************************************************************//**
\file  OVR_ErrorCode.h
\brief     This header provides LibOVR error code declarations.
\copyright Copyright 2015-2016 Oculus VR, LLC All Rights reserved.
*************************************************************************************/

#ifndef OVR_ErrorCode_h
#define OVR_ErrorCode_h


#include "OVR_Version.h"
#include <stdint.h>







#ifndef OVR_RESULT_DEFINED
#define OVR_RESULT_DEFINED ///< Allows ovrResult to be independently defined.
/// API call results are represented at the highest level by a single ovrResult.
typedef int32_t ovrResult;
#endif


/// \brief Indicates if an ovrResult indicates success.
///
/// Some functions return additional successful values other than ovrSucces and
/// require usage of this macro to indicate successs.
///
#if !defined(OVR_SUCCESS)
    #define OVR_SUCCESS(result) (result >= 0)
#endif


/// \brief Indicates if an ovrResult indicates an unqualified success.
///
/// This is useful for indicating that the code intentionally wants to
/// check for result == ovrSuccess as opposed to OVR_SUCCESS(), which
/// checks for result >= ovrSuccess.
///
#if !defined(OVR_UNQUALIFIED_SUCCESS)
    #define OVR_UNQUALIFIED_SUCCESS(result) (result == ovrSuccess)
#endif


/// \brief Indicates if an ovrResult indicates failure.
///
#if !defined(OVR_FAILURE)
    #define OVR_FAILURE(result) (!OVR_SUCCESS(result))
#endif


// Success is a value greater or equal to 0, while all error types are negative values.
#ifndef OVR_SUCCESS_DEFINED
#define OVR_SUCCESS_DEFINED ///< Allows ovrResult to be independently defined.
typedef enum ovrSuccessType_
{
    /// This is a general success result. Use OVR_SUCCESS to test for success.
    ovrSuccess = 0,

    /// Returned from a call to SubmitFrame. The call succeeded, but what the app
    /// rendered will not be visible on the HMD. Ideally the app should continue
    /// calling SubmitFrame, but not do any rendering. When the result becomes
    /// ovrSuccess, rendering should continue as usual.
    ovrSuccess_NotVisible                 = 1000,

    ovrSuccess_HMDFirmwareMismatch        = 4100,   ///< The HMD Firmware is out of date but is acceptable.
    ovrSuccess_TrackerFirmwareMismatch    = 4101,   ///< The Tracker Firmware is out of date but is acceptable.
    ovrSuccess_ControllerFirmwareMismatch = 4104,   ///< The controller firmware is out of date but is acceptable.
    ovrSuccess_TrackerDriverNotFound      = 4105,   ///< The tracker driver interface was not found. Can be a temporary error

} ovrSuccessType;
#endif


typedef enum ovrErrorType_
{
    /* General errors */
    ovrError_MemoryAllocationFailure    = -1000,   ///< Failure to allocate memory.
    ovrError_SocketCreationFailure      = -1001,   ///< Failure to create a socket.
    ovrError_InvalidSession             = -1002,   ///< Invalid ovrSession parameter provided.
    ovrError_Timeout                    = -1003,   ///< The operation timed out.
    ovrError_NotInitialized             = -1004,   ///< The system or component has not been initialized.
    ovrError_InvalidParameter           = -1005,   ///< Invalid parameter provided. See error info or log for details.
    ovrError_ServiceError               = -1006,   ///< Generic service error. See error info or log for details.
    ovrError_NoHmd                      = -1007,   ///< The given HMD doesn't exist.
    ovrError_Unsupported                = -1009,   ///< Function call is not supported on this hardware/software
    ovrError_DeviceUnavailable          = -1010,   ///< Specified device type isn't available.
    ovrError_InvalidHeadsetOrientation  = -1011,   ///< The headset was in an invalid orientation for the requested operation (e.g. vertically oriented during ovr_RecenterPose).
    ovrError_ClientSkippedDestroy       = -1012,   ///< The client failed to call ovr_Destroy on an active session before calling ovr_Shutdown. Or the client crashed.
    ovrError_ClientSkippedShutdown      = -1013,   ///< The client failed to call ovr_Shutdown or the client crashed.
    ovrError_ServiceDeadlockDetected    = -1014,   ///< The service watchdog discovered a deadlock.

    /* Audio error range, reserved for Audio errors. */
    ovrError_AudioReservedBegin         = -2000,   ///< First Audio error.
    ovrError_AudioDeviceNotFound        = -2001,   ///< Failure to find the specified audio device.
    ovrError_AudioComError              = -2002,   ///< Generic COM error.
    ovrError_AudioReservedEnd           = -2999,   ///< Last Audio error.

    /* Initialization errors. */
    ovrError_Initialize                 = -3000,   ///< Generic initialization error.
    ovrError_LibLoad                    = -3001,   ///< Couldn't load LibOVRRT.
    ovrError_LibVersion                 = -3002,   ///< LibOVRRT version incompatibility.
    ovrError_ServiceConnection          = -3003,   ///< Couldn't connect to the OVR Service.
    ovrError_ServiceVersion             = -3004,   ///< OVR Service version incompatibility.
    ovrError_IncompatibleOS             = -3005,   ///< The operating system version is incompatible.
    ovrError_DisplayInit                = -3006,   ///< Unable to initialize the HMD display.
    ovrError_ServerStart                = -3007,   ///< Unable to start the server. Is it already running?
    ovrError_Reinitialization           = -3008,   ///< Attempting to re-initialize with a different version.
    ovrError_MismatchedAdapters         = -3009,   ///< Chosen rendering adapters between client and service do not match
    ovrError_LeakingResources           = -3010,   ///< Calling application has leaked resources
    ovrError_ClientVersion              = -3011,   ///< Client version too old to connect to service
    ovrError_OutOfDateOS                = -3012,   ///< The operating system is out of date.
    ovrError_OutOfDateGfxDriver         = -3013,   ///< The graphics driver is out of date.
    ovrError_IncompatibleGPU            = -3014,   ///< The graphics hardware is not supported
    ovrError_NoValidVRDisplaySystem     = -3015,   ///< No valid VR display system found.
    ovrError_Obsolete                   = -3016,   ///< Feature or API is obsolete and no longer supported.
    ovrError_DisabledOrDefaultAdapter   = -3017,   ///< No supported VR display system found, but disabled or driverless adapter found.
    ovrError_HybridGraphicsNotSupported = -3018,   ///< The system is using hybrid graphics (Optimus, etc...), which is not support.
    ovrError_DisplayManagerInit         = -3019,   ///< Initialization of the DisplayManager failed.
    ovrError_TrackerDriverInit          = -3020,   ///< Failed to get the interface for an attached tracker

    /* Hardware errors */
    ovrError_InvalidBundleAdjustment    = -4000,   ///< Headset has no bundle adjustment data.
    ovrError_USBBandwidth               = -4001,   ///< The USB hub cannot handle the camera frame bandwidth.
    ovrError_USBEnumeratedSpeed         = -4002,   ///< The USB camera is not enumerating at the correct device speed.
    ovrError_ImageSensorCommError       = -4003,   ///< Unable to communicate with the image sensor.
    ovrError_GeneralTrackerFailure      = -4004,   ///< We use this to report various sensor issues that don't fit in an easily classifiable bucket.
    ovrError_ExcessiveFrameTruncation   = -4005,   ///< A more than acceptable number of frames are coming back truncated.
    ovrError_ExcessiveFrameSkipping     = -4006,   ///< A more than acceptable number of frames have been skipped.
    ovrError_SyncDisconnected           = -4007,   ///< The sensor is not receiving the sync signal (cable disconnected?).
    ovrError_TrackerMemoryReadFailure   = -4008,   ///< Failed to read memory from the sensor.
    ovrError_TrackerMemoryWriteFailure  = -4009,   ///< Failed to write memory from the sensor.
    ovrError_TrackerFrameTimeout        = -4010,   ///< Timed out waiting for a camera frame.
    ovrError_TrackerTruncatedFrame      = -4011,   ///< Truncated frame returned from sensor.
    ovrError_TrackerDriverFailure       = -4012,   ///< The sensor driver has encountered a problem.
    ovrError_TrackerNRFFailure          = -4013,   ///< The sensor wireless subsystem has encountered a problem.
    ovrError_HardwareGone               = -4014,   ///< The hardware has been unplugged
    ovrError_NordicEnabledNoSync        = -4015,   ///< The nordic indicates that sync is enabled but it is not sending sync pulses
    ovrError_NordicSyncNoFrames         = -4016,   ///< It looks like we're getting a sync signal, but no camera frames have been received
    ovrError_CatastrophicFailure        = -4017,   ///< A catastrophic failure has occurred.  We will attempt to recover by resetting the device
    ovrError_CatastrophicTimeout        = -4018,   ///< The catastrophic recovery has timed out.
    ovrError_RepeatCatastrophicFail     = -4019,   ///< Catastrophic failure has repeated too many times.
    ovrError_USBOpenDeviceFailure       = -4020,   ///< Could not open handle for Rift device (likely already in use by another process).
    ovrError_HMDGeneralFailure          = -4021,   ///< Unexpected HMD issues that don't fit a specific bucket.

    ovrError_HMDFirmwareMismatch        = -4100,   ///< The HMD Firmware is out of date and is unacceptable.
    ovrError_TrackerFirmwareMismatch    = -4101,   ///< The sensor Firmware is out of date and is unacceptable.
    ovrError_BootloaderDeviceDetected   = -4102,   ///< A bootloader HMD is detected by the service.
    ovrError_TrackerCalibrationError    = -4103,   ///< The sensor calibration is missing or incorrect.
    ovrError_ControllerFirmwareMismatch = -4104,   ///< The controller firmware is out of date and is unacceptable.
    ovrError_DevManDeviceDetected       = -4105,   ///< A DeviceManagement mode HMD is detected by the service.
    ovrError_RebootedBootloaderDevice   = -4106,   ///< Had to reboot bootloader device, which succeeded.
    ovrError_FailedRebootBootloaderDev  = -4107,   ///< Had to reboot bootloader device, which failed.  Device is stuck in bootloader mode.

    ovrError_IMUTooManyLostSamples      = -4200,   ///< Too many lost IMU samples.
    ovrError_IMURateError               = -4201,   ///< IMU rate is outside of the expected range.
    ovrError_FeatureReportFailure       = -4202,   ///< A feature report has failed.
    ovrError_HMDWirelessTimeout         = -4203,   ///< HMD wireless interface never returned from busy state.

    ovrError_BootloaderAssertLog        = -4300,   ///< HMD Bootloader Assert Log was not empty.
    ovrError_AppAssertLog               = -4301,   ///< HMD App Assert Log was not empty.

    /* Synchronization errors */
    ovrError_Incomplete                 = -5000,   ///< Requested async work not yet complete.
    ovrError_Abandoned                  = -5001,   ///< Requested async work was abandoned and result is incomplete.

    /* Rendering errors */
    ovrError_DisplayLost                = -6000,   ///< In the event of a system-wide graphics reset or cable unplug this is returned to the app.
    ovrError_TextureSwapChainFull       = -6001,   ///< ovr_CommitTextureSwapChain was called too many times on a texture swapchain without calling submit to use the chain.
    ovrError_TextureSwapChainInvalid    = -6002,   ///< The ovrTextureSwapChain is in an incomplete or inconsistent state. Ensure ovr_CommitTextureSwapChain was called at least once first.
    ovrError_GraphicsDeviceReset        = -6003,   ///< Graphics device has been reset (TDR, etc...)
    ovrError_DisplayRemoved             = -6004,   ///< HMD removed from the display adapter
    ovrError_ContentProtectionNotAvailable = -6005,///<Content protection is not available for the display
    ovrError_ApplicationInvisible       = -6006,   ///< Application declared itself as an invisible type and is not allowed to submit frames.
    ovrError_Disallowed                 = -6007,   ///< The given request is disallowed under the current conditions.
    ovrError_DisplayPluggedIncorrectly  = -6008,   ///< Display portion of HMD is plugged into an incompatible port (ex: IGP)

    /* Fatal errors */
    ovrError_RuntimeException           = -7000,   ///< A runtime exception occurred. The application is required to shutdown LibOVR and re-initialize it before this error state will be cleared.


    ovrError_MetricsUnknownApp            = -90000,
    ovrError_MetricsDuplicateApp          = -90001,
    ovrError_MetricsNoEvents              = -90002,
    ovrError_MetricsRuntime               = -90003,
    ovrError_MetricsFile                  = -90004,
    ovrError_MetricsNoClientInfo          = -90005,
    ovrError_MetricsNoAppMetaData         = -90006,
    ovrError_MetricsNoApp                 = -90007,
    ovrError_MetricsOafFailure            = -90008,
    ovrError_MetricsSessionAlreadyActive  = -90009,
    ovrError_MetricsSessionNotActive      = -90010,

} ovrErrorType;



/// Provides information about the last error.
/// \see ovr_GetLastErrorInfo
typedef struct ovrErrorInfo_
{
    ovrResult Result;               ///< The result from the last API call that generated an error ovrResult.
    char      ErrorString[512];     ///< A UTF8-encoded null-terminated English string describing the problem. The format of this string is subject to change in future versions.
} ovrErrorInfo;

#endif /* OVR_ErrorCode_h */
