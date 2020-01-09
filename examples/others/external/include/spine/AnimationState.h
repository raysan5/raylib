/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated January 1, 2020. Replaces all prior versions.
 *
 * Copyright (c) 2013-2020, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software
 * or otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THE SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef SPINE_ANIMATIONSTATE_H_
#define SPINE_ANIMATIONSTATE_H_

#include <spine/dll.h>
#include <spine/Animation.h>
#include <spine/AnimationStateData.h>
#include <spine/Event.h>
#include <spine/Array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_ANIMATION_START, SP_ANIMATION_INTERRUPT, SP_ANIMATION_END, SP_ANIMATION_COMPLETE, SP_ANIMATION_DISPOSE, SP_ANIMATION_EVENT
} spEventType;

typedef struct spAnimationState spAnimationState;
typedef struct spTrackEntry spTrackEntry;

typedef void (*spAnimationStateListener) (spAnimationState* state, spEventType type, spTrackEntry* entry, spEvent* event);

_SP_ARRAY_DECLARE_TYPE(spTrackEntryArray, spTrackEntry*)

struct spTrackEntry {
	spAnimation* animation;
	spTrackEntry* next;
	spTrackEntry* mixingFrom;
	spTrackEntry* mixingTo;
	spAnimationStateListener listener;
	int trackIndex;
	int /*boolean*/ loop;
	int /*boolean*/ holdPrevious;
	float eventThreshold, attachmentThreshold, drawOrderThreshold;
	float animationStart, animationEnd, animationLast, nextAnimationLast;
	float delay, trackTime, trackLast, nextTrackLast, trackEnd, timeScale;
	float alpha, mixTime, mixDuration, interruptAlpha, totalAlpha;
	spMixBlend mixBlend;
	spIntArray* timelineMode;
	spTrackEntryArray* timelineHoldMix;
	float* timelinesRotation;
	int timelinesRotationCount;
	void* rendererObject;
	void* userData;

#ifdef __cplusplus
	spTrackEntry() :
		animation(0),
		next(0), mixingFrom(0), mixingTo(0),
		listener(0),
		trackIndex(0),
		loop(0),
		holdPrevious(0),
		eventThreshold(0), attachmentThreshold(0), drawOrderThreshold(0),
		animationStart(0), animationEnd(0), animationLast(0), nextAnimationLast(0),
		delay(0), trackTime(0), trackLast(0), nextTrackLast(0), trackEnd(0), timeScale(0),
		alpha(0), mixTime(0), mixDuration(0), interruptAlpha(0), totalAlpha(0),
		mixBlend(SP_MIX_BLEND_REPLACE),
		timelineMode(0),
		timelineHoldMix(0),
		timelinesRotation(0),
		timelinesRotationCount(0),
		rendererObject(0), userData(0) {
	}
#endif
};

struct spAnimationState {
	spAnimationStateData* const data;

	int tracksCount;
	spTrackEntry** tracks;

	spAnimationStateListener listener;

	float timeScale;

	void* rendererObject;
	void* userData;

#ifdef __cplusplus
	spAnimationState() :
		data(0),
		tracksCount(0),
		tracks(0),
		listener(0),
		timeScale(0),
		rendererObject(0),
		userData(0) {
	}
#endif
};

/* @param data May be 0 for no mixing. */
SP_API spAnimationState* spAnimationState_create (spAnimationStateData* data);
SP_API void spAnimationState_dispose (spAnimationState* self);

SP_API void spAnimationState_update (spAnimationState* self, float delta);
SP_API int /**bool**/ spAnimationState_apply (spAnimationState* self, struct spSkeleton* skeleton);

SP_API void spAnimationState_clearTracks (spAnimationState* self);
SP_API void spAnimationState_clearTrack (spAnimationState* self, int trackIndex);

/** Set the current animation. Any queued animations are cleared. */
SP_API spTrackEntry* spAnimationState_setAnimationByName (spAnimationState* self, int trackIndex, const char* animationName,
		int/*bool*/loop);
SP_API spTrackEntry* spAnimationState_setAnimation (spAnimationState* self, int trackIndex, spAnimation* animation, int/*bool*/loop);

/** Adds an animation to be played delay seconds after the current or last queued animation, taking into account any mix
 * duration. */
SP_API spTrackEntry* spAnimationState_addAnimationByName (spAnimationState* self, int trackIndex, const char* animationName,
		int/*bool*/loop, float delay);
SP_API spTrackEntry* spAnimationState_addAnimation (spAnimationState* self, int trackIndex, spAnimation* animation, int/*bool*/loop,
		float delay);
SP_API spTrackEntry* spAnimationState_setEmptyAnimation(spAnimationState* self, int trackIndex, float mixDuration);
SP_API spTrackEntry* spAnimationState_addEmptyAnimation(spAnimationState* self, int trackIndex, float mixDuration, float delay);
SP_API void spAnimationState_setEmptyAnimations(spAnimationState* self, float mixDuration);

SP_API spTrackEntry* spAnimationState_getCurrent (spAnimationState* self, int trackIndex);

SP_API void spAnimationState_clearListenerNotifications(spAnimationState* self);

SP_API float spTrackEntry_getAnimationTime (spTrackEntry* entry);

/** Use this to dispose static memory before your app exits to appease your memory leak detector*/
SP_API void spAnimationState_disposeStatics ();

#ifdef SPINE_SHORT_NAMES
typedef spEventType EventType;
#define ANIMATION_START SP_ANIMATION_START
#define ANIMATION_INTERRUPT SP_ANIMATION_INTERRUPT
#define ANIMATION_END SP_ANIMATION_END
#define ANIMATION_COMPLETE SP_ANIMATION_COMPLETE
#define ANIMATION_DISPOSE SP_ANIMATION_DISPOSE
#define ANIMATION_EVENT SP_ANIMATION_EVENT
typedef spAnimationStateListener AnimationStateListener;
typedef spTrackEntry TrackEntry;
typedef spAnimationState AnimationState;
#define AnimationState_create(...) spAnimationState_create(__VA_ARGS__)
#define AnimationState_dispose(...) spAnimationState_dispose(__VA_ARGS__)
#define AnimationState_update(...) spAnimationState_update(__VA_ARGS__)
#define AnimationState_apply(...) spAnimationState_apply(__VA_ARGS__)
#define AnimationState_clearTracks(...) spAnimationState_clearTracks(__VA_ARGS__)
#define AnimationState_clearTrack(...) spAnimationState_clearTrack(__VA_ARGS__)
#define AnimationState_setAnimationByName(...) spAnimationState_setAnimationByName(__VA_ARGS__)
#define AnimationState_setAnimation(...) spAnimationState_setAnimation(__VA_ARGS__)
#define AnimationState_addAnimationByName(...) spAnimationState_addAnimationByName(__VA_ARGS__)
#define AnimationState_addAnimation(...) spAnimationState_addAnimation(__VA_ARGS__)
#define AnimationState_setEmptyAnimation(...) spAnimationState_setEmptyAnimation(__VA_ARGS__)
#define AnimationState_addEmptyAnimation(...) spAnimationState_addEmptyAnimation(__VA_ARGS__)
#define AnimationState_setEmptyAnimations(...) spAnimationState_setEmptyAnimations(__VA_ARGS__)
#define AnimationState_getCurrent(...) spAnimationState_getCurrent(__VA_ARGS__)
#define AnimationState_clearListenerNotifications(...) spAnimationState_clearListenerNotifications(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ANIMATIONSTATE_H_ */
