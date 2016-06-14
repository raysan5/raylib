/************************************************************************************

Filename    :   OVR_StereoProjection.h
Content     :   Stereo projection functions
Created     :   November 30, 2013
Authors     :   Tom Fosyth

Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License"); 
you may not use the Oculus VR Rift SDK except in compliance with the License, 
which is provided at the time of installation or download, or which 
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3 

Unless required by applicable law or agreed to in writing, the Oculus VR SDK 
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*************************************************************************************/

#ifndef OVR_StereoProjection_h
#define OVR_StereoProjection_h


#include "Extras/OVR_Math.h"


namespace OVR {


//-----------------------------------------------------------------------------------
// ***** Stereo Enumerations

// StereoEye specifies which eye we are rendering for; it is used to
// retrieve StereoEyeParams.
enum StereoEye
{
    StereoEye_Left,
    StereoEye_Right,
    StereoEye_Center
};



//-----------------------------------------------------------------------------------
// ***** Propjection functions

Matrix4f            CreateProjection ( bool rightHanded, bool isOpenGL, FovPort fov, StereoEye eye,
                                       float zNear = 0.01f, float zFar = 10000.0f,
                                       bool flipZ = false, bool farAtInfinity = false);

Matrix4f            CreateOrthoSubProjection ( bool rightHanded, StereoEye eyeType,
                                               float tanHalfFovX, float tanHalfFovY,
                                               float unitsX, float unitsY, float distanceFromCamera,
                                               float interpupillaryDistance, Matrix4f const &projection,
                                               float zNear = 0.0f, float zFar = 0.0f,
                                               bool flipZ = false, bool farAtInfinity = false);

ScaleAndOffset2D    CreateNDCScaleAndOffsetFromFov ( FovPort fov );


} //namespace OVR

#endif // OVR_StereoProjection_h
