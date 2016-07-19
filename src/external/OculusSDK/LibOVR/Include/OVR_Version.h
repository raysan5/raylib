/********************************************************************************//**
\file      OVR_Version.h
\brief     This header provides LibOVR version identification.
\copyright Copyright 2014-2016 Oculus VR, LLC All Rights reserved.
*************************************************************************************/

#ifndef OVR_Version_h
#define OVR_Version_h



/// Conventional string-ification macro.
#if !defined(OVR_STRINGIZE)
    #define OVR_STRINGIZEIMPL(x) #x
    #define OVR_STRINGIZE(x)     OVR_STRINGIZEIMPL(x)
#endif


// Master version numbers
#define OVR_PRODUCT_VERSION 1  // Product version doesn't participate in semantic versioning.
#define OVR_MAJOR_VERSION   1  // If you change these values then you need to also make sure to change LibOVR/Projects/Windows/LibOVR.props in parallel.
#define OVR_MINOR_VERSION   6  // 
#define OVR_PATCH_VERSION   0
#define OVR_BUILD_NUMBER    0

// This is the ((product * 100) + major) version of the service that the DLL is compatible with.
// When we backport changes to old versions of the DLL we update the old DLLs
// to move this version number up to the latest version.
// The DLL is responsible for checking that the service is the version it supports
// and returning an appropriate error message if it has not been made compatible.
#define OVR_DLL_COMPATIBLE_VERSION 101

#define OVR_FEATURE_VERSION 0


/// "Major.Minor.Patch"
#if !defined(OVR_VERSION_STRING)
    #define OVR_VERSION_STRING  OVR_STRINGIZE(OVR_MAJOR_VERSION.OVR_MINOR_VERSION.OVR_PATCH_VERSION)
#endif


/// "Major.Minor.Patch.Build"
#if !defined(OVR_DETAILED_VERSION_STRING)
    #define OVR_DETAILED_VERSION_STRING OVR_STRINGIZE(OVR_MAJOR_VERSION.OVR_MINOR_VERSION.OVR_PATCH_VERSION.OVR_BUILD_NUMBER)
#endif


/// \brief file description for version info
/// This appears in the user-visible file properties. It is intended to convey publicly
/// available additional information such as feature builds.
#if !defined(OVR_FILE_DESCRIPTION_STRING)
    #if defined(_DEBUG)
        #define OVR_FILE_DESCRIPTION_STRING "dev build debug"
    #else
        #define OVR_FILE_DESCRIPTION_STRING "dev build"
    #endif
#endif


#endif // OVR_Version_h
