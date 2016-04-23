/********************************************************************************//**
\file      OVR_CAPI.h
\brief     Keys for CAPI proprty function calls
\copyright Copyright 2015 Oculus VR, LLC All Rights reserved.
************************************************************************************/

#ifndef OVR_CAPI_Keys_h
#define OVR_CAPI_Keys_h

#include "OVR_Version.h"



#define OVR_KEY_USER                        "User"                // string

#define OVR_KEY_NAME                        "Name"                // string

#define OVR_KEY_GENDER                      "Gender"              // string "Male", "Female", or "Unknown"
#define OVR_DEFAULT_GENDER                  "Unknown"

#define OVR_KEY_PLAYER_HEIGHT               "PlayerHeight"        // float meters
#define OVR_DEFAULT_PLAYER_HEIGHT           1.778f

#define OVR_KEY_EYE_HEIGHT                  "EyeHeight"           // float meters
#define OVR_DEFAULT_EYE_HEIGHT              1.675f

#define OVR_KEY_NECK_TO_EYE_DISTANCE        "NeckEyeDistance"     // float[2] meters
#define OVR_DEFAULT_NECK_TO_EYE_HORIZONTAL  0.0805f
#define OVR_DEFAULT_NECK_TO_EYE_VERTICAL    0.075f


#define OVR_KEY_EYE_TO_NOSE_DISTANCE        "EyeToNoseDist"       // float[2] meters





#define OVR_PERF_HUD_MODE                       "PerfHudMode"                       // int, allowed values are defined in enum ovrPerfHudMode

#define OVR_LAYER_HUD_MODE                      "LayerHudMode"                      // int, allowed values are defined in enum ovrLayerHudMode
#define OVR_LAYER_HUD_CURRENT_LAYER             "LayerHudCurrentLayer"              // int, The layer to show 
#define OVR_LAYER_HUD_SHOW_ALL_LAYERS           "LayerHudShowAll"                   // bool, Hide other layers when the hud is enabled

#define OVR_DEBUG_HUD_STEREO_MODE               "DebugHudStereoMode"                // int, allowed values are defined in enum ovrDebugHudStereoMode
#define OVR_DEBUG_HUD_STEREO_GUIDE_INFO_ENABLE  "DebugHudStereoGuideInfoEnable"     // bool
#define OVR_DEBUG_HUD_STEREO_GUIDE_SIZE         "DebugHudStereoGuideSize2f"         // float[2]
#define OVR_DEBUG_HUD_STEREO_GUIDE_POSITION     "DebugHudStereoGuidePosition3f"     // float[3]
#define OVR_DEBUG_HUD_STEREO_GUIDE_YAWPITCHROLL "DebugHudStereoGuideYawPitchRoll3f" // float[3]
#define OVR_DEBUG_HUD_STEREO_GUIDE_COLOR        "DebugHudStereoGuideColor4f"        // float[4]



#endif // OVR_CAPI_Keys_h
