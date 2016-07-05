/********************************************************************************//**
\file      OVR_CAPI.h
\brief     C Interface to the Oculus PC SDK tracking and rendering library.
\copyright Copyright 2014 Oculus VR, LLC All Rights reserved.
************************************************************************************/

#ifndef OVR_CAPI_h  //   We don't use version numbers within this name, as all versioned variations of this file are currently mutually exclusive.
#define OVR_CAPI_h  ///< Header include guard


#include "OVR_CAPI_Keys.h"
#include "OVR_Version.h"
#include "OVR_ErrorCode.h"


#include <stdint.h>

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4324) // structure was padded due to __declspec(align())
    #pragma warning(disable: 4359) // The alignment specified for a type is less than the alignment of the type of one of its data members
#endif



//-----------------------------------------------------------------------------------
// ***** OVR_OS
//
#if !defined(OVR_OS_WIN32) && defined(_WIN32)
    #define OVR_OS_WIN32
#endif

#if !defined(OVR_OS_MAC) && defined(__APPLE__)
    #define OVR_OS_MAC
#endif

#if !defined(OVR_OS_LINUX) && defined(__linux__)
    #define OVR_OS_LINUX
#endif



//-----------------------------------------------------------------------------------
// ***** OVR_CPP
//
#if !defined(OVR_CPP)
    #if defined(__cplusplus)
        #define OVR_CPP(x) x
    #else
        #define OVR_CPP(x) /* Not C++ */
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** OVR_CDECL
//
/// LibOVR calling convention for 32-bit Windows builds.
//
#if !defined(OVR_CDECL)
    #if defined(_WIN32)
        #define OVR_CDECL __cdecl
    #else
        #define OVR_CDECL
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** OVR_EXTERN_C
//
/// Defined as extern "C" when built from C++ code.
//
#if !defined(OVR_EXTERN_C)
    #ifdef __cplusplus
        #define OVR_EXTERN_C extern "C"
    #else
        #define OVR_EXTERN_C
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** OVR_PUBLIC_FUNCTION / OVR_PRIVATE_FUNCTION
//
// OVR_PUBLIC_FUNCTION  - Functions that externally visible from a shared library. Corresponds to Microsoft __dllexport.
// OVR_PUBLIC_CLASS     - C++ structs and classes that are externally visible from a shared library. Corresponds to Microsoft __dllexport.
// OVR_PRIVATE_FUNCTION - Functions that are not visible outside of a shared library. They are private to the shared library.
// OVR_PRIVATE_CLASS    - C++ structs and classes that are not visible outside of a shared library. They are private to the shared library.
//
// OVR_DLL_BUILD        - Used to indicate that the current compilation unit is of a shared library.
// OVR_DLL_IMPORT       - Used to indicate that the current compilation unit is a user of the corresponding shared library.
// OVR_STATIC_BUILD     - used to indicate that the current compilation unit is not a shared library but rather statically linked code.
//
#if !defined(OVR_PUBLIC_FUNCTION)
    #if defined(OVR_DLL_BUILD)
        #if defined(_WIN32)
            #define OVR_PUBLIC_FUNCTION(rval) OVR_EXTERN_C __declspec(dllexport) rval OVR_CDECL
            #define OVR_PUBLIC_CLASS          __declspec(dllexport)
            #define OVR_PRIVATE_FUNCTION(rval) rval OVR_CDECL
            #define OVR_PRIVATE_CLASS
        #else
            #define OVR_PUBLIC_FUNCTION(rval) OVR_EXTERN_C __attribute__((visibility("default"))) rval OVR_CDECL /* Requires GCC 4.0+ */
            #define OVR_PUBLIC_CLASS          __attribute__((visibility("default"))) /* Requires GCC 4.0+ */
            #define OVR_PRIVATE_FUNCTION(rval) __attribute__((visibility("hidden"))) rval OVR_CDECL
            #define OVR_PRIVATE_CLASS         __attribute__((visibility("hidden")))
        #endif
    #elif defined(OVR_DLL_IMPORT)
        #if defined(_WIN32)
            #define OVR_PUBLIC_FUNCTION(rval) OVR_EXTERN_C __declspec(dllimport) rval OVR_CDECL
            #define OVR_PUBLIC_CLASS          __declspec(dllimport)
        #else
            #define OVR_PUBLIC_FUNCTION(rval) OVR_EXTERN_C rval OVR_CDECL
            #define OVR_PUBLIC_CLASS
        #endif
        #define OVR_PRIVATE_FUNCTION(rval) rval OVR_CDECL
        #define OVR_PRIVATE_CLASS
    #else // OVR_STATIC_BUILD
        #define OVR_PUBLIC_FUNCTION(rval)     OVR_EXTERN_C rval OVR_CDECL
        #define OVR_PUBLIC_CLASS
        #define OVR_PRIVATE_FUNCTION(rval) rval OVR_CDECL
        #define OVR_PRIVATE_CLASS
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** OVR_EXPORT
//
/// Provided for backward compatibility with older versions of this library.
//
#if !defined(OVR_EXPORT)
    #ifdef OVR_OS_WIN32
        #define OVR_EXPORT __declspec(dllexport)
    #else
        #define OVR_EXPORT
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** OVR_ALIGNAS
//
#if !defined(OVR_ALIGNAS)
    #if defined(__GNUC__) || defined(__clang__)
        #define OVR_ALIGNAS(n) __attribute__((aligned(n)))
    #elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
        #define OVR_ALIGNAS(n) __declspec(align(n))
    #elif defined(__CC_ARM)
        #define OVR_ALIGNAS(n) __align(n)
    #else
        #error Need to define OVR_ALIGNAS
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** OVR_CC_HAS_FEATURE
//
// This is a portable way to use compile-time feature identification available
// with some compilers in a clean way. Direct usage of __has_feature in preprocessing
// statements of non-supporting compilers results in a preprocessing error.
//
// Example usage:
//     #if OVR_CC_HAS_FEATURE(is_pod)
//         if(__is_pod(T)) // If the type is plain data then we can safely memcpy it.
//             memcpy(&destObject, &srcObject, sizeof(object));
//     #endif
//
#if !defined(OVR_CC_HAS_FEATURE)
    #if defined(__clang__) // http://clang.llvm.org/docs/LanguageExtensions.html#id2
        #define OVR_CC_HAS_FEATURE(x) __has_feature(x)
    #else
        #define OVR_CC_HAS_FEATURE(x) 0
    #endif
#endif


// ------------------------------------------------------------------------
// ***** OVR_STATIC_ASSERT
//
// Portable support for C++11 static_assert().
// Acts as if the following were declared:
//     void OVR_STATIC_ASSERT(bool const_expression, const char* msg);
//
// Example usage:
//     OVR_STATIC_ASSERT(sizeof(int32_t) == 4, "int32_t expected to be 4 bytes.");

#if !defined(OVR_STATIC_ASSERT)
    #if !(defined(__cplusplus) && (__cplusplus >= 201103L)) /* Other */ && \
        !(defined(__GXX_EXPERIMENTAL_CXX0X__)) /* GCC */ && \
        !(defined(__clang__) && defined(__cplusplus) && OVR_CC_HAS_FEATURE(cxx_static_assert)) /* clang */ && \
        !(defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(__cplusplus)) /* VS2010+  */

        #if !defined(OVR_SA_UNUSED)
        #if defined(OVR_CC_GNU) || defined(OVR_CC_CLANG)
            #define OVR_SA_UNUSED __attribute__((unused))
        #else
            #define OVR_SA_UNUSED
        #endif
        #define OVR_SA_PASTE(a,b) a##b
        #define OVR_SA_HELP(a,b)  OVR_SA_PASTE(a,b)
        #endif

        #if defined(__COUNTER__)
            #define OVR_STATIC_ASSERT(expression, msg) typedef char OVR_SA_HELP(compileTimeAssert, __COUNTER__) [((expression) != 0) ? 1 : -1] OVR_SA_UNUSED
        #else
            #define OVR_STATIC_ASSERT(expression, msg) typedef char OVR_SA_HELP(compileTimeAssert, __LINE__) [((expression) != 0) ? 1 : -1] OVR_SA_UNUSED
        #endif

    #else
        #define OVR_STATIC_ASSERT(expression, msg) static_assert(expression, msg)
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** Padding
//
/// Defines explicitly unused space for a struct.
/// When used correcly, usage of this macro should not change the size of the struct.
/// Compile-time and runtime behavior with and without this defined should be identical.
///
#if !defined(OVR_UNUSED_STRUCT_PAD)
    #define OVR_UNUSED_STRUCT_PAD(padName, size) char padName[size];
#endif


//-----------------------------------------------------------------------------------
// ***** Word Size
//
/// Specifies the size of a pointer on the given platform.
///
#if !defined(OVR_PTR_SIZE)
    #if defined(__WORDSIZE)
        #define OVR_PTR_SIZE ((__WORDSIZE) / 8)
    #elif defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64) || defined(__ia64__) || defined(__arch64__) || defined(__64BIT__) || defined(__Ptr_Is_64)
        #define OVR_PTR_SIZE 8
    #elif defined(__CC_ARM) && (__sizeof_ptr == 8)
        #define OVR_PTR_SIZE 8
    #else
        #define OVR_PTR_SIZE 4
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** OVR_ON32 / OVR_ON64
//
#if OVR_PTR_SIZE == 8
    #define OVR_ON32(x)
    #define OVR_ON64(x) x
#else
    #define OVR_ON32(x) x
    #define OVR_ON64(x)
#endif


//-----------------------------------------------------------------------------------
// ***** ovrBool

typedef char ovrBool;   ///< Boolean type
#define ovrFalse 0      ///< ovrBool value of false.
#define ovrTrue  1      ///< ovrBool value of true.


//-----------------------------------------------------------------------------------
// ***** Simple Math Structures

/// A 2D vector with integer components.
typedef struct OVR_ALIGNAS(4) ovrVector2i_
{
    int x, y;
} ovrVector2i;

/// A 2D size with integer components.
typedef struct OVR_ALIGNAS(4) ovrSizei_
{
    int w, h;
} ovrSizei;

/// A 2D rectangle with a position and size.
/// All components are integers.
typedef struct OVR_ALIGNAS(4) ovrRecti_
{
    ovrVector2i Pos;
    ovrSizei    Size;
} ovrRecti;

/// A quaternion rotation.
typedef struct OVR_ALIGNAS(4) ovrQuatf_
{
    float x, y, z, w;
} ovrQuatf;

/// A 2D vector with float components.
typedef struct OVR_ALIGNAS(4) ovrVector2f_
{
    float x, y;
} ovrVector2f;

/// A 3D vector with float components.
typedef struct OVR_ALIGNAS(4) ovrVector3f_
{
    float x, y, z;
} ovrVector3f;

/// A 4x4 matrix with float elements.
typedef struct OVR_ALIGNAS(4) ovrMatrix4f_
{
    float M[4][4];
} ovrMatrix4f;


/// Position and orientation together.
typedef struct OVR_ALIGNAS(4) ovrPosef_
{
    ovrQuatf     Orientation;
    ovrVector3f  Position;
} ovrPosef;

/// A full pose (rigid body) configuration with first and second derivatives.
///
/// Body refers to any object for which ovrPoseStatef is providing data.
/// It can be the HMD, Touch controller, sensor or something else. The context 
/// depends on the usage of the struct.
typedef struct OVR_ALIGNAS(8) ovrPoseStatef_
{
    ovrPosef     ThePose;               ///< Position and orientation.
    ovrVector3f  AngularVelocity;       ///< Angular velocity in radians per second.
    ovrVector3f  LinearVelocity;        ///< Velocity in meters per second.
    ovrVector3f  AngularAcceleration;   ///< Angular acceleration in radians per second per second.
    ovrVector3f  LinearAcceleration;    ///< Acceleration in meters per second per second.
    OVR_UNUSED_STRUCT_PAD(pad0, 4)      ///< \internal struct pad.
    double       TimeInSeconds;         ///< Absolute time that this pose refers to. \see ovr_GetTimeInSeconds
} ovrPoseStatef;

/// Describes the up, down, left, and right angles of the field of view.
///
/// Field Of View (FOV) tangent of the angle units.
/// \note For a standard 90 degree vertical FOV, we would
/// have: { UpTan = tan(90 degrees / 2), DownTan = tan(90 degrees / 2) }.
typedef struct OVR_ALIGNAS(4) ovrFovPort_
{
    float UpTan;    ///< The tangent of the angle between the viewing vector and the top edge of the field of view.
    float DownTan;  ///< The tangent of the angle between the viewing vector and the bottom edge of the field of view.
    float LeftTan;  ///< The tangent of the angle between the viewing vector and the left edge of the field of view.
    float RightTan; ///< The tangent of the angle between the viewing vector and the right edge of the field of view.
} ovrFovPort;


//-----------------------------------------------------------------------------------
// ***** HMD Types

/// Enumerates all HMD types that we support.
///
/// The currently released developer kits are ovrHmd_DK1 and ovrHmd_DK2. The other enumerations are for internal use only.
typedef enum ovrHmdType_
{
    ovrHmd_None      = 0,
    ovrHmd_DK1       = 3,
    ovrHmd_DKHD      = 4,
    ovrHmd_DK2       = 6,
    ovrHmd_CB        = 8,
    ovrHmd_Other     = 9,
    ovrHmd_E3_2015   = 10,
    ovrHmd_ES06      = 11,
    ovrHmd_ES09      = 12,
    ovrHmd_ES11      = 13,
    ovrHmd_CV1       = 14,

    ovrHmd_EnumSize  = 0x7fffffff ///< \internal Force type int32_t.
} ovrHmdType;


/// HMD capability bits reported by device.
///
typedef enum ovrHmdCaps_
{
    // Read-only flags
    ovrHmdCap_DebugDevice             = 0x0010,   ///< <B>(read only)</B> Specifies that the HMD is a virtual debug device.


    ovrHmdCap_EnumSize            = 0x7fffffff ///< \internal Force type int32_t.
} ovrHmdCaps;


/// Tracking capability bits reported by the device.
/// Used with ovr_GetTrackingCaps.
typedef enum ovrTrackingCaps_
{
    ovrTrackingCap_Orientation      = 0x0010,    ///< Supports orientation tracking (IMU).
    ovrTrackingCap_MagYawCorrection = 0x0020,    ///< Supports yaw drift correction via a magnetometer or other means.
    ovrTrackingCap_Position         = 0x0040,    ///< Supports positional tracking.
    ovrTrackingCap_EnumSize         = 0x7fffffff ///< \internal Force type int32_t.
} ovrTrackingCaps;


/// Specifies which eye is being used for rendering.
/// This type explicitly does not include a third "NoStereo" monoscopic option, as such is
/// not required for an HMD-centered API.
typedef enum ovrEyeType_
{
    ovrEye_Left     = 0,         ///< The left eye, from the viewer's perspective.
    ovrEye_Right    = 1,         ///< The right eye, from the viewer's perspective.
    ovrEye_Count    = 2,         ///< \internal Count of enumerated elements.
    ovrEye_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} ovrEyeType;

/// Specifies the coordinate system ovrTrackingState returns tracking poses in.
/// Used with ovr_SetTrackingOriginType()
typedef enum ovrTrackingOrigin_
{
    /// \brief Tracking system origin reported at eye (HMD) height
    /// \details Prefer using this origin when your application requires
    /// matching user's current physical head pose to a virtual head pose
    /// without any regards to a the height of the floor. Cockpit-based,
    /// or 3rd-person experiences are ideal candidates.
    /// When used, all poses in ovrTrackingState are reported as an offset
    /// transform from the profile calibrated or recentered HMD pose.
    /// It is recommended that apps using this origin type call ovr_RecenterTrackingOrigin
    /// prior to starting the VR experience, but notify the user before doing so
    /// to make sure the user is in a comfortable pose, facing a comfortable
    /// direction.
    ovrTrackingOrigin_EyeLevel = 0,
    /// \brief Tracking system origin reported at floor height
    /// \details Prefer using this origin when your application requires the
    /// physical floor height to match the virtual floor height, such as
    /// standing experiences.
    /// When used, all poses in ovrTrackingState are reported as an offset
    /// transform from the profile calibrated floor pose. Calling ovr_RecenterTrackingOrigin
    /// will recenter the X & Z axes as well as yaw, but the Y-axis (i.e. height) will continue
    /// to be reported using the floor height as the origin for all poses.
    ovrTrackingOrigin_FloorLevel = 1,
    ovrTrackingOrigin_Count = 2,            ///< \internal Count of enumerated elements.
    ovrTrackingOrigin_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} ovrTrackingOrigin;

/// Identifies a graphics device in a platform-specific way.
/// For Windows this is a LUID type.
typedef struct OVR_ALIGNAS(OVR_PTR_SIZE) ovrGraphicsLuid_
{
    // Public definition reserves space for graphics API-specific implementation
    char        Reserved[8];
} ovrGraphicsLuid;


/// This is a complete descriptor of the HMD.
typedef struct OVR_ALIGNAS(OVR_PTR_SIZE) ovrHmdDesc_
{
    ovrHmdType   Type;                         ///< The type of HMD.
    OVR_ON64(OVR_UNUSED_STRUCT_PAD(pad0, 4))   ///< \internal struct paddding.
    char         ProductName[64];              ///< UTF8-encoded product identification string (e.g. "Oculus Rift DK1").
    char         Manufacturer[64];             ///< UTF8-encoded HMD manufacturer identification string.
    short        VendorId;                     ///< HID (USB) vendor identifier of the device.
    short        ProductId;                    ///< HID (USB) product identifier of the device.
    char         SerialNumber[24];             ///< HMD serial number.
    short        FirmwareMajor;                ///< HMD firmware major version.
    short        FirmwareMinor;                ///< HMD firmware minor version.
    unsigned int AvailableHmdCaps;             ///< Capability bits described by ovrHmdCaps which the HMD currently supports.
    unsigned int DefaultHmdCaps;               ///< Capability bits described by ovrHmdCaps which are default for the current Hmd.
    unsigned int AvailableTrackingCaps;        ///< Capability bits described by ovrTrackingCaps which the system currently supports.
    unsigned int DefaultTrackingCaps;          ///< Capability bits described by ovrTrackingCaps which are default for the current system.
    ovrFovPort   DefaultEyeFov[ovrEye_Count];  ///< Defines the recommended FOVs for the HMD.
    ovrFovPort   MaxEyeFov[ovrEye_Count];      ///< Defines the maximum FOVs for the HMD.
    ovrSizei     Resolution;                   ///< Resolution of the full HMD screen (both eyes) in pixels.
    float        DisplayRefreshRate;           ///< Nominal refresh rate of the display in cycles per second at the time of HMD creation.
    OVR_ON64(OVR_UNUSED_STRUCT_PAD(pad1, 4))   ///< \internal struct paddding.
} ovrHmdDesc;


/// Used as an opaque pointer to an OVR session.
typedef struct ovrHmdStruct* ovrSession;



/// Bit flags describing the current status of sensor tracking.
///  The values must be the same as in enum StatusBits
///
/// \see ovrTrackingState
///
typedef enum ovrStatusBits_
{
    ovrStatus_OrientationTracked    = 0x0001,    ///< Orientation is currently tracked (connected and in use).
    ovrStatus_PositionTracked       = 0x0002,    ///< Position is currently tracked (false if out of range).
    ovrStatus_EnumSize              = 0x7fffffff ///< \internal Force type int32_t.
} ovrStatusBits;


///  Specifies the description of a single sensor.
///
/// \see ovr_GetTrackerDesc
///
typedef struct OVR_ALIGNAS(OVR_PTR_SIZE) ovrTrackerDesc_
{
    float FrustumHFovInRadians;      ///< Sensor frustum horizontal field-of-view (if present).
    float FrustumVFovInRadians;      ///< Sensor frustum vertical field-of-view (if present).
    float FrustumNearZInMeters;      ///< Sensor frustum near Z (if present).
    float FrustumFarZInMeters;       ///< Sensor frustum far Z (if present).
} ovrTrackerDesc;


///  Specifies sensor flags.
///
///  /see ovrTrackerPose
///
typedef enum ovrTrackerFlags_
{
    ovrTracker_Connected   = 0x0020,      ///< The sensor is present, else the sensor is absent or offline.
    ovrTracker_PoseTracked = 0x0004       ///< The sensor has a valid pose, else the pose is unavailable. This will only be set if ovrTracker_Connected is set.
} ovrTrackerFlags;


///  Specifies the pose for a single sensor.
///
typedef struct OVR_ALIGNAS(8) _ovrTrackerPose
{
    unsigned int TrackerFlags;      ///< ovrTrackerFlags.
    ovrPosef     Pose;              ///< The sensor's pose. This pose includes sensor tilt (roll and pitch). For a leveled coordinate system use LeveledPose.
    ovrPosef     LeveledPose;       ///< The sensor's leveled pose, aligned with gravity. This value includes position and yaw of the sensor, but not roll and pitch. It can be used as a reference point to render real-world objects in the correct location.
    OVR_UNUSED_STRUCT_PAD(pad0, 4)  ///< \internal struct pad.
} ovrTrackerPose;


/// Tracking state at a given absolute time (describes predicted HMD pose, etc.).
/// Returned by ovr_GetTrackingState.
///
/// \see ovr_GetTrackingState
///
typedef struct OVR_ALIGNAS(8) ovrTrackingState_
{
    /// Predicted head pose (and derivatives) at the requested absolute time.
    ovrPoseStatef  HeadPose;

    /// HeadPose tracking status described by ovrStatusBits.
    unsigned int   StatusFlags;

    /// The most recent calculated pose for each hand when hand controller tracking is present.
    /// HandPoses[ovrHand_Left] refers to the left hand and HandPoses[ovrHand_Right] to the right hand.
    /// These values can be combined with ovrInputState for complete hand controller information.
    ovrPoseStatef  HandPoses[2];

    /// HandPoses status flags described by ovrStatusBits.
    /// Only ovrStatus_OrientationTracked and ovrStatus_PositionTracked are reported.
    unsigned int   HandStatusFlags[2];

    /// The pose of the origin captured during calibration.
    /// Like all other poses here, this is expressed in the space set by ovr_RecenterTrackingOrigin,
    /// and so will change every time that is called. This pose can be used to calculate
    /// where the calibrated origin lands in the new recentered space.
    /// If an application never calls ovr_RecenterTrackingOrigin, expect this value to be the identity
    /// pose and as such will point respective origin based on ovrTrackingOrigin requested when
    /// calling ovr_GetTrackingState.
    ovrPosef      CalibratedOrigin;

} ovrTrackingState;


/// Rendering information for each eye. Computed by ovr_GetRenderDesc() based on the
/// specified FOV. Note that the rendering viewport is not included
/// here as it can be specified separately and modified per frame by
/// passing different Viewport values in the layer structure.
///
/// \see ovr_GetRenderDesc
///
typedef struct OVR_ALIGNAS(4) ovrEyeRenderDesc_
{
    ovrEyeType  Eye;                        ///< The eye index to which this instance corresponds.
    ovrFovPort  Fov;                        ///< The field of view.
    ovrRecti    DistortedViewport;          ///< Distortion viewport.
    ovrVector2f PixelsPerTanAngleAtCenter;  ///< How many display pixels will fit in tan(angle) = 1.
    ovrVector3f HmdToEyeOffset;             ///< Translation of each eye, in meters.
} ovrEyeRenderDesc;


/// Projection information for ovrLayerEyeFovDepth.
///
/// Use the utility function ovrTimewarpProjectionDesc_FromProjection to
/// generate this structure from the application's projection matrix.
///
/// \see ovrLayerEyeFovDepth, ovrTimewarpProjectionDesc_FromProjection
///
typedef struct OVR_ALIGNAS(4) ovrTimewarpProjectionDesc_
{
    float Projection22;     ///< Projection matrix element [2][2].
    float Projection23;     ///< Projection matrix element [2][3].
    float Projection32;     ///< Projection matrix element [3][2].
} ovrTimewarpProjectionDesc;


/// Contains the data necessary to properly calculate position info for various layer types.
/// - HmdToEyeOffset is the same value pair provided in ovrEyeRenderDesc.
/// - HmdSpaceToWorldScaleInMeters is used to scale player motion into in-application units.
///   In other words, it is how big an in-application unit is in the player's physical meters.
///   For example, if the application uses inches as its units then HmdSpaceToWorldScaleInMeters would be 0.0254.
///   Note that if you are scaling the player in size, this must also scale. So if your application
///   units are inches, but you're shrinking the player to half their normal size, then
///   HmdSpaceToWorldScaleInMeters would be 0.0254*2.0.
///
/// \see ovrEyeRenderDesc, ovr_SubmitFrame
///
typedef struct OVR_ALIGNAS(4) ovrViewScaleDesc_
{
    ovrVector3f HmdToEyeOffset[ovrEye_Count];   ///< Translation of each eye.
    float       HmdSpaceToWorldScaleInMeters;   ///< Ratio of viewer units to meter units.
} ovrViewScaleDesc;


//-----------------------------------------------------------------------------------
// ***** Platform-independent Rendering Configuration

/// The type of texture resource.
///
/// \see ovrTextureSwapChainDesc
///
typedef enum ovrTextureType_
{
    ovrTexture_2D,              ///< 2D textures.
    ovrTexture_2D_External,     ///< External 2D texture. Not used on PC
    ovrTexture_Cube,            ///< Cube maps. Not currently supported on PC.
    ovrTexture_Count,
    ovrTexture_EnumSize = 0x7fffffff  ///< \internal Force type int32_t.
} ovrTextureType;

/// The bindings required for texture swap chain.
///
/// All texture swap chains are automatically bindable as shader
/// input resources since the Oculus runtime needs this to read them.
///
/// \see ovrTextureSwapChainDesc
///
typedef enum ovrTextureBindFlags_
{
    ovrTextureBind_None,
    ovrTextureBind_DX_RenderTarget = 0x0001,    ///< The application can write into the chain with pixel shader
    ovrTextureBind_DX_UnorderedAccess = 0x0002, ///< The application can write to the chain with compute shader
    ovrTextureBind_DX_DepthStencil = 0x0004,    ///< The chain buffers can be bound as depth and/or stencil buffers

    ovrTextureBind_EnumSize = 0x7fffffff  ///< \internal Force type int32_t.
} ovrTextureBindFlags;

/// The format of a texture.
///
/// \see ovrTextureSwapChainDesc
///
typedef enum ovrTextureFormat_
{
    OVR_FORMAT_UNKNOWN,
    OVR_FORMAT_B5G6R5_UNORM,    ///< Not currently supported on PC. Would require a DirectX 11.1 device.
    OVR_FORMAT_B5G5R5A1_UNORM,  ///< Not currently supported on PC. Would require a DirectX 11.1 device.
    OVR_FORMAT_B4G4R4A4_UNORM,  ///< Not currently supported on PC. Would require a DirectX 11.1 device.
    OVR_FORMAT_R8G8B8A8_UNORM,
    OVR_FORMAT_R8G8B8A8_UNORM_SRGB,
    OVR_FORMAT_B8G8R8A8_UNORM,
    OVR_FORMAT_B8G8R8A8_UNORM_SRGB, ///< Not supported for OpenGL applications
    OVR_FORMAT_B8G8R8X8_UNORM,      ///< Not supported for OpenGL applications
    OVR_FORMAT_B8G8R8X8_UNORM_SRGB, ///< Not supported for OpenGL applications
    OVR_FORMAT_R16G16B16A16_FLOAT,
    OVR_FORMAT_D16_UNORM,
    OVR_FORMAT_D24_UNORM_S8_UINT,
    OVR_FORMAT_D32_FLOAT,
    OVR_FORMAT_D32_FLOAT_S8X24_UINT,

    // Added in 1.5 compressed formats can be used for static layers
    OVR_FORMAT_BC1_UNORM,
    OVR_FORMAT_BC1_UNORM_SRGB,
    OVR_FORMAT_BC2_UNORM,
    OVR_FORMAT_BC2_UNORM_SRGB,
    OVR_FORMAT_BC3_UNORM,
    OVR_FORMAT_BC3_UNORM_SRGB,
    OVR_FORMAT_BC6H_UF16,
    OVR_FORMAT_BC6H_SF16,
    OVR_FORMAT_BC7_UNORM,
    OVR_FORMAT_BC7_UNORM_SRGB,

    OVR_FORMAT_ENUMSIZE = 0x7fffffff  ///< \internal Force type int32_t.
} ovrTextureFormat;

/// Misc flags overriding particular
///   behaviors of a texture swap chain
///
/// \see ovrTextureSwapChainDesc
///
typedef enum ovrTextureMiscFlags_
{
    ovrTextureMisc_None, 

    /// DX only: The underlying texture is created with a TYPELESS equivalent of the
    /// format specified in the texture desc. The SDK will still access the
    /// texture using the format specified in the texture desc, but the app can
    /// create views with different formats if this is specified.
    ovrTextureMisc_DX_Typeless = 0x0001,

    /// DX only: Allow generation of the mip chain on the GPU via the GenerateMips
    /// call. This flag requires that RenderTarget binding also be specified.
    ovrTextureMisc_AllowGenerateMips = 0x0002,

    /// Texture swap chain contains protected content, and requires
    /// HDCP connection in order to display to HMD. Also prevents
    /// mirroring or other redirection of any frame containing this contents
    ovrTextureMisc_ProtectedContent = 0x0004,

    ovrTextureMisc_EnumSize = 0x7fffffff  ///< \internal Force type int32_t.
} ovrTextureFlags;

/// Description used to create a texture swap chain.
///
/// \see ovr_CreateTextureSwapChainDX
/// \see ovr_CreateTextureSwapChainGL
///
typedef struct ovrTextureSwapChainDesc_
{
    ovrTextureType      Type;
    ovrTextureFormat    Format;
    int                 ArraySize;      ///< Only supported with ovrTexture_2D. Not supported on PC at this time.
    int                 Width;
    int                 Height;
    int                 MipLevels;
    int                 SampleCount;    ///< Current only supported on depth textures
    ovrBool             StaticImage;    ///< Not buffered in a chain. For images that don't change
    unsigned int        MiscFlags;      ///< ovrTextureFlags
    unsigned int        BindFlags;      ///< ovrTextureBindFlags. Not used for GL.
} ovrTextureSwapChainDesc;

/// Description used to create a mirror texture.
///
/// \see ovr_CreateMirrorTextureDX
/// \see ovr_CreateMirrorTextureGL
///
typedef struct ovrMirrorTextureDesc_
{
    ovrTextureFormat    Format;
    int                 Width;
    int                 Height;
    unsigned int        MiscFlags;      ///< ovrTextureFlags
} ovrMirrorTextureDesc;

typedef struct ovrTextureSwapChainData* ovrTextureSwapChain;
typedef struct ovrMirrorTextureData* ovrMirrorTexture;

//-----------------------------------------------------------------------------------

/// Describes button input types.
/// Button inputs are combined; that is they will be reported as pressed if they are 
/// pressed on either one of the two devices.
/// The ovrButton_Up/Down/Left/Right map to both XBox D-Pad and directional buttons.
/// The ovrButton_Enter and ovrButton_Return map to Start and Back controller buttons, respectively.
typedef enum ovrButton_
{    
    ovrButton_A         = 0x00000001,
    ovrButton_B         = 0x00000002,
    ovrButton_RThumb    = 0x00000004,
    ovrButton_RShoulder = 0x00000008,

    // Bit mask of all buttons on the right Touch controller
    ovrButton_RMask     = ovrButton_A | ovrButton_B | ovrButton_RThumb | ovrButton_RShoulder,

    ovrButton_X         = 0x00000100,
    ovrButton_Y         = 0x00000200,
    ovrButton_LThumb    = 0x00000400,  
    ovrButton_LShoulder = 0x00000800,

    // Bit mask of all buttons on the left Touch controller
    ovrButton_LMask     = ovrButton_X | ovrButton_Y | ovrButton_LThumb | ovrButton_LShoulder,

    // Navigation through DPad.
    ovrButton_Up        = 0x00010000,
    ovrButton_Down      = 0x00020000,
    ovrButton_Left      = 0x00040000,
    ovrButton_Right     = 0x00080000,
    ovrButton_Enter     = 0x00100000, // Start on XBox controller.
    ovrButton_Back      = 0x00200000, // Back on Xbox controller.
    ovrButton_VolUp     = 0x00400000,  // only supported by Remote.
    ovrButton_VolDown   = 0x00800000,  // only supported by Remote.
    ovrButton_Home      = 0x01000000,  
    ovrButton_Private   = ovrButton_VolUp | ovrButton_VolDown | ovrButton_Home,


    ovrButton_EnumSize  = 0x7fffffff ///< \internal Force type int32_t.
} ovrButton;

/// Describes touch input types.
/// These values map to capacitive touch values reported ovrInputState::Touch.
/// Some of these values are mapped to button bits for consistency.
typedef enum ovrTouch_
{
    ovrTouch_A              = ovrButton_A,
    ovrTouch_B              = ovrButton_B,
    ovrTouch_RThumb         = ovrButton_RThumb,
    ovrTouch_RThumbRest     = 0x00000008,
    ovrTouch_RIndexTrigger  = 0x00000010,

    // Bit mask of all the button touches on the right controller
    ovrTouch_RButtonMask    = ovrTouch_A | ovrTouch_B | ovrTouch_RThumb | ovrTouch_RThumbRest | ovrTouch_RIndexTrigger,

    ovrTouch_X              = ovrButton_X,
    ovrTouch_Y              = ovrButton_Y,
    ovrTouch_LThumb         = ovrButton_LThumb,
    ovrTouch_LThumbRest     = 0x00000800,
    ovrTouch_LIndexTrigger  = 0x00001000,

    // Bit mask of all the button touches on the left controller
    ovrTouch_LButtonMask    = ovrTouch_X | ovrTouch_Y | ovrTouch_LThumb | ovrTouch_LThumbRest | ovrTouch_LIndexTrigger,

    // Finger pose state 
    // Derived internally based on distance, proximity to sensors and filtering.
    ovrTouch_RIndexPointing = 0x00000020,
    ovrTouch_RThumbUp       = 0x00000040,

    // Bit mask of all right controller poses
    ovrTouch_RPoseMask      = ovrTouch_RIndexPointing | ovrTouch_RThumbUp,

    ovrTouch_LIndexPointing = 0x00002000,
    ovrTouch_LThumbUp       = 0x00004000,

    // Bit mask of all left controller poses
    ovrTouch_LPoseMask      = ovrTouch_LIndexPointing | ovrTouch_LThumbUp,

    ovrTouch_EnumSize       = 0x7fffffff ///< \internal Force type int32_t.
} ovrTouch;

/// Specifies which controller is connected; multiple can be connected at once.
typedef enum ovrControllerType_
{
    ovrControllerType_None      = 0x00,
    ovrControllerType_LTouch    = 0x01,
    ovrControllerType_RTouch    = 0x02,
    ovrControllerType_Touch     = 0x03,
    ovrControllerType_Remote    = 0x04,
    ovrControllerType_XBox      = 0x10,

    ovrControllerType_Active    = 0xff,      ///< Operate on or query whichever controller is active.

    ovrControllerType_EnumSize  = 0x7fffffff ///< \internal Force type int32_t.
} ovrControllerType;


/// Provides names for the left and right hand array indexes.
///
/// \see ovrInputState, ovrTrackingState
/// 
typedef enum ovrHandType_
{
    ovrHand_Left  = 0,
    ovrHand_Right = 1,
    ovrHand_Count = 2,
    ovrHand_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} ovrHandType;



/// ovrInputState describes the complete controller input state, including Oculus Touch,
/// and XBox gamepad. If multiple inputs are connected and used at the same time,
/// their inputs are combined.
typedef struct ovrInputState_
{
    // System type when the controller state was last updated.
    double              TimeInSeconds;

    // Values for buttons described by ovrButton.
    unsigned int        Buttons;

    // Touch values for buttons and sensors as described by ovrTouch.
    unsigned int        Touches;
    
    // Left and right finger trigger values (ovrHand_Left and ovrHand_Right), in the range 0.0 to 1.0f.
    float               IndexTrigger[ovrHand_Count];
    
    // Left and right hand trigger values (ovrHand_Left and ovrHand_Right), in the range 0.0 to 1.0f.
    float               HandTrigger[ovrHand_Count];

    // Horizontal and vertical thumbstick axis values (ovrHand_Left and ovrHand_Right), in the range -1.0f to 1.0f.
    ovrVector2f         Thumbstick[ovrHand_Count];

    // The type of the controller this state is for.
    ovrControllerType   ControllerType;
    
} ovrInputState;



//-----------------------------------------------------------------------------------
// ***** Initialize structures

/// Initialization flags.
///
/// \see ovrInitParams, ovr_Initialize
///
typedef enum ovrInitFlags_
{
    /// When a debug library is requested, a slower debugging version of the library will
    /// run which can be used to help solve problems in the library and debug application code.
    ovrInit_Debug          = 0x00000001,

    /// When a version is requested, the LibOVR runtime respects the RequestedMinorVersion
    /// field and verifies that the RequestedMinorVersion is supported.
    ovrInit_RequestVersion = 0x00000004,

    // These bits are writable by user code.
    ovrinit_WritableBits   = 0x00ffffff,

    ovrInit_EnumSize       = 0x7fffffff ///< \internal Force type int32_t.
} ovrInitFlags;


/// Logging levels
///
/// \see ovrInitParams, ovrLogCallback
///
typedef enum ovrLogLevel_
{
    ovrLogLevel_Debug    = 0, ///< Debug-level log event.
    ovrLogLevel_Info     = 1, ///< Info-level log event.
    ovrLogLevel_Error    = 2, ///< Error-level log event.

    ovrLogLevel_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} ovrLogLevel;


/// Signature of the logging callback function pointer type.
///
/// \param[in] userData is an arbitrary value specified by the user of ovrInitParams.
/// \param[in] level is one of the ovrLogLevel constants.
/// \param[in] message is a UTF8-encoded null-terminated string.
/// \see ovrInitParams, ovrLogLevel, ovr_Initialize
///
typedef void (OVR_CDECL* ovrLogCallback)(uintptr_t userData, int level, const char* message);


/// Parameters for ovr_Initialize.
///
/// \see ovr_Initialize
///
typedef struct OVR_ALIGNAS(8) ovrInitParams_
{
    /// Flags from ovrInitFlags to override default behavior.
    /// Use 0 for the defaults.
    uint32_t       Flags;

    /// Requests a specific minimum minor version of the LibOVR runtime.
    /// Flags must include ovrInit_RequestVersion or this will be ignored
    /// and OVR_MINOR_VERSION will be used.
    uint32_t       RequestedMinorVersion;

    /// User-supplied log callback function, which may be called at any time
    /// asynchronously from multiple threads until ovr_Shutdown completes.
    /// Use NULL to specify no log callback.
    ovrLogCallback LogCallback;

    /// User-supplied data which is passed as-is to LogCallback. Typically this 
    /// is used to store an application-specific pointer which is read in the 
    /// callback function.
    uintptr_t      UserData;

    /// Relative number of milliseconds to wait for a connection to the server
    /// before failing. Use 0 for the default timeout.
    uint32_t       ConnectionTimeoutMS;

    OVR_ON64(OVR_UNUSED_STRUCT_PAD(pad0, 4)) ///< \internal

} ovrInitParams;


#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------
// ***** API Interfaces

/// Initializes LibOVR
///
/// Initialize LibOVR for application usage. This includes finding and loading the LibOVRRT
/// shared library. No LibOVR API functions, other than ovr_GetLastErrorInfo and ovr_Detect, can
/// be called unless ovr_Initialize succeeds. A successful call to ovr_Initialize must be eventually
/// followed by a call to ovr_Shutdown. ovr_Initialize calls are idempotent.
/// Calling ovr_Initialize twice does not require two matching calls to ovr_Shutdown.
/// If already initialized, the return value is ovr_Success.
/// 
/// LibOVRRT shared library search order:
///      -# Current working directory (often the same as the application directory).
///      -# Module directory (usually the same as the application directory,
///         but not if the module is a separate shared library).
///      -# Application directory
///      -# Development directory (only if OVR_ENABLE_DEVELOPER_SEARCH is enabled,
///         which is off by default).
///      -# Standard OS shared library search location(s) (OS-specific).
///
/// \param params Specifies custom initialization options. May be NULL to indicate default options.
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information. Example failed results include:
///     - ovrError_Initialize: Generic initialization error.
///     - ovrError_LibLoad: Couldn't load LibOVRRT.
///     - ovrError_LibVersion: LibOVRRT version incompatibility.
///     - ovrError_ServiceConnection: Couldn't connect to the OVR Service.
///     - ovrError_ServiceVersion: OVR Service version incompatibility.
///     - ovrError_IncompatibleOS: The operating system version is incompatible.
///     - ovrError_DisplayInit: Unable to initialize the HMD display.
///     - ovrError_ServerStart:  Unable to start the server. Is it already running?
///     - ovrError_Reinitialization: Attempted to re-initialize with a different version.
///
/// <b>Example code</b>
///     \code{.cpp}
///         ovrResult result = ovr_Initialize(NULL);
///         if(OVR_FAILURE(result)) {
///             ovrErrorInfo errorInfo;
///             ovr_GetLastErrorInfo(&errorInfo);
///             DebugLog("ovr_Initialize failed: %s", errorInfo.ErrorString);
///             return false;
///         }
///         [...]
///     \endcode
///
/// \see ovr_Shutdown
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_Initialize(const ovrInitParams* params);


/// Shuts down LibOVR
///
/// A successful call to ovr_Initialize must be eventually matched by a call to ovr_Shutdown.
/// After calling ovr_Shutdown, no LibOVR functions can be called except ovr_GetLastErrorInfo
/// or another ovr_Initialize. ovr_Shutdown invalidates all pointers, references, and created objects
/// previously returned by LibOVR functions. The LibOVRRT shared library can be unloaded by
/// ovr_Shutdown.
///
/// \see ovr_Initialize
///
OVR_PUBLIC_FUNCTION(void) ovr_Shutdown();

/// Returns information about the most recent failed return value by the
/// current thread for this library.
///
/// This function itself can never generate an error.
/// The last error is never cleared by LibOVR, but will be overwritten by new errors.
/// Do not use this call to determine if there was an error in the last API
/// call as successful API calls don't clear the last ovrErrorInfo.
/// To avoid any inconsistency, ovr_GetLastErrorInfo should be called immediately
/// after an API function that returned a failed ovrResult, with no other API
/// functions called in the interim.
///
/// \param[out] errorInfo The last ovrErrorInfo for the current thread.
///
/// \see ovrErrorInfo
///
OVR_PUBLIC_FUNCTION(void) ovr_GetLastErrorInfo(ovrErrorInfo* errorInfo);


/// Returns the version string representing the LibOVRRT version.
///
/// The returned string pointer is valid until the next call to ovr_Shutdown.
///
/// Note that the returned version string doesn't necessarily match the current
/// OVR_MAJOR_VERSION, etc., as the returned string refers to the LibOVRRT shared
/// library version and not the locally compiled interface version.
///
/// The format of this string is subject to change in future versions and its contents
/// should not be interpreted.
///
/// \return Returns a UTF8-encoded null-terminated version string.
///
OVR_PUBLIC_FUNCTION(const char*) ovr_GetVersionString();


/// Writes a message string to the LibOVR tracing mechanism (if enabled).
///
/// This message will be passed back to the application via the ovrLogCallback if
/// it was registered.
///
/// \param[in] level One of the ovrLogLevel constants.
/// \param[in] message A UTF8-encoded null-terminated string.
/// \return returns the strlen of the message or a negative value if the message is too large.
///
/// \see ovrLogLevel, ovrLogCallback
///
OVR_PUBLIC_FUNCTION(int) ovr_TraceMessage(int level, const char* message);


/// Identify client application info.
///
/// The string is one or more newline-delimited lines of optional info
/// indicating engine name, engine version, engine plugin name, engine plugin
/// version, engine editor. The order of the lines is not relevant. Individual
/// lines are optional. A newline is not necessary at the end of the last line.
/// Call after ovr_Initialize and before the first call to ovr_Create.
/// Each value is limited to 20 characters. Key names such as 'EngineName:'
/// 'EngineVersion:' do not count towards this limit.
///
/// \param[in] identity Specifies one or more newline-delimited lines of optional info:
///             EngineName: %s\n
///             EngineVersion: %s\n
///             EnginePluginName: %s\n
///             EnginePluginVersion: %s\n
///             EngineEditor: <boolean> ('true' or 'false')\n
///
/// <b>Example code</b>
///     \code{.cpp}
///     ovr_IdentifyClient("EngineName: Unity\n"
///                        "EngineVersion: 5.3.3\n"
///                        "EnginePluginName: OVRPlugin\n"
///                        "EnginePluginVersion: 1.2.0\n"
///                        "EngineEditor: true");
///     \endcode
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_IdentifyClient(const char* identity);


//-------------------------------------------------------------------------------------
/// @name HMD Management
///
/// Handles the enumeration, creation, destruction, and properties of an HMD (head-mounted display).
///@{


/// Returns information about the current HMD.
///
/// ovr_Initialize must have first been called in order for this to succeed, otherwise ovrHmdDesc::Type
/// will be reported as ovrHmd_None.
/// 
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create, else NULL in which
///                case this function detects whether an HMD is present and returns its info if so.
///
/// \return Returns an ovrHmdDesc. If the hmd is NULL and ovrHmdDesc::Type is ovrHmd_None then 
///         no HMD is present.
///
OVR_PUBLIC_FUNCTION(ovrHmdDesc) ovr_GetHmdDesc(ovrSession session);


/// Returns the number of sensors. 
///
/// The number of sensors may change at any time, so this function should be called before use 
/// as opposed to once on startup.
/// 
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
///
/// \return Returns unsigned int count.
///
OVR_PUBLIC_FUNCTION(unsigned int) ovr_GetTrackerCount(ovrSession session);


/// Returns a given sensor description.
///
/// It's possible that sensor desc [0] may indicate a unconnnected or non-pose tracked sensor, but 
/// sensor desc [1] may be connected.
///
/// ovr_Initialize must have first been called in order for this to succeed, otherwise the returned
/// trackerDescArray will be zero-initialized. The data returned by this function can change at runtime.
/// 
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// 
/// \param[in] trackerDescIndex Specifies a sensor index. The valid indexes are in the range of 0 to 
///            the sensor count returned by ovr_GetTrackerCount.
///
/// \return Returns ovrTrackerDesc. An empty ovrTrackerDesc will be returned if trackerDescIndex is out of range.
///
/// \see ovrTrackerDesc, ovr_GetTrackerCount
///
OVR_PUBLIC_FUNCTION(ovrTrackerDesc) ovr_GetTrackerDesc(ovrSession session, unsigned int trackerDescIndex);


/// Creates a handle to a VR session.
///
/// Upon success the returned ovrSession must be eventually freed with ovr_Destroy when it is no longer needed.
/// A second call to ovr_Create will result in an error return value if the previous session has not been destroyed.
///
/// \param[out] pSession Provides a pointer to an ovrSession which will be written to upon success.
/// \param[out] luid Provides a system specific graphics adapter identifier that locates which
/// graphics adapter has the HMD attached. This must match the adapter used by the application
/// or no rendering output will be possible. This is important for stability on multi-adapter systems. An
/// application that simply chooses the default adapter will not run reliably on multi-adapter systems.
/// \return Returns an ovrResult indicating success or failure. Upon failure
///         the returned ovrSession will be NULL.
///
/// <b>Example code</b>
///     \code{.cpp}
///         ovrSession session;
///         ovrGraphicsLuid luid;
///         ovrResult result = ovr_Create(&session, &luid);
///         if(OVR_FAILURE(result))
///            ...
///     \endcode
///
/// \see ovr_Destroy
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_Create(ovrSession* pSession, ovrGraphicsLuid* pLuid);


/// Destroys the session.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \see ovr_Create
///
OVR_PUBLIC_FUNCTION(void) ovr_Destroy(ovrSession session);


/// Specifies status information for the current session.
///
/// \see ovr_GetSessionStatus
///
typedef struct ovrSessionStatus_
{
    ovrBool IsVisible;    ///< True if the process has VR focus and thus is visible in the HMD.
    ovrBool HmdPresent;   ///< True if an HMD is present.
    ovrBool HmdMounted;   ///< True if the HMD is on the user's head.
    ovrBool DisplayLost;  ///< True if the session is in a display-lost state. See ovr_SubmitFrame.
    ovrBool ShouldQuit;   ///< True if the application should initiate shutdown.    
    ovrBool ShouldRecenter;  ///< True if UX has requested re-centering. Must call ovr_ClearShouldRecenterFlag or ovr_RecenterTrackingOrigin.
}ovrSessionStatus;


/// Returns status information for the application.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[out] sessionStatus Provides an ovrSessionStatus that is filled in.
///
/// \return Returns an ovrResult indicating success or failure. In the case of
///         failure, use ovr_GetLastErrorInfo to get more information.
//          Return values include but aren't limited to:
///     - ovrSuccess: Completed successfully.
///     - ovrError_ServiceConnection: The service connection was lost and the application
//        must destroy the session.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetSessionStatus(ovrSession session, ovrSessionStatus* sessionStatus);


//@}



//-------------------------------------------------------------------------------------
/// @name Tracking
///
/// Tracking functions handle the position, orientation, and movement of the HMD in space.
///
/// All tracking interface functions are thread-safe, allowing tracking state to be sampled
/// from different threads.
///
///@{



/// Sets the tracking origin type
///
/// When the tracking origin is changed, all of the calls that either provide
/// or accept ovrPosef will use the new tracking origin provided.
/// 
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] origin Specifies an ovrTrackingOrigin to be used for all ovrPosef
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
/// \see ovrTrackingOrigin, ovr_GetTrackingOriginType
OVR_PUBLIC_FUNCTION(ovrResult) ovr_SetTrackingOriginType(ovrSession session, ovrTrackingOrigin origin);


/// Gets the tracking origin state
/// 
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
///
/// \return Returns the ovrTrackingOrigin that was either set by default, or previous set by the application.
///
/// \see ovrTrackingOrigin, ovr_SetTrackingOriginType
OVR_PUBLIC_FUNCTION(ovrTrackingOrigin) ovr_GetTrackingOriginType(ovrSession session);


/// Re-centers the sensor position and orientation.
///
/// This resets the (x,y,z) positional components and the yaw orientation component.
/// The Roll and pitch orientation components are always determined by gravity and cannot
/// be redefined. All future tracking will report values relative to this new reference position.
/// If you are using ovrTrackerPoses then you will need to call ovr_GetTrackerPose after 
/// this, because the sensor position(s) will change as a result of this.
/// 
/// The headset cannot be facing vertically upward or downward but rather must be roughly
/// level otherwise this function will fail with ovrError_InvalidHeadsetOrientation.
///
/// For more info, see the notes on each ovrTrackingOrigin enumeration to understand how
/// recenter will vary slightly in its behavior based on the current ovrTrackingOrigin setting.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information. Return values include but aren't limited to:
///     - ovrSuccess: Completed successfully.
///     - ovrError_InvalidHeadsetOrientation: The headset was facing an invalid direction when
///       attempting recentering, such as facing vertically.
///
/// \see ovrTrackingOrigin, ovr_GetTrackerPose
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_RecenterTrackingOrigin(ovrSession session);


/// Clears the ShouldRecenter status bit in ovrSessionStatus.
///
/// Clears the ShouldRecenter status bit in ovrSessionStatus, allowing further recenter 
/// requests to be detected. Since this is automatically done by ovr_RecenterTrackingOrigin,
/// this is only needs to be called when application is doing its own re-centering.
OVR_PUBLIC_FUNCTION(void) ovr_ClearShouldRecenterFlag(ovrSession session);


/// Returns tracking state reading based on the specified absolute system time.
///
/// Pass an absTime value of 0.0 to request the most recent sensor reading. In this case
/// both PredictedPose and SamplePose will have the same value.
///
/// This may also be used for more refined timing of front buffer rendering logic, and so on.
/// This may be called by multiple threads.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] absTime Specifies the absolute future time to predict the return
///            ovrTrackingState value. Use 0 to request the most recent tracking state.
/// \param[in] latencyMarker Specifies that this call is the point in time where
///            the "App-to-Mid-Photon" latency timer starts from. If a given ovrLayer
///            provides "SensorSampleTime", that will override the value stored here.
/// \return Returns the ovrTrackingState that is predicted for the given absTime.
///
/// \see ovrTrackingState, ovr_GetEyePoses, ovr_GetTimeInSeconds
///
OVR_PUBLIC_FUNCTION(ovrTrackingState) ovr_GetTrackingState(ovrSession session, double absTime, ovrBool latencyMarker);



/// Returns the ovrTrackerPose for the given sensor.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] trackerPoseIndex Index of the sensor being requested.
///
/// \return Returns the requested ovrTrackerPose. An empty ovrTrackerPose will be returned if trackerPoseIndex is out of range.
///
/// \see ovr_GetTrackerCount
///
OVR_PUBLIC_FUNCTION(ovrTrackerPose) ovr_GetTrackerPose(ovrSession session, unsigned int trackerPoseIndex);



/// Returns the most recent input state for controllers, without positional tracking info.
///
/// \param[out] inputState Input state that will be filled in.
/// \param[in] ovrControllerType Specifies which controller the input will be returned for.
/// \return Returns ovrSuccess if the new state was successfully obtained.
///
/// \see ovrControllerType
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetInputState(ovrSession session, ovrControllerType controllerType, ovrInputState* inputState);


/// Returns controller types connected to the system OR'ed together.
///
/// \return A bitmask of ovrControllerTypes connected to the system.
///
/// \see ovrControllerType
///
OVR_PUBLIC_FUNCTION(unsigned int) ovr_GetConnectedControllerTypes(ovrSession session);


/// Turns on vibration of the given controller.
///
/// To disable vibration, call ovr_SetControllerVibration with an amplitude of 0.
/// Vibration automatically stops after a nominal amount of time, so if you want vibration 
/// to be continuous over multiple seconds then you need to call this function periodically.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] controllerType Specifies the controller to apply the vibration to.
/// \param[in] frequency Specifies a vibration frequency in the range of 0.0 to 1.0. 
///            Currently the only valid values are 0.0, 0.5, and 1.0 and other values will
///            be clamped to one of these.
/// \param[in] amplitude Specifies a vibration amplitude in the range of 0.0 to 1.0.
///
/// \return Returns ovrSuccess upon success.
///
/// \see ovrControllerType
/// 
OVR_PUBLIC_FUNCTION(ovrResult) ovr_SetControllerVibration(ovrSession session, ovrControllerType controllerType, float frequency, float amplitude);


///@}

//-------------------------------------------------------------------------------------
// @name Layers
//
///@{


///  Specifies the maximum number of layers supported by ovr_SubmitFrame.
///
///  /see ovr_SubmitFrame
///
enum {
    ovrMaxLayerCount = 16
};

/// Describes layer types that can be passed to ovr_SubmitFrame.
/// Each layer type has an associated struct, such as ovrLayerEyeFov.
///
/// \see ovrLayerHeader
///
typedef enum ovrLayerType_
{
    ovrLayerType_Disabled    = 0,         ///< Layer is disabled.
    ovrLayerType_EyeFov      = 1,         ///< Described by ovrLayerEyeFov.
    ovrLayerType_Quad        = 3,         ///< Described by ovrLayerQuad. Previously called ovrLayerType_QuadInWorld.
    /// enum 4 used to be ovrLayerType_QuadHeadLocked. Instead, use ovrLayerType_Quad with ovrLayerFlag_HeadLocked.
    ovrLayerType_EyeMatrix   = 5,         ///< Described by ovrLayerEyeMatrix.
    ovrLayerType_EnumSize    = 0x7fffffff ///< Force type int32_t.
} ovrLayerType;


/// Identifies flags used by ovrLayerHeader and which are passed to ovr_SubmitFrame.
///
/// \see ovrLayerHeader
///
typedef enum ovrLayerFlags_
{
    /// ovrLayerFlag_HighQuality enables 4x anisotropic sampling during the composition of the layer.
    /// The benefits are mostly visible at the periphery for high-frequency & high-contrast visuals.
    /// For best results consider combining this flag with an ovrTextureSwapChain that has mipmaps and
    /// instead of using arbitrary sized textures, prefer texture sizes that are powers-of-two.
    /// Actual rendered viewport and doesn't necessarily have to fill the whole texture.
    ovrLayerFlag_HighQuality               = 0x01,

    /// ovrLayerFlag_TextureOriginAtBottomLeft: the opposite is TopLeft.
    /// Generally this is false for D3D, true for OpenGL.
    ovrLayerFlag_TextureOriginAtBottomLeft = 0x02,

    /// Mark this surface as "headlocked", which means it is specified
    /// relative to the HMD and moves with it, rather than being specified
    /// relative to sensor/torso space and remaining still while the head moves.
    /// What used to be ovrLayerType_QuadHeadLocked is now ovrLayerType_Quad plus this flag.
    /// However the flag can be applied to any layer type to achieve a similar effect.
    ovrLayerFlag_HeadLocked                = 0x04

} ovrLayerFlags;


/// Defines properties shared by all ovrLayer structs, such as ovrLayerEyeFov.
///
/// ovrLayerHeader is used as a base member in these larger structs.
/// This struct cannot be used by itself except for the case that Type is ovrLayerType_Disabled.
///
/// \see ovrLayerType, ovrLayerFlags
///
typedef struct OVR_ALIGNAS(OVR_PTR_SIZE) ovrLayerHeader_
{
    ovrLayerType    Type;   ///< Described by ovrLayerType.
    unsigned        Flags;  ///< Described by ovrLayerFlags.
} ovrLayerHeader;


/// Describes a layer that specifies a monoscopic or stereoscopic view.
/// This is the kind of layer that's typically used as layer 0 to ovr_SubmitFrame,
/// as it is the kind of layer used to render a 3D stereoscopic view.
///
/// Three options exist with respect to mono/stereo texture usage:
///    - ColorTexture[0] and ColorTexture[1] contain the left and right stereo renderings, respectively.
///      Viewport[0] and Viewport[1] refer to ColorTexture[0] and ColorTexture[1], respectively.
///    - ColorTexture[0] contains both the left and right renderings, ColorTexture[1] is NULL,
///      and Viewport[0] and Viewport[1] refer to sub-rects with ColorTexture[0].
///    - ColorTexture[0] contains a single monoscopic rendering, and Viewport[0] and
///      Viewport[1] both refer to that rendering.
///
/// \see ovrTextureSwapChain, ovr_SubmitFrame
///
typedef struct OVR_ALIGNAS(OVR_PTR_SIZE) ovrLayerEyeFov_
{
    /// Header.Type must be ovrLayerType_EyeFov.
    ovrLayerHeader      Header;

    /// ovrTextureSwapChains for the left and right eye respectively.
    /// The second one of which can be NULL for cases described above.
    ovrTextureSwapChain  ColorTexture[ovrEye_Count];

    /// Specifies the ColorTexture sub-rect UV coordinates.
    /// Both Viewport[0] and Viewport[1] must be valid.
    ovrRecti            Viewport[ovrEye_Count];

    /// The viewport field of view.
    ovrFovPort          Fov[ovrEye_Count];

    /// Specifies the position and orientation of each eye view, with the position specified in meters.
    /// RenderPose will typically be the value returned from ovr_CalcEyePoses,
    /// but can be different in special cases if a different head pose is used for rendering.
    ovrPosef            RenderPose[ovrEye_Count];

    /// Specifies the timestamp when the source ovrPosef (used in calculating RenderPose)
    /// was sampled from the SDK. Typically retrieved by calling ovr_GetTimeInSeconds
    /// around the instant the application calls ovr_GetTrackingState
    /// The main purpose for this is to accurately track app tracking latency.
    double              SensorSampleTime;

} ovrLayerEyeFov;




/// Describes a layer that specifies a monoscopic or stereoscopic view.
/// This uses a direct 3x4 matrix to map from view space to the UV coordinates.
/// It is essentially the same thing as ovrLayerEyeFov but using a much
/// lower level. This is mainly to provide compatibility with specific apps.
/// Unless the application really requires this flexibility, it is usually better
/// to use ovrLayerEyeFov.
///
/// Three options exist with respect to mono/stereo texture usage:
///    - ColorTexture[0] and ColorTexture[1] contain the left and right stereo renderings, respectively.
///      Viewport[0] and Viewport[1] refer to ColorTexture[0] and ColorTexture[1], respectively.
///    - ColorTexture[0] contains both the left and right renderings, ColorTexture[1] is NULL,
///      and Viewport[0] and Viewport[1] refer to sub-rects with ColorTexture[0].
///    - ColorTexture[0] contains a single monoscopic rendering, and Viewport[0] and
///      Viewport[1] both refer to that rendering.
///
/// \see ovrTextureSwapChain, ovr_SubmitFrame
///
typedef struct OVR_ALIGNAS(OVR_PTR_SIZE) ovrLayerEyeMatrix_
{
    /// Header.Type must be ovrLayerType_EyeMatrix.
    ovrLayerHeader      Header;

    /// ovrTextureSwapChains for the left and right eye respectively.
    /// The second one of which can be NULL for cases described above.
    ovrTextureSwapChain  ColorTexture[ovrEye_Count];

    /// Specifies the ColorTexture sub-rect UV coordinates.
    /// Both Viewport[0] and Viewport[1] must be valid.
    ovrRecti            Viewport[ovrEye_Count];

    /// Specifies the position and orientation of each eye view, with the position specified in meters.
    /// RenderPose will typically be the value returned from ovr_CalcEyePoses,
    /// but can be different in special cases if a different head pose is used for rendering.
    ovrPosef            RenderPose[ovrEye_Count];

    /// Specifies the mapping from a view-space vector
    /// to a UV coordinate on the textures given above.
    /// P = (x,y,z,1)*Matrix
    /// TexU  = P.x/P.z
    /// TexV  = P.y/P.z
    ovrMatrix4f         Matrix[ovrEye_Count];

    /// Specifies the timestamp when the source ovrPosef (used in calculating RenderPose)
    /// was sampled from the SDK. Typically retrieved by calling ovr_GetTimeInSeconds
    /// around the instant the application calls ovr_GetTrackingState
    /// The main purpose for this is to accurately track app tracking latency.
    double              SensorSampleTime;

} ovrLayerEyeMatrix;





/// Describes a layer of Quad type, which is a single quad in world or viewer space.
/// It is used for ovrLayerType_Quad. This type of layer represents a single
/// object placed in the world and not a stereo view of the world itself.
///
/// A typical use of ovrLayerType_Quad is to draw a television screen in a room
/// that for some reason is more convenient to draw as a layer than as part of the main
/// view in layer 0. For example, it could implement a 3D popup GUI that is drawn at a
/// higher resolution than layer 0 to improve fidelity of the GUI.
///
/// Quad layers are visible from both sides; they are not back-face culled.
///
/// \see ovrTextureSwapChain, ovr_SubmitFrame
///
typedef struct OVR_ALIGNAS(OVR_PTR_SIZE) ovrLayerQuad_
{
    /// Header.Type must be ovrLayerType_Quad.
    ovrLayerHeader      Header;

    /// Contains a single image, never with any stereo view.
    ovrTextureSwapChain  ColorTexture;

    /// Specifies the ColorTexture sub-rect UV coordinates.
    ovrRecti            Viewport;

    /// Specifies the orientation and position of the center point of a Quad layer type.
    /// The supplied direction is the vector perpendicular to the quad.
    /// The position is in real-world meters (not the application's virtual world,
    /// the physical world the user is in) and is relative to the "zero" position
    /// set by ovr_RecenterTrackingOrigin unless the ovrLayerFlag_HeadLocked flag is used.
    ovrPosef            QuadPoseCenter;

    /// Width and height (respectively) of the quad in meters.
    ovrVector2f         QuadSize;

} ovrLayerQuad;




/// Union that combines ovrLayer types in a way that allows them
/// to be used in a polymorphic way.
typedef union ovrLayer_Union_
{
    ovrLayerHeader      Header;
    ovrLayerEyeFov      EyeFov;
    ovrLayerQuad        Quad;
} ovrLayer_Union;


//@}



/// @name SDK Distortion Rendering
///
/// All of rendering functions including the configure and frame functions
/// are not thread safe. It is OK to use ConfigureRendering on one thread and handle
/// frames on another thread, but explicit synchronization must be done since
/// functions that depend on configured state are not reentrant.
///
/// These functions support rendering of distortion by the SDK.
///
//@{

/// TextureSwapChain creation is rendering API-specific.
/// ovr_CreateTextureSwapChainDX and ovr_CreateTextureSwapChainGL can be found in the
/// rendering API-specific headers, such as OVR_CAPI_D3D.h and OVR_CAPI_GL.h

/// Gets the number of buffers in an ovrTextureSwapChain.
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  chain Specifies the ovrTextureSwapChain for which the length should be retrieved.
/// \param[out] out_Length Returns the number of buffers in the specified chain.
///
/// \return Returns an ovrResult for which OVR_SUCCESS(result) is false upon error. 
///
/// \see ovr_CreateTextureSwapChainDX, ovr_CreateTextureSwapChainGL
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetTextureSwapChainLength(ovrSession session, ovrTextureSwapChain chain, int* out_Length);

/// Gets the current index in an ovrTextureSwapChain.
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  chain Specifies the ovrTextureSwapChain for which the index should be retrieved.
/// \param[out] out_Index Returns the current (free) index in specified chain.
///
/// \return Returns an ovrResult for which OVR_SUCCESS(result) is false upon error. 
///
/// \see ovr_CreateTextureSwapChainDX, ovr_CreateTextureSwapChainGL
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetTextureSwapChainCurrentIndex(ovrSession session, ovrTextureSwapChain chain, int* out_Index);

/// Gets the description of the buffers in an ovrTextureSwapChain
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  chain Specifies the ovrTextureSwapChain for which the description should be retrieved.
/// \param[out] out_Desc Returns the description of the specified chain.
///
/// \return Returns an ovrResult for which OVR_SUCCESS(result) is false upon error. 
///
/// \see ovr_CreateTextureSwapChainDX, ovr_CreateTextureSwapChainGL
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetTextureSwapChainDesc(ovrSession session, ovrTextureSwapChain chain, ovrTextureSwapChainDesc* out_Desc);

/// Commits any pending changes to an ovrTextureSwapChain, and advances its current index
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  chain Specifies the ovrTextureSwapChain to commit.
///
/// \note When Commit is called, the texture at the current index is considered ready for use by the
/// runtime, and further writes to it should be avoided. The swap chain's current index is advanced,
/// providing there's room in the chain. The next time the SDK dereferences this texture swap chain,
/// it will synchronize with the app's graphics context and pick up the submitted index, opening up
/// room in the swap chain for further commits.
///
/// \return Returns an ovrResult for which OVR_SUCCESS(result) is false upon error. 
///         Failures include but aren't limited to:
///     - ovrError_TextureSwapChainFull: ovr_CommitTextureSwapChain was called too many times on a texture swapchain without calling submit to use the chain.
///
/// \see ovr_CreateTextureSwapChainDX, ovr_CreateTextureSwapChainGL
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_CommitTextureSwapChain(ovrSession session, ovrTextureSwapChain chain);

/// Destroys an ovrTextureSwapChain and frees all the resources associated with it.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] chain Specifies the ovrTextureSwapChain to destroy. If it is NULL then this function has no effect.
///
/// \see ovr_CreateTextureSwapChainDX, ovr_CreateTextureSwapChainGL
///
OVR_PUBLIC_FUNCTION(void) ovr_DestroyTextureSwapChain(ovrSession session, ovrTextureSwapChain chain);


/// MirrorTexture creation is rendering API-specific.
/// ovr_CreateMirrorTextureDX and ovr_CreateMirrorTextureGL can be found in the
/// rendering API-specific headers, such as OVR_CAPI_D3D.h and OVR_CAPI_GL.h

/// Destroys a mirror texture previously created by one of the mirror texture creation functions.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] mirrorTexture Specifies the ovrTexture to destroy. If it is NULL then this function has no effect.
///
/// \see ovr_CreateMirrorTextureDX, ovr_CreateMirrorTextureGL
///
OVR_PUBLIC_FUNCTION(void) ovr_DestroyMirrorTexture(ovrSession session, ovrMirrorTexture mirrorTexture);


/// Calculates the recommended viewport size for rendering a given eye within the HMD
/// with a given FOV cone.
///
/// Higher FOV will generally require larger textures to maintain quality.
/// Apps packing multiple eye views together on the same texture should ensure there are
/// at least 8 pixels of padding between them to prevent texture filtering and chromatic
/// aberration causing images to leak between the two eye views.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] eye Specifies which eye (left or right) to calculate for.
/// \param[in] fov Specifies the ovrFovPort to use.
/// \param[in] pixelsPerDisplayPixel Specifies the ratio of the number of render target pixels
///            to display pixels at the center of distortion. 1.0 is the default value. Lower
///            values can improve performance, higher values give improved quality.
///
/// <b>Example code</b>
///     \code{.cpp}
///         ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);
///         ovrSizei eyeSizeLeft  = ovr_GetFovTextureSize(session, ovrEye_Left,  hmdDesc.DefaultEyeFov[ovrEye_Left],  1.0f);
///         ovrSizei eyeSizeRight = ovr_GetFovTextureSize(session, ovrEye_Right, hmdDesc.DefaultEyeFov[ovrEye_Right], 1.0f);
///     \endcode
///
/// \return Returns the texture width and height size.
///
OVR_PUBLIC_FUNCTION(ovrSizei) ovr_GetFovTextureSize(ovrSession session, ovrEyeType eye, ovrFovPort fov,
                                                       float pixelsPerDisplayPixel);

/// Computes the distortion viewport, view adjust, and other rendering parameters for
/// the specified eye.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] eyeType Specifies which eye (left or right) for which to perform calculations.
/// \param[in] fov Specifies the ovrFovPort to use.
///
/// \return Returns the computed ovrEyeRenderDesc for the given eyeType and field of view.
///
/// \see ovrEyeRenderDesc
///
OVR_PUBLIC_FUNCTION(ovrEyeRenderDesc) ovr_GetRenderDesc(ovrSession session,
                                                           ovrEyeType eyeType, ovrFovPort fov);

/// Submits layers for distortion and display.
///
/// ovr_SubmitFrame triggers distortion and processing which might happen asynchronously.
/// The function will return when there is room in the submission queue and surfaces
/// are available. Distortion might or might not have completed.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
///
/// \param[in] frameIndex Specifies the targeted application frame index, or 0 to refer to one frame
///        after the last time ovr_SubmitFrame was called.
///
/// \param[in] viewScaleDesc Provides additional information needed only if layerPtrList contains
///        an ovrLayerType_Quad. If NULL, a default version is used based on the current configuration and a 1.0 world scale.
///
/// \param[in] layerPtrList Specifies a list of ovrLayer pointers, which can include NULL entries to
///        indicate that any previously shown layer at that index is to not be displayed.
///        Each layer header must be a part of a layer structure such as ovrLayerEyeFov or ovrLayerQuad,
///        with Header.Type identifying its type. A NULL layerPtrList entry in the array indicates the
//         absence of the given layer.
///
/// \param[in] layerCount Indicates the number of valid elements in layerPtrList. The maximum
///        supported layerCount is not currently specified, but may be specified in a future version.
///
/// - Layers are drawn in the order they are specified in the array, regardless of the layer type.
///
/// - Layers are not remembered between successive calls to ovr_SubmitFrame. A layer must be
///   specified in every call to ovr_SubmitFrame or it won't be displayed.
///
/// - If a layerPtrList entry that was specified in a previous call to ovr_SubmitFrame is
///   passed as NULL or is of type ovrLayerType_Disabled, that layer is no longer displayed.
///
/// - A layerPtrList entry can be of any layer type and multiple entries of the same layer type
///   are allowed. No layerPtrList entry may be duplicated (i.e. the same pointer as an earlier entry).
///
/// <b>Example code</b>
///     \code{.cpp}
///         ovrLayerEyeFov  layer0;
///         ovrLayerQuad    layer1;
///           ...
///         ovrLayerHeader* layers[2] = { &layer0.Header, &layer1.Header };
///         ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, layers, 2);
///     \endcode
///
/// \return Returns an ovrResult for which OVR_SUCCESS(result) is false upon error and true
///         upon success. Return values include but aren't limited to:
///     - ovrSuccess: rendering completed successfully.
///     - ovrSuccess_NotVisible: rendering completed successfully but was not displayed on the HMD,
///       usually because another application currently has ownership of the HMD. Applications receiving
///       this result should stop rendering new content, but continue to call ovr_SubmitFrame periodically
///       until it returns a value other than ovrSuccess_NotVisible.
///     - ovrError_DisplayLost: The session has become invalid (such as due to a device removal)
///       and the shared resources need to be released (ovr_DestroyTextureSwapChain), the session needs to
///       destroyed (ovr_Destroy) and recreated (ovr_Create), and new resources need to be created
///       (ovr_CreateTextureSwapChainXXX). The application's existing private graphics resources do not
///       need to be recreated unless the new ovr_Create call returns a different GraphicsLuid.
///     - ovrError_TextureSwapChainInvalid: The ovrTextureSwapChain is in an incomplete or inconsistent state. 
///       Ensure ovr_CommitTextureSwapChain was called at least once first.
///
/// \see ovr_GetPredictedDisplayTime, ovrViewScaleDesc, ovrLayerHeader
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_SubmitFrame(ovrSession session, long long frameIndex,
                                                  const ovrViewScaleDesc* viewScaleDesc,
                                                  ovrLayerHeader const * const * layerPtrList, unsigned int layerCount);
///@}



//-------------------------------------------------------------------------------------
/// @name Frame Timing
///
//@{


/// Gets the time of the specified frame midpoint.
///
/// Predicts the time at which the given frame will be displayed. The predicted time 
/// is the middle of the time period during which the corresponding eye images will 
/// be displayed. 
///
/// The application should increment frameIndex for each successively targeted frame,
/// and pass that index to any relevent OVR functions that need to apply to the frame
/// identified by that index.
///
/// This function is thread-safe and allows for multiple application threads to target
/// their processing to the same displayed frame.
/// 
/// In the even that prediction fails due to various reasons (e.g. the display being off
/// or app has yet to present any frames), the return value will be current CPU time.
/// 
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] frameIndex Identifies the frame the caller wishes to target.
///            A value of zero returns the next frame index.
/// \return Returns the absolute frame midpoint time for the given frameIndex. 
/// \see ovr_GetTimeInSeconds
///
OVR_PUBLIC_FUNCTION(double) ovr_GetPredictedDisplayTime(ovrSession session, long long frameIndex);


/// Returns global, absolute high-resolution time in seconds.
///
/// The time frame of reference for this function is not specified and should not be
/// depended upon.
///
/// \return Returns seconds as a floating point value.
/// \see ovrPoseStatef, ovrFrameTiming
///
OVR_PUBLIC_FUNCTION(double) ovr_GetTimeInSeconds();


/// Performance HUD enables the HMD user to see information critical to
/// the real-time operation of the VR application such as latency timing,
/// and CPU & GPU performance metrics
///
///     App can toggle performance HUD modes as such:
///     \code{.cpp}
///         ovrPerfHudMode PerfHudMode = ovrPerfHud_LatencyTiming;
///         ovr_SetInt(session, OVR_PERF_HUD_MODE, (int)PerfHudMode);
///     \endcode
///
typedef enum ovrPerfHudMode_
{
    ovrPerfHud_Off                = 0,  ///< Turns off the performance HUD
    ovrPerfHud_PerfSummary        = 1,  ///< Shows performance summary and headroom
    ovrPerfHud_LatencyTiming      = 2,  ///< Shows latency related timing info
    ovrPerfHud_AppRenderTiming    = 3,  ///< Shows render timing info for application
    ovrPerfHud_CompRenderTiming   = 4,  ///< Shows render timing info for OVR compositor
    ovrPerfHud_VersionInfo        = 5,  ///< Shows SDK & HMD version Info
    ovrPerfHud_Count              = 6,  ///< \internal Count of enumerated elements.
    ovrPerfHud_EnumSize = 0x7fffffff    ///< \internal Force type int32_t.
} ovrPerfHudMode;

/// Layer HUD enables the HMD user to see information about a layer
///
///     App can toggle layer HUD modes as such:
///     \code{.cpp}
///         ovrLayerHudMode LayerHudMode = ovrLayerHud_Info;
///         ovr_SetInt(session, OVR_LAYER_HUD_MODE, (int)LayerHudMode);
///     \endcode
///
typedef enum ovrLayerHudMode_
{
    ovrLayerHud_Off = 0, ///< Turns off the layer HUD
    ovrLayerHud_Info = 1, ///< Shows info about a specific layer
    ovrLayerHud_EnumSize = 0x7fffffff
} ovrLayerHudMode;

///@}

/// Debug HUD is provided to help developers gauge and debug the fidelity of their app's
/// stereo rendering characteristics. Using the provided quad and crosshair guides, 
/// the developer can verify various aspects such as VR tracking units (e.g. meters),
/// stereo camera-parallax properties (e.g. making sure objects at infinity are rendered
/// with the proper separation), measuring VR geometry sizes and distances and more.
///
///     App can toggle the debug HUD modes as such:
///     \code{.cpp}
///         ovrDebugHudStereoMode DebugHudMode = ovrDebugHudStereo_QuadWithCrosshair;
///         ovr_SetInt(session, OVR_DEBUG_HUD_STEREO_MODE, (int)DebugHudMode);
///     \endcode
///
/// The app can modify the visual properties of the stereo guide (i.e. quad, crosshair)
/// using the ovr_SetFloatArray function. For a list of tweakable properties,
/// see the OVR_DEBUG_HUD_STEREO_GUIDE_* keys in the OVR_CAPI_Keys.h header file.
typedef enum ovrDebugHudStereoMode_
{
    ovrDebugHudStereo_Off                 = 0,  ///< Turns off the Stereo Debug HUD
    ovrDebugHudStereo_Quad                = 1,  ///< Renders Quad in world for Stereo Debugging
    ovrDebugHudStereo_QuadWithCrosshair   = 2,  ///< Renders Quad+crosshair in world for Stereo Debugging
    ovrDebugHudStereo_CrosshairAtInfinity = 3,  ///< Renders screen-space crosshair at infinity for Stereo Debugging
    ovrDebugHudStereo_Count,                    ///< \internal Count of enumerated elements

    ovrDebugHudStereo_EnumSize = 0x7fffffff     ///< \internal Force type int32_t
} ovrDebugHudStereoMode;




// -----------------------------------------------------------------------------------
/// @name Property Access
///
/// These functions read and write OVR properties. Supported properties
/// are defined in OVR_CAPI_Keys.h
///
//@{

/// Reads a boolean property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid for only the call.
/// \param[in] defaultVal specifes the value to return if the property couldn't be read.
/// \return Returns the property interpreted as a boolean value. Returns defaultVal if
///         the property doesn't exist.
OVR_PUBLIC_FUNCTION(ovrBool) ovr_GetBool(ovrSession session, const char* propertyName, ovrBool defaultVal);

/// Writes or creates a boolean property.
/// If the property wasn't previously a boolean property, it is changed to a boolean property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The value to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetBool(ovrSession session, const char* propertyName, ovrBool value);


/// Reads an integer property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] defaultVal Specifes the value to return if the property couldn't be read.
/// \return Returns the property interpreted as an integer value. Returns defaultVal if
///         the property doesn't exist.
OVR_PUBLIC_FUNCTION(int) ovr_GetInt(ovrSession session, const char* propertyName, int defaultVal);

/// Writes or creates an integer property.
///
/// If the property wasn't previously a boolean property, it is changed to an integer property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The value to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetInt(ovrSession session, const char* propertyName, int value);


/// Reads a float property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] defaultVal specifes the value to return if the property couldn't be read.
/// \return Returns the property interpreted as an float value. Returns defaultVal if
///         the property doesn't exist.
OVR_PUBLIC_FUNCTION(float) ovr_GetFloat(ovrSession session, const char* propertyName, float defaultVal);

/// Writes or creates a float property.
/// If the property wasn't previously a float property, it's changed to a float property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The value to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetFloat(ovrSession session, const char* propertyName, float value);


/// Reads a float array property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] values An array of float to write to.
/// \param[in] valuesCapacity Specifies the maximum number of elements to write to the values array.
/// \return Returns the number of elements read, or 0 if property doesn't exist or is empty.
OVR_PUBLIC_FUNCTION(unsigned int) ovr_GetFloatArray(ovrSession session, const char* propertyName,
                                                       float values[], unsigned int valuesCapacity);

/// Writes or creates a float array property.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] values An array of float to write from.
/// \param[in] valuesSize Specifies the number of elements to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetFloatArray(ovrSession session, const char* propertyName,
                                                  const float values[], unsigned int valuesSize);


/// Reads a string property.
/// Strings are UTF8-encoded and null-terminated.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] defaultVal Specifes the value to return if the property couldn't be read.
/// \return Returns the string property if it exists. Otherwise returns defaultVal, which can be specified as NULL.
///         The return memory is guaranteed to be valid until next call to ovr_GetString or
///         until the session is destroyed, whichever occurs first.
OVR_PUBLIC_FUNCTION(const char*) ovr_GetString(ovrSession session, const char* propertyName,
                                                  const char* defaultVal);

/// Writes or creates a string property.
/// Strings are UTF8-encoded and null-terminated.
///
/// \param[in] session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The string property, which only needs to be valid for the duration of the call.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetString(ovrSession session, const char* propertyName,
                                              const char* value);

///@}



#ifdef __cplusplus
} // extern "C"
#endif


#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

/// @cond DoxygenIgnore
//-----------------------------------------------------------------------------
// ***** Compiler packing validation
//
// These checks ensure that the compiler settings being used will be compatible
// with with pre-built dynamic library provided with the runtime.

OVR_STATIC_ASSERT(sizeof(ovrBool) == 1,         "ovrBool size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrVector2i) == 4 * 2, "ovrVector2i size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrSizei) == 4 * 2,    "ovrSizei size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrRecti) == sizeof(ovrVector2i) + sizeof(ovrSizei), "ovrRecti size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrQuatf) == 4 * 4,    "ovrQuatf size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrVector2f) == 4 * 2, "ovrVector2f size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrVector3f) == 4 * 3, "ovrVector3f size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrMatrix4f) == 4 * 16, "ovrMatrix4f size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrPosef) == (7 * 4),       "ovrPosef size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrPoseStatef) == (22 * 4), "ovrPoseStatef size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrFovPort) == (4 * 4),     "ovrFovPort size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrHmdCaps) == 4,      "ovrHmdCaps size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrTrackingCaps) == 4, "ovrTrackingCaps size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrEyeType) == 4,      "ovrEyeType size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrHmdType) == 4,      "ovrHmdType size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrTrackerDesc) == 4 + 4 + 4 + 4, "ovrTrackerDesc size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrTrackerPose) == 4 + 4 + sizeof(ovrPosef) + sizeof(ovrPosef), "ovrTrackerPose size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrTrackingState) == sizeof(ovrPoseStatef) + 4 + 4 + (sizeof(ovrPoseStatef) * 2) + (sizeof(unsigned int) * 2) + sizeof(ovrPosef) + 4, "ovrTrackingState size mismatch");


//OVR_STATIC_ASSERT(sizeof(ovrTextureHeader) == sizeof(ovrRenderAPIType) + sizeof(ovrSizei),
//                      "ovrTextureHeader size mismatch");
//OVR_STATIC_ASSERT(sizeof(ovrTexture) == sizeof(ovrTextureHeader) OVR_ON64(+4) + sizeof(uintptr_t) * 8,
//                      "ovrTexture size mismatch");
//
OVR_STATIC_ASSERT(sizeof(ovrStatusBits) == 4, "ovrStatusBits size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrSessionStatus) == 6, "ovrSessionStatus size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrEyeRenderDesc) == sizeof(ovrEyeType) + sizeof(ovrFovPort) + sizeof(ovrRecti) +
                                                  sizeof(ovrVector2f) + sizeof(ovrVector3f),
                      "ovrEyeRenderDesc size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrTimewarpProjectionDesc) == 4 * 3, "ovrTimewarpProjectionDesc size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrInitFlags) == 4, "ovrInitFlags size mismatch");
OVR_STATIC_ASSERT(sizeof(ovrLogLevel) == 4, "ovrLogLevel size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrInitParams) == 4 + 4 + sizeof(ovrLogCallback) + sizeof(uintptr_t) + 4 + 4,
                      "ovrInitParams size mismatch");

OVR_STATIC_ASSERT(sizeof(ovrHmdDesc) == 
    + sizeof(ovrHmdType)                // Type
    OVR_ON64(+ 4)                       // pad0
    + 64                                // ProductName 
    + 64                                // Manufacturer
    + 2                                 // VendorId
    + 2                                 // ProductId
    + 24                                // SerialNumber
    + 2                                 // FirmwareMajor
    + 2                                 // FirmwareMinor
    + 4 * 4                             // AvailableHmdCaps - DefaultTrackingCaps
    + sizeof(ovrFovPort) * 2            // DefaultEyeFov
    + sizeof(ovrFovPort) * 2            // MaxEyeFov
    + sizeof(ovrSizei)                  // Resolution
    + 4                                 // DisplayRefreshRate
    OVR_ON64(+ 4)                       // pad1
    , "ovrHmdDesc size mismatch");


// -----------------------------------------------------------------------------------
// ***** Backward compatibility #includes
//
// This is at the bottom of this file because the following is dependent on the
// declarations above.

#if !defined(OVR_CAPI_NO_UTILS)
    #include "Extras/OVR_CAPI_Util.h"
#endif

/// @endcond

#endif // OVR_CAPI_h
