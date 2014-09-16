/**********************************************************************************************************************************
*
*   OpenAL cross platform audio library
*   Copyright (c) 2004-2006, Apple Computer, Inc. All rights reserved.
*   Copyright (c) 2007-2008, Apple Inc. All rights reserved.
*
*   Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following 
*   conditions are met:
*
*   1.  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
*   2.  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
*       disclaimer in the documentation and/or other materials provided with the distribution. 
*   3.  Neither the name of Apple Inc. ("Apple") nor the names of its contributors may be used to endorse or promote products derived 
*       from this software without specific prior written permission. 
*
*   THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
*   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS 
*   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
*   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
*   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**********************************************************************************************************************************/

#ifndef __OAL_MAC_OSX_OAL_EXTENSIONS_H__
#define __OAL_MAC_OSX_OAL_EXTENSIONS_H__

#include <OpenAL/al.h>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ALC_EXT_MAC_OSX
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Retrieve functions via alGetProcAddress() by passing in strings: alcMacOSXMixerOutputRate or alcMacOSXGetMixerOutputRate

// Setting the Mixer Output Rate effectively sets the samnple rate at which the mixer
typedef ALvoid (*alcMacOSXRenderingQualityProcPtr) (ALint value);
typedef ALvoid (*alMacOSXRenderChannelCountProcPtr) (ALint value);
typedef ALvoid (*alcMacOSXMixerMaxiumumBussesProcPtr) (ALint value);
typedef ALvoid (*alcMacOSXMixerOutputRateProcPtr) (ALdouble value);

typedef ALint (*alcMacOSXGetRenderingQualityProcPtr) ();
typedef ALint (*alMacOSXGetRenderChannelCountProcPtr) ();
typedef ALint (*alcMacOSXGetMixerMaxiumumBussesProcPtr) ();
typedef ALdouble (*alcMacOSXGetMixerOutputRateProcPtr) ();

/* Render Quality. Used with alcMacOSXRenderingQuality() */
    
    #define ALC_MAC_OSX_SPATIAL_RENDERING_QUALITY_HIGH          'rqhi'
    #define ALC_MAC_OSX_SPATIAL_RENDERING_QUALITY_LOW           'rdlo'
    
    // High Quality Spatial Algorithm suitable only for headphone use
    #define ALC_IPHONE_SPATIAL_RENDERING_QUALITY_HEADPHONES     'hdph'      

/*  
    Render Channels. Used with alMacOSXRenderChannelCount()
    Allows a user to force OpenAL to render to stereo, regardless of the audio hardware being used
*/
    #define ALC_MAC_OSX_RENDER_CHANNEL_COUNT_STEREO         'rcst'

/* GameKit extension */

    #define AL_GAMEKIT                                          'gksr'

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 AL_EXT_SOURCE_NOTIFICATIONS
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*
 Source Notifications
 
 Eliminates the need for continuous polling for source state by providing a
 mechanism for the application to receive source state change notifications.
 Upon receiving a notification, the application can retrieve the actual state
 corresponding to the notification ID for which the notification was sent.
 */

#define AL_QUEUE_HAS_LOOPED                                 0x9000

/*
 Notification Proc: ALSourceNotificationProc
 
 sid        - source id
 notificationID - id of state that has changed
 userData   - user data provided to alSourceAddNotification()
 */

typedef ALvoid (*alSourceNotificationProc)(ALuint sid, ALuint   notificationID, ALvoid* userData);

/*
 API: alSourceAddNotification
 
 sid        - source id
 notificationID - id of state for which caller wants to be notified of a change                     
 notifyProc - notification proc
 userData   - ptr to applications user data, will be returned in the notification proc
 
 Returns AL_NO_ERROR if request is successful.
 
 Valid IDs:
 AL_SOURCE_STATE
 AL_BUFFERS_PROCESSED
 AL_QUEUE_HAS_LOOPED    - notification sent when a looping source has looped to it's start point
 */
typedef ALenum (*alSourceAddNotificationProcPtr) (ALuint sid, ALuint notificationID, alSourceNotificationProc notifyProc, ALvoid* userData);

/*
 API: alSourceRemoveStateNotification
 
 sid        - source id
 notificationID - id of state for which caller wants to remove an existing notification                     
 notifyProc - notification proc
 userData   - ptr to applications user data, will be returned in the notification proc
 */
typedef ALvoid (*alSourceRemoveNotificationProcPtr) (ALuint sid, ALuint notificationID, alSourceNotificationProc notifyProc, ALvoid* userData);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ALC_EXT_ASA : Apple Spatial Audio Extension
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*
    Used with the ASA API calls: alcASAGetSource(), alcASASetSource(), alcASAGetListener(), alcASASetListener()
*/

typedef ALenum  (*alcASAGetSourceProcPtr) (ALuint property, ALuint source, ALvoid *data, ALuint* dataSize);
typedef ALenum  (*alcASASetSourceProcPtr) (ALuint property, ALuint source, ALvoid *data, ALuint dataSize);
typedef ALenum  (*alcASAGetListenerProcPtr) (ALuint property, ALvoid *data, ALuint* dataSize);
typedef ALenum  (*alcASASetListenerProcPtr) (ALuint property, ALvoid *data, ALuint dataSize);

    /* listener properties */
    #define ALC_ASA_REVERB_ON                           'rvon'  // type ALuint
    #define ALC_ASA_REVERB_GLOBAL_LEVEL                 'rvgl'  // type ALfloat -40.0 db - 40.0 db

    #define ALC_ASA_REVERB_ROOM_TYPE                    'rvrt'  // type ALint   
    
    /* reverb room type presets for the ALC_ASA_REVERB_ROOM_TYPE property */
    #define ALC_ASA_REVERB_ROOM_TYPE_SmallRoom          0
    #define ALC_ASA_REVERB_ROOM_TYPE_MediumRoom         1
    #define ALC_ASA_REVERB_ROOM_TYPE_LargeRoom          2
    #define ALC_ASA_REVERB_ROOM_TYPE_MediumHall         3
    #define ALC_ASA_REVERB_ROOM_TYPE_LargeHall          4
    #define ALC_ASA_REVERB_ROOM_TYPE_Plate              5
    #define ALC_ASA_REVERB_ROOM_TYPE_MediumChamber      6
    #define ALC_ASA_REVERB_ROOM_TYPE_LargeChamber       7
    #define ALC_ASA_REVERB_ROOM_TYPE_Cathedral          8
    #define ALC_ASA_REVERB_ROOM_TYPE_LargeRoom2         9
    #define ALC_ASA_REVERB_ROOM_TYPE_MediumHall2        10
    #define ALC_ASA_REVERB_ROOM_TYPE_MediumHall3        11
    #define ALC_ASA_REVERB_ROOM_TYPE_LargeHall2         12

    #define ALC_ASA_REVERB_EQ_GAIN                      'rveg'  // type ALfloat
    #define ALC_ASA_REVERB_EQ_BANDWITH                  'rveb'  // type ALfloat
    #define ALC_ASA_REVERB_EQ_FREQ                      'rvef'  // type ALfloat

    /* source properties */
    #define ALC_ASA_REVERB_SEND_LEVEL                   'rvsl'  // type ALfloat 0.0 (dry) - 1.0 (wet) (0-100% dry/wet mix, 0.0 default)
    #define ALC_ASA_OCCLUSION                           'occl'  // type ALfloat -100.0 db (most occlusion) - 0.0 db (no occlusion, 0.0 default)
    #define ALC_ASA_OBSTRUCTION                         'obst'  // type ALfloat -100.0 db (most obstruction) - 0.0 db (no obstruction, 0.0 default) 
                            
#endif // __OAL_MAC_OSX_OAL_EXTENSIONS_H__
