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

#include <spine/Animation.h>
#include <spine/IkConstraint.h>
#include <limits.h>
#include <spine/extension.h>

spAnimation* spAnimation_create (const char* name, int timelinesCount) {
	spAnimation* self = NEW(spAnimation);
	MALLOC_STR(self->name, name);
	self->timelinesCount = timelinesCount;
	self->timelines = MALLOC(spTimeline*, timelinesCount);
	return self;
}

void spAnimation_dispose (spAnimation* self) {
	int i;
	for (i = 0; i < self->timelinesCount; ++i)
		spTimeline_dispose(self->timelines[i]);
	FREE(self->timelines);
	FREE(self->name);
	FREE(self);
}

void spAnimation_apply (const spAnimation* self, spSkeleton* skeleton, float lastTime, float time, int loop, spEvent** events,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int i, n = self->timelinesCount;

	if (loop && self->duration) {
		time = FMOD(time, self->duration);
		if (lastTime > 0) lastTime = FMOD(lastTime, self->duration);
	}

	for (i = 0; i < n; ++i)
		spTimeline_apply(self->timelines[i], skeleton, lastTime, time, events, eventsCount, alpha, blend, direction);
}

/**/

typedef struct _spTimelineVtable {
	void (*apply) (const spTimeline* self, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
		int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction);
	int (*getPropertyId) (const spTimeline* self);
	void (*dispose) (spTimeline* self);
} _spTimelineVtable;

void _spTimeline_init (spTimeline* self, spTimelineType type, /**/
	void (*dispose) (spTimeline* self), /**/
	void (*apply) (const spTimeline* self, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
		int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction),
	int (*getPropertyId) (const spTimeline* self)
) {
	CONST_CAST(spTimelineType, self->type) = type;
	CONST_CAST(_spTimelineVtable*, self->vtable) = NEW(_spTimelineVtable);
	VTABLE(spTimeline, self)->dispose = dispose;
	VTABLE(spTimeline, self)->apply = apply;
	VTABLE(spTimeline, self)->getPropertyId = getPropertyId;
}

void _spTimeline_deinit (spTimeline* self) {
	FREE(self->vtable);
}

void spTimeline_dispose (spTimeline* self) {
	VTABLE(spTimeline, self)->dispose(self);
}

void spTimeline_apply (const spTimeline* self, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
		int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction) {
	VTABLE(spTimeline, self)->apply(self, skeleton, lastTime, time, firedEvents, eventsCount, alpha, blend, direction);
}

int spTimeline_getPropertyId (const spTimeline* self) {
	return VTABLE(spTimeline, self)->getPropertyId(self);
}

/**/

static const float CURVE_LINEAR = 0, CURVE_STEPPED = 1, CURVE_BEZIER = 2;
static const int BEZIER_SIZE = 10 * 2 - 1;

void _spCurveTimeline_init (spCurveTimeline* self, spTimelineType type, int framesCount, /**/
	void (*dispose) (spTimeline* self), /**/
	void (*apply) (const spTimeline* self, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
		int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction),
	int (*getPropertyId)(const spTimeline* self)
) {
	_spTimeline_init(SUPER(self), type, dispose, apply, getPropertyId);
	self->curves = CALLOC(float, (framesCount - 1) * BEZIER_SIZE);
}

void _spCurveTimeline_deinit (spCurveTimeline* self) {
	_spTimeline_deinit(SUPER(self));
	FREE(self->curves);
}

void spCurveTimeline_setLinear (spCurveTimeline* self, int frameIndex) {
	self->curves[frameIndex * BEZIER_SIZE] = CURVE_LINEAR;
}

void spCurveTimeline_setStepped (spCurveTimeline* self, int frameIndex) {
	self->curves[frameIndex * BEZIER_SIZE] = CURVE_STEPPED;
}

void spCurveTimeline_setCurve (spCurveTimeline* self, int frameIndex, float cx1, float cy1, float cx2, float cy2) {
	float tmpx = (-cx1 * 2 + cx2) * 0.03f, tmpy = (-cy1 * 2 + cy2) * 0.03f;
	float dddfx = ((cx1 - cx2) * 3 + 1) * 0.006f, dddfy = ((cy1 - cy2) * 3 + 1) * 0.006f;
	float ddfx = tmpx * 2 + dddfx, ddfy = tmpy * 2 + dddfy;
	float dfx = cx1 * 0.3f + tmpx + dddfx * 0.16666667f, dfy = cy1 * 0.3f + tmpy + dddfy * 0.16666667f;
	float x = dfx, y = dfy;

	int i = frameIndex * BEZIER_SIZE, n = i + BEZIER_SIZE - 1;
	self->curves[i++] = CURVE_BEZIER;

	for (; i < n; i += 2) {
		self->curves[i] = x;
		self->curves[i + 1] = y;
		dfx += ddfx;
		dfy += ddfy;
		ddfx += dddfx;
		ddfy += dddfy;
		x += dfx;
		y += dfy;
	}
}

float spCurveTimeline_getCurvePercent (const spCurveTimeline* self, int frameIndex, float percent) {
	float x, y;
	int i = frameIndex * BEZIER_SIZE, start, n;
	float type = self->curves[i];
	percent = CLAMP(percent, 0, 1);
	if (type == CURVE_LINEAR) return percent;
	if (type == CURVE_STEPPED) return 0;
	i++;
	x = 0;
	for (start = i, n = i + BEZIER_SIZE - 1; i < n; i += 2) {
		x = self->curves[i];
		if (x >= percent) {
			float prevX, prevY;
			if (i == start) {
				prevX = 0;
				prevY = 0;
			} else {
				prevX = self->curves[i - 2];
				prevY = self->curves[i - 1];
			}
			return prevY + (self->curves[i + 1] - prevY) * (percent - prevX) / (x - prevX);
		}
	}
	y = self->curves[i - 1];
	return y + (1 - y) * (percent - x) / (1 - x); /* Last point is 1,1. */
}

/* @param target After the first and before the last entry. */
static int binarySearch (float *values, int valuesLength, float target, int step) {
	int low = 0, current;
	int high = valuesLength / step - 2;
	if (high == 0) return step;
	current = high >> 1;
	while (1) {
		if (values[(current + 1) * step] <= target)
			low = current + 1;
		else
			high = current;
		if (low == high) return (low + 1) * step;
		current = (low + high) >> 1;
	}
	return 0;
}

int _spCurveTimeline_binarySearch (float *values, int valuesLength, float target, int step) {
	return binarySearch(values, valuesLength, target, step);
}

/* @param target After the first and before the last entry. */
static int binarySearch1 (float *values, int valuesLength, float target) {
	int low = 0, current;
	int high = valuesLength - 2;
	if (high == 0) return 1;
	current = high >> 1;
	while (1) {
		if (values[(current + 1)] <= target)
			low = current + 1;
		else
			high = current;
		if (low == high) return low + 1;
		current = (low + high) >> 1;
	}
	return 0;
}

/**/

void _spBaseTimeline_dispose (spTimeline* timeline) {
	struct spBaseTimeline* self = SUB_CAST(struct spBaseTimeline, timeline);
	_spCurveTimeline_deinit(SUPER(self));
	FREE(self->frames);
	FREE(self);
}

/* Many timelines have structure identical to struct spBaseTimeline and extend spCurveTimeline. **/
struct spBaseTimeline* _spBaseTimeline_create (int framesCount, spTimelineType type, int frameSize, /**/
	void (*apply) (const spTimeline* self, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
		int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction),
	int (*getPropertyId) (const spTimeline* self)
) {
	struct spBaseTimeline* self = NEW(struct spBaseTimeline);
	_spCurveTimeline_init(SUPER(self), type, framesCount, _spBaseTimeline_dispose, apply, getPropertyId);

	CONST_CAST(int, self->framesCount) = framesCount * frameSize;
	CONST_CAST(float*, self->frames) = CALLOC(float, self->framesCount);

	return self;
}

/**/

void _spRotateTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	spBone *bone;
	int frame;
	float prevRotation, frameTime, percent, r;
	spRotateTimeline* self = SUB_CAST(spRotateTimeline, timeline);

	bone = skeleton->bones[self->boneIndex];
	if (!bone->active) return;
	if (time < self->frames[0]) {
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			bone->rotation = bone->data->rotation;
			return;
		case SP_MIX_BLEND_FIRST:
			r = bone->data->rotation - bone->rotation;
			r -= (16384 - (int)(16384.499999999996 - r / 360)) * 360;
			bone->rotation += r * alpha;
		case SP_MIX_BLEND_REPLACE:
		case SP_MIX_BLEND_ADD:
			; /* to appease compiler */
		}
		return;
	}

	if (time >= self->frames[self->framesCount - ROTATE_ENTRIES]) { /* Time is after last frame. */
		r = self->frames[self->framesCount + ROTATE_PREV_ROTATION];
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			bone->rotation = bone->data->rotation + r * alpha;
			break;
		case SP_MIX_BLEND_FIRST:
		case SP_MIX_BLEND_REPLACE:
			r += bone->data->rotation - bone->rotation;
			r -= (16384 - (int)(16384.499999999996 - r / 360)) * 360; /* Wrap within -180 and 180. */
		case SP_MIX_BLEND_ADD:
			bone->rotation += r * alpha;
		}
		return;
	}

	/* Interpolate between the previous frame and the current frame. */
	frame = binarySearch(self->frames, self->framesCount, time, ROTATE_ENTRIES);
	prevRotation = self->frames[frame + ROTATE_PREV_ROTATION];
	frameTime = self->frames[frame];
	percent = spCurveTimeline_getCurvePercent(SUPER(self), (frame >> 1) - 1, 1 - (time - frameTime) / (self->frames[frame + ROTATE_PREV_TIME] - frameTime));

	r = self->frames[frame + ROTATE_ROTATION] - prevRotation;
	r = prevRotation + (r - (16384 - (int)(16384.499999999996 - r / 360)) * 360) * percent;
	switch (blend) {
	case SP_MIX_BLEND_SETUP:
		bone->rotation = bone->data->rotation + (r - (16384 - (int)(16384.499999999996 - r / 360)) * 360) * alpha;
		break;
	case SP_MIX_BLEND_FIRST:
	case SP_MIX_BLEND_REPLACE:
		r += bone->data->rotation - bone->rotation;
	case SP_MIX_BLEND_ADD:
		bone->rotation += (r - (16384 - (int)(16384.499999999996 - r / 360)) * 360) * alpha;
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spRotateTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_ROTATE << 25) + SUB_CAST(spRotateTimeline, timeline)->boneIndex;
}

spRotateTimeline* spRotateTimeline_create (int framesCount) {
	return _spBaseTimeline_create(framesCount, SP_TIMELINE_ROTATE, ROTATE_ENTRIES, _spRotateTimeline_apply, _spRotateTimeline_getPropertyId);
}

void spRotateTimeline_setFrame (spRotateTimeline* self, int frameIndex, float time, float degrees) {
	frameIndex <<= 1;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + ROTATE_ROTATION] = degrees;
}

/**/

static const int TRANSLATE_PREV_TIME = -3, TRANSLATE_PREV_X = -2, TRANSLATE_PREV_Y = -1;
static const int TRANSLATE_X = 1, TRANSLATE_Y = 2;

void _spTranslateTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
	spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	spBone *bone;
	int frame;
	float frameTime, percent;
	float x, y;
	float *frames;
	int framesCount;

	spTranslateTimeline* self = SUB_CAST(spTranslateTimeline, timeline);

	bone = skeleton->bones[self->boneIndex];
	if (!bone->active) return;
	if (time < self->frames[0]) {
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			bone->x = bone->data->x;
			bone->y = bone->data->y;
			return;
		case SP_MIX_BLEND_FIRST:
			bone->x += (bone->data->x - bone->x) * alpha;
			bone->y += (bone->data->y - bone->y) * alpha;
		case SP_MIX_BLEND_REPLACE:
		case SP_MIX_BLEND_ADD:
			; /* to appease compiler */
		}
		return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	if (time >= frames[framesCount - TRANSLATE_ENTRIES]) { /* Time is after last frame. */
		x = frames[framesCount + TRANSLATE_PREV_X];
		y = frames[framesCount + TRANSLATE_PREV_Y];
	} else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(frames, framesCount, time, TRANSLATE_ENTRIES);
		x = frames[frame + TRANSLATE_PREV_X];
		y = frames[frame + TRANSLATE_PREV_Y];
		frameTime = frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / TRANSLATE_ENTRIES - 1,
			1 - (time - frameTime) / (frames[frame + TRANSLATE_PREV_TIME] - frameTime));

		x += (frames[frame + TRANSLATE_X] - x) * percent;
		y += (frames[frame + TRANSLATE_Y] - y) * percent;
	}
	switch (blend) {
	case SP_MIX_BLEND_SETUP:
		bone->x = bone->data->x + x * alpha;
		bone->y = bone->data->y + y * alpha;
		break;
	case SP_MIX_BLEND_FIRST:
	case SP_MIX_BLEND_REPLACE:
		bone->x += (bone->data->x + x - bone->x) * alpha;
		bone->y += (bone->data->y + y - bone->y) * alpha;
		break;
	case SP_MIX_BLEND_ADD:
		bone->x += x * alpha;
		bone->y += y * alpha;
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spTranslateTimeline_getPropertyId (const spTimeline* self) {
	return (SP_TIMELINE_TRANSLATE << 24) + SUB_CAST(spTranslateTimeline, self)->boneIndex;
}

spTranslateTimeline* spTranslateTimeline_create (int framesCount) {
	return _spBaseTimeline_create(framesCount, SP_TIMELINE_TRANSLATE, TRANSLATE_ENTRIES, _spTranslateTimeline_apply, _spTranslateTimeline_getPropertyId);
}

void spTranslateTimeline_setFrame (spTranslateTimeline* self, int frameIndex, float time, float x, float y) {
	frameIndex *= TRANSLATE_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + TRANSLATE_X] = x;
	self->frames[frameIndex + TRANSLATE_Y] = y;
}

/**/

void _spScaleTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	spBone *bone;
	int frame;
	float frameTime, percent, x, y;
	float *frames;
	int framesCount;

	spScaleTimeline* self = SUB_CAST(spScaleTimeline, timeline);

	bone = skeleton->bones[self->boneIndex];
	if (!bone->active) return;
	if (time < self->frames[0]) {
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			bone->scaleX = bone->data->scaleX;
			bone->scaleY = bone->data->scaleY;
			return;
		case SP_MIX_BLEND_FIRST:
			bone->scaleX += (bone->data->scaleX - bone->scaleX) * alpha;
			bone->scaleY += (bone->data->scaleY - bone->scaleY) * alpha;
		case SP_MIX_BLEND_REPLACE:
		case SP_MIX_BLEND_ADD:
			; /* to appease compiler */
		}
		return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	if (time >= frames[framesCount - TRANSLATE_ENTRIES]) { /* Time is after last frame. */
		x = frames[framesCount + TRANSLATE_PREV_X] * bone->data->scaleX;
		y = frames[framesCount + TRANSLATE_PREV_Y] * bone->data->scaleY;
	} else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(frames, framesCount, time, TRANSLATE_ENTRIES);
		x = frames[frame + TRANSLATE_PREV_X];
		y = frames[frame + TRANSLATE_PREV_Y];
		frameTime = frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / TRANSLATE_ENTRIES - 1,
			1 - (time - frameTime) / (frames[frame + TRANSLATE_PREV_TIME] - frameTime));

		x = (x + (frames[frame + TRANSLATE_X] - x) * percent) * bone->data->scaleX;
		y = (y + (frames[frame + TRANSLATE_Y] - y) * percent) * bone->data->scaleY;
	}
	if (alpha == 1) {
		if (blend == SP_MIX_BLEND_ADD) {
			bone->scaleX += x - bone->data->scaleX;
			bone->scaleY += y - bone->data->scaleY;
		} else {
			bone->scaleX = x;
			bone->scaleY = y;
		}
	} else {
		float bx, by;
		if (direction == SP_MIX_DIRECTION_OUT) {
			switch (blend) {
			case SP_MIX_BLEND_SETUP:
				bx = bone->data->scaleX;
				by = bone->data->scaleY;
				bone->scaleX = bx + (ABS(x) * SIGNUM(bx) - bx) * alpha;
				bone->scaleY = by + (ABS(y) * SIGNUM(by) - by) * alpha;
				break;
			case SP_MIX_BLEND_FIRST:
			case SP_MIX_BLEND_REPLACE:
				bx = bone->scaleX;
				by = bone->scaleY;
				bone->scaleX = bx + (ABS(x) * SIGNUM(bx) - bx) * alpha;
				bone->scaleY = by + (ABS(y) * SIGNUM(by) - by) * alpha;
				break;
			case SP_MIX_BLEND_ADD:
				bx = bone->scaleX;
				by = bone->scaleY;
				bone->scaleX = bx + (ABS(x) * SIGNUM(bx) - bone->data->scaleX) * alpha;
				bone->scaleY = by + (ABS(y) * SIGNUM(by) - bone->data->scaleY) * alpha;
			}
		} else {
			switch (blend) {
			case SP_MIX_BLEND_SETUP:
				bx = ABS(bone->data->scaleX) * SIGNUM(x);
				by = ABS(bone->data->scaleY) * SIGNUM(y);
				bone->scaleX = bx + (x - bx) * alpha;
				bone->scaleY = by + (y - by) * alpha;
				break;
			case SP_MIX_BLEND_FIRST:
			case SP_MIX_BLEND_REPLACE:
				bx = ABS(bone->scaleX) * SIGNUM(x);
				by = ABS(bone->scaleY) * SIGNUM(y);
				bone->scaleX = bx + (x - bx) * alpha;
				bone->scaleY = by + (y - by) * alpha;
				break;
			case SP_MIX_BLEND_ADD:
				bx = SIGNUM(x);
				by = SIGNUM(y);
				bone->scaleX = ABS(bone->scaleX) * bx + (x - ABS(bone->data->scaleX) * bx) * alpha;
				bone->scaleY = ABS(bone->scaleY) * by + (y - ABS(bone->data->scaleY) * by) * alpha;
			}
		}
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
}

int _spScaleTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_SCALE << 24) + SUB_CAST(spScaleTimeline, timeline)->boneIndex;
}

spScaleTimeline* spScaleTimeline_create (int framesCount) {
	return _spBaseTimeline_create(framesCount, SP_TIMELINE_SCALE, TRANSLATE_ENTRIES, _spScaleTimeline_apply, _spScaleTimeline_getPropertyId);
}

void spScaleTimeline_setFrame (spScaleTimeline* self, int frameIndex, float time, float x, float y) {
	spTranslateTimeline_setFrame(self, frameIndex, time, x, y);
}

/**/

void _spShearTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	spBone *bone;
	int frame;
	float frameTime, percent, x, y;
	float *frames;
	int framesCount;

	spShearTimeline* self = SUB_CAST(spShearTimeline, timeline);

	bone = skeleton->bones[self->boneIndex];
	if (!bone->active) return;
	frames = self->frames;
	framesCount = self->framesCount;
	if (time < self->frames[0]) {
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			bone->shearX = bone->data->shearX;
			bone->shearY = bone->data->shearY;
			return;
		case SP_MIX_BLEND_FIRST:
			bone->shearX += (bone->data->shearX - bone->shearX) * alpha;
			bone->shearY += (bone->data->shearY - bone->shearY) * alpha;
		case SP_MIX_BLEND_REPLACE:
		case SP_MIX_BLEND_ADD:
			; /* to appease compiler */
		}
		return;
	}

	if (time >= frames[framesCount - TRANSLATE_ENTRIES]) { /* Time is after last frame. */
		x = frames[framesCount + TRANSLATE_PREV_X];
		y = frames[framesCount + TRANSLATE_PREV_Y];
	} else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(frames, framesCount, time, TRANSLATE_ENTRIES);
		x = frames[frame + TRANSLATE_PREV_X];
		y = frames[frame + TRANSLATE_PREV_Y];
		frameTime = frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / TRANSLATE_ENTRIES - 1,
			1 - (time - frameTime) / (frames[frame + TRANSLATE_PREV_TIME] - frameTime));

		x = x + (frames[frame + TRANSLATE_X] - x) * percent;
		y = y + (frames[frame + TRANSLATE_Y] - y) * percent;
	}
	switch (blend) {
	case SP_MIX_BLEND_SETUP:
		bone->shearX = bone->data->shearX + x * alpha;
		bone->shearY = bone->data->shearY + y * alpha;
		break;
	case SP_MIX_BLEND_FIRST:
	case SP_MIX_BLEND_REPLACE:
		bone->shearX += (bone->data->shearX + x - bone->shearX) * alpha;
		bone->shearY += (bone->data->shearY + y - bone->shearY) * alpha;
		break;
	case SP_MIX_BLEND_ADD:
		bone->shearX += x * alpha;
		bone->shearY += y * alpha;
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spShearTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_SHEAR << 24) + SUB_CAST(spShearTimeline, timeline)->boneIndex;
}

spShearTimeline* spShearTimeline_create (int framesCount) {
	return (spShearTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_SHEAR, 3, _spShearTimeline_apply, _spShearTimeline_getPropertyId);
}

void spShearTimeline_setFrame (spShearTimeline* self, int frameIndex, float time, float x, float y) {
	spTranslateTimeline_setFrame(self, frameIndex, time, x, y);
}

/**/

static const int COLOR_PREV_TIME = -5, COLOR_PREV_R = -4, COLOR_PREV_G = -3, COLOR_PREV_B = -2, COLOR_PREV_A = -1;
static const int COLOR_R = 1, COLOR_G = 2, COLOR_B = 3, COLOR_A = 4;

void _spColorTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	spSlot *slot;
	int frame;
	float percent, frameTime;
	float r, g, b, a;
	spColor* color;
	spColor* setup;
	spColorTimeline* self = (spColorTimeline*)timeline;
	slot = skeleton->slots[self->slotIndex];
	if (!slot->bone->active) return;

	if (time < self->frames[0]) {
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			spColor_setFromColor(&slot->color, &slot->data->color);
			return;
		case SP_MIX_BLEND_FIRST:
			color = &slot->color;
			setup = &slot->data->color;
			spColor_addFloats(color, (setup->r - color->r) * alpha, (setup->g - color->g) * alpha, (setup->b - color->b) * alpha,
				(setup->a - color->a) * alpha);
		case SP_MIX_BLEND_REPLACE:
		case SP_MIX_BLEND_ADD:
			; /* to appease compiler */
		}
		return;
	}

	if (time >= self->frames[self->framesCount - 5]) { /* Time is after last frame */
		int i = self->framesCount;
		r = self->frames[i + COLOR_PREV_R];
		g = self->frames[i + COLOR_PREV_G];
		b = self->frames[i + COLOR_PREV_B];
		a = self->frames[i + COLOR_PREV_A];
	} else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(self->frames, self->framesCount, time, COLOR_ENTRIES);

		r = self->frames[frame + COLOR_PREV_R];
		g = self->frames[frame + COLOR_PREV_G];
		b = self->frames[frame + COLOR_PREV_B];
		a = self->frames[frame + COLOR_PREV_A];

		frameTime = self->frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / COLOR_ENTRIES - 1,
			1 - (time - frameTime) / (self->frames[frame + COLOR_PREV_TIME] - frameTime));

		r += (self->frames[frame + COLOR_R] - r) * percent;
		g += (self->frames[frame + COLOR_G] - g) * percent;
		b += (self->frames[frame + COLOR_B] - b) * percent;
		a += (self->frames[frame + COLOR_A] - a) * percent;
	}
	if (alpha == 1) {
		spColor_setFromFloats(&slot->color, r, g, b, a);
	} else {
		if (blend == SP_MIX_BLEND_SETUP) spColor_setFromColor(&slot->color, &slot->data->color);
		spColor_addFloats(&slot->color, (r - slot->color.r) * alpha, (g - slot->color.g) * alpha, (b - slot->color.b) * alpha, (a - slot->color.a) * alpha);
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spColorTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_COLOR << 24) + SUB_CAST(spColorTimeline, timeline)->slotIndex;
}

spColorTimeline* spColorTimeline_create (int framesCount) {
	return (spColorTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_COLOR, 5, _spColorTimeline_apply, _spColorTimeline_getPropertyId);
}

void spColorTimeline_setFrame (spColorTimeline* self, int frameIndex, float time, float r, float g, float b, float a) {
	frameIndex *= COLOR_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + COLOR_R] = r;
	self->frames[frameIndex + COLOR_G] = g;
	self->frames[frameIndex + COLOR_B] = b;
	self->frames[frameIndex + COLOR_A] = a;
}

/**/

static const int TWOCOLOR_PREV_TIME = -8, TWOCOLOR_PREV_R = -7, TWOCOLOR_PREV_G = -6, TWOCOLOR_PREV_B = -5, TWOCOLOR_PREV_A = -4;
static const int TWOCOLOR_PREV_R2 = -3, TWOCOLOR_PREV_G2 = -2, TWOCOLOR_PREV_B2 = -1;
static const int TWOCOLOR_R = 1, TWOCOLOR_G = 2, TWOCOLOR_B = 3, TWOCOLOR_A = 4, TWOCOLOR_R2 = 5, TWOCOLOR_G2 = 6, TWOCOLOR_B2 = 7;

void _spTwoColorTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	spSlot *slot;
	int frame;
	float percent, frameTime;
	float r, g, b, a, r2, g2, b2;
	spColor* light;
	spColor* dark;
	spColor* setupLight;
	spColor* setupDark;
	spColorTimeline* self = (spColorTimeline*)timeline;
	slot = skeleton->slots[self->slotIndex];
	if (!slot->bone->active) return;

	if (time < self->frames[0]) {
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			spColor_setFromColor(&slot->color, &slot->data->color);
			spColor_setFromColor(slot->darkColor, slot->data->darkColor);
			return;
		case SP_MIX_BLEND_FIRST:
			light = &slot->color;
			dark = slot->darkColor;
			setupLight = &slot->data->color;
			setupDark = slot->data->darkColor;
			spColor_addFloats(light, (setupLight->r - light->r) * alpha, (setupLight->g - light->g) * alpha, (setupLight->b - light->b) * alpha,
				(setupLight->a - light->a) * alpha);
			spColor_addFloats(dark, (setupDark->r - dark->r) * alpha, (setupDark->g - dark->g) * alpha, (setupDark->b - dark->b) * alpha, 0);
		case SP_MIX_BLEND_REPLACE:
		case SP_MIX_BLEND_ADD:
			; /* to appease compiler */
		}
		return;
	}

	if (time >= self->frames[self->framesCount - TWOCOLOR_ENTRIES]) { /* Time is after last frame */
		int i = self->framesCount;
		r = self->frames[i + TWOCOLOR_PREV_R];
		g = self->frames[i + TWOCOLOR_PREV_G];
		b = self->frames[i + TWOCOLOR_PREV_B];
		a = self->frames[i + TWOCOLOR_PREV_A];
		r2 = self->frames[i + TWOCOLOR_PREV_R2];
		g2 = self->frames[i + TWOCOLOR_PREV_G2];
		b2 = self->frames[i + TWOCOLOR_PREV_B2];
	} else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(self->frames, self->framesCount, time, TWOCOLOR_ENTRIES);

		r = self->frames[frame + TWOCOLOR_PREV_R];
		g = self->frames[frame + TWOCOLOR_PREV_G];
		b = self->frames[frame + TWOCOLOR_PREV_B];
		a = self->frames[frame + TWOCOLOR_PREV_A];
		r2 = self->frames[frame + TWOCOLOR_PREV_R2];
		g2 = self->frames[frame + TWOCOLOR_PREV_G2];
		b2 = self->frames[frame + TWOCOLOR_PREV_B2];

		frameTime = self->frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / TWOCOLOR_ENTRIES - 1,
			1 - (time - frameTime) / (self->frames[frame + TWOCOLOR_PREV_TIME] - frameTime));

		r += (self->frames[frame + TWOCOLOR_R] - r) * percent;
		g += (self->frames[frame + TWOCOLOR_G] - g) * percent;
		b += (self->frames[frame + TWOCOLOR_B] - b) * percent;
		a += (self->frames[frame + TWOCOLOR_A] - a) * percent;
		r2 += (self->frames[frame + TWOCOLOR_R2] - r2) * percent;
		g2 += (self->frames[frame + TWOCOLOR_G2] - g2) * percent;
		b2 += (self->frames[frame + TWOCOLOR_B2] - b2) * percent;
	}
	if (alpha == 1) {
		spColor_setFromFloats(&slot->color, r, g, b, a);
		spColor_setFromFloats(slot->darkColor, r2, g2, b2, 1);
	} else {
		light = &slot->color;
		dark = slot->darkColor;
		if (blend == SP_MIX_BLEND_SETUP) {
			spColor_setFromColor(light, &slot->data->color);
			spColor_setFromColor(dark, slot->data->darkColor);
		}
		spColor_addFloats(light, (r - light->r) * alpha, (g - light->g) * alpha, (b - light->b) * alpha, (a - light->a) * alpha);
		spColor_addFloats(dark, (r2 - dark->r) * alpha, (g2 - dark->g) * alpha, (b2 - dark->b) * alpha, 0);
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spTwoColorTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_TWOCOLOR << 24) + SUB_CAST(spTwoColorTimeline, timeline)->slotIndex;
}

spTwoColorTimeline* spTwoColorTimeline_create (int framesCount) {
	return (spTwoColorTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_TWOCOLOR, TWOCOLOR_ENTRIES, _spTwoColorTimeline_apply, _spTwoColorTimeline_getPropertyId);
}

void spTwoColorTimeline_setFrame (spTwoColorTimeline* self, int frameIndex, float time, float r, float g, float b, float a, float r2, float g2, float b2) {
	frameIndex *= TWOCOLOR_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + TWOCOLOR_R] = r;
	self->frames[frameIndex + TWOCOLOR_G] = g;
	self->frames[frameIndex + TWOCOLOR_B] = b;
	self->frames[frameIndex + TWOCOLOR_A] = a;
	self->frames[frameIndex + TWOCOLOR_R2] = r2;
	self->frames[frameIndex + TWOCOLOR_G2] = g2;
	self->frames[frameIndex + TWOCOLOR_B2] = b2;
}

/**/

void _spAttachmentTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
		spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction) {
	const char* attachmentName;
	spAttachmentTimeline* self = (spAttachmentTimeline*)timeline;
	int frameIndex;
	spSlot* slot = skeleton->slots[self->slotIndex];
	if (!slot->bone->active) return;

	if (direction == SP_MIX_DIRECTION_OUT && blend == SP_MIX_BLEND_SETUP) {
		attachmentName = slot->data->attachmentName;
		spSlot_setAttachment(slot, attachmentName ? spSkeleton_getAttachmentForSlotIndex(skeleton, self->slotIndex, attachmentName) : 0);
		return;
	}

	if (time < self->frames[0]) {
		if (blend == SP_MIX_BLEND_SETUP || blend == SP_MIX_BLEND_FIRST) {
			attachmentName = slot->data->attachmentName;
			spSlot_setAttachment(skeleton->slots[self->slotIndex],
				attachmentName ? spSkeleton_getAttachmentForSlotIndex(skeleton, self->slotIndex, attachmentName) : 0);
		}
		return;
	}

	if (time >= self->frames[self->framesCount - 1])
		frameIndex = self->framesCount - 1;
	else
		frameIndex = binarySearch1(self->frames, self->framesCount, time) - 1;

	attachmentName = self->attachmentNames[frameIndex];
	spSlot_setAttachment(skeleton->slots[self->slotIndex],
		attachmentName ? spSkeleton_getAttachmentForSlotIndex(skeleton, self->slotIndex, attachmentName) : 0);

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(alpha);
}

int _spAttachmentTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_ATTACHMENT << 24) + SUB_CAST(spAttachmentTimeline, timeline)->slotIndex;
}

void _spAttachmentTimeline_dispose (spTimeline* timeline) {
	spAttachmentTimeline* self = SUB_CAST(spAttachmentTimeline, timeline);
	int i;

	_spTimeline_deinit(timeline);

	for (i = 0; i < self->framesCount; ++i)
		FREE(self->attachmentNames[i]);
	FREE(self->attachmentNames);
	FREE(self->frames);
	FREE(self);
}

spAttachmentTimeline* spAttachmentTimeline_create (int framesCount) {
	spAttachmentTimeline* self = NEW(spAttachmentTimeline);
	_spTimeline_init(SUPER(self), SP_TIMELINE_ATTACHMENT, _spAttachmentTimeline_dispose, _spAttachmentTimeline_apply, _spAttachmentTimeline_getPropertyId);

	CONST_CAST(int, self->framesCount) = framesCount;
	CONST_CAST(float*, self->frames) = CALLOC(float, framesCount);
	CONST_CAST(char**, self->attachmentNames) = CALLOC(char*, framesCount);

	return self;
}

void spAttachmentTimeline_setFrame (spAttachmentTimeline* self, int frameIndex, float time, const char* attachmentName) {
	self->frames[frameIndex] = time;

	FREE(self->attachmentNames[frameIndex]);
	if (attachmentName)
		MALLOC_STR(self->attachmentNames[frameIndex], attachmentName);
	else
		self->attachmentNames[frameIndex] = 0;
}

/**/

void _spDeformTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int frame, i, vertexCount;
	float percent, frameTime;
	const float* prevVertices;
	const float* nextVertices;
	float* frames;
	int framesCount;
	const float** frameVertices;
	float* deformArray;
	spDeformTimeline* self = (spDeformTimeline*)timeline;

	spSlot *slot = skeleton->slots[self->slotIndex];
	if (!slot->bone->active) return;

	if (!slot->attachment) return;
	switch (slot->attachment->type) {
		case SP_ATTACHMENT_BOUNDING_BOX:
		case SP_ATTACHMENT_CLIPPING:
		case SP_ATTACHMENT_MESH:
		case SP_ATTACHMENT_PATH: {
			spVertexAttachment* vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
			if (vertexAttachment->deformAttachment != SUB_CAST(spVertexAttachment, self->attachment)) return;
			break;
		}
		default:
			return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	vertexCount = self->frameVerticesCount;
	if (slot->deformCount < vertexCount) {
		if (slot->deformCapacity < vertexCount) {
			FREE(slot->deform);
			slot->deform = MALLOC(float, vertexCount);
			slot->deformCapacity = vertexCount;
		}
	}
	if (slot->deformCount == 0) blend = SP_MIX_BLEND_SETUP;

	frameVertices = self->frameVertices;
	deformArray = slot->deform;

	if (time < frames[0]) { /* Time is before first frame. */
		spVertexAttachment* vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
		switch (blend) {
			case SP_MIX_BLEND_SETUP:
				slot->deformCount = 0;
				return;
			case SP_MIX_BLEND_FIRST:
				if (alpha == 1) {
					slot->deformCount = 0;
					return;
				}
				slot->deformCount = vertexCount;
				if (!vertexAttachment->bones) {
					float* setupVertices = vertexAttachment->vertices;
					for (i = 0; i < vertexCount; i++) {
						deformArray[i] += (setupVertices[i] - deformArray[i]) * alpha;
					}
				} else {
					alpha = 1 - alpha;
					for (i = 0; i < vertexCount; i++) {
						deformArray[i] *= alpha;
					}
				}
			case SP_MIX_BLEND_REPLACE:
			case SP_MIX_BLEND_ADD:
				; /* to appease compiler */
		}
		return;
	}

	slot->deformCount = vertexCount;
	if (time >= frames[framesCount - 1]) { /* Time is after last frame. */
		const float* lastVertices = self->frameVertices[framesCount - 1];
		if (alpha == 1) {
			if (blend == SP_MIX_BLEND_ADD) {
				spVertexAttachment* vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
				if (!vertexAttachment->bones) {
					/* Unweighted vertex positions, with alpha. */
					float* setupVertices = vertexAttachment->vertices;
					for (i = 0; i < vertexCount; i++) {
						deformArray[i] += lastVertices[i] - setupVertices[i];
					}
				} else {
					/* Weighted deform offsets, with alpha. */
					for (i = 0; i < vertexCount; i++)
						deformArray[i] += lastVertices[i];
				}
			} else {
				/* Vertex positions or deform offsets, no alpha. */
				memcpy(deformArray, lastVertices, vertexCount * sizeof(float));
			}
		} else {
			spVertexAttachment* vertexAttachment;
			switch (blend) {
				case SP_MIX_BLEND_SETUP:
					vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
					if (!vertexAttachment->bones) {
						/* Unweighted vertex positions, with alpha. */
						float* setupVertices = vertexAttachment->vertices;
						for (i = 0; i < vertexCount; i++) {
							float setup = setupVertices[i];
							deformArray[i] = setup + (lastVertices[i] - setup) * alpha;
						}
					} else {
						/* Weighted deform offsets, with alpha. */
						for (i = 0; i < vertexCount; i++)
							deformArray[i] = lastVertices[i] * alpha;
					}
					break;
				case SP_MIX_BLEND_FIRST:
				case SP_MIX_BLEND_REPLACE:
					/* Vertex positions or deform offsets, with alpha. */
					for (i = 0; i < vertexCount; i++)
						deformArray[i] += (lastVertices[i] - deformArray[i]) * alpha;
				case SP_MIX_BLEND_ADD:
					vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
					if (!vertexAttachment->bones) {
						/* Unweighted vertex positions, with alpha. */
						float* setupVertices = vertexAttachment->vertices;
						for (i = 0; i < vertexCount; i++) {
							deformArray[i] += (lastVertices[i] - setupVertices[i]) * alpha;
						}
					} else {
						for (i = 0; i < vertexCount; i++)
							deformArray[i] += lastVertices[i] * alpha;
					}
			}
		}
		return;
	}

	/* Interpolate between the previous frame and the current frame. */
	frame = binarySearch(frames, framesCount, time, 1);
	prevVertices = frameVertices[frame - 1];
	nextVertices = frameVertices[frame];
	frameTime = frames[frame];
	percent = spCurveTimeline_getCurvePercent(SUPER(self), frame - 1, 1 - (time - frameTime) / (frames[frame - 1] - frameTime));

	if (alpha == 1) {
		if (blend == SP_MIX_BLEND_ADD) {
			spVertexAttachment* vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
			if (!vertexAttachment->bones) {
				float* setupVertices = vertexAttachment->vertices;
				for (i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deformArray[i] += prev + (nextVertices[i] - prev) * percent - setupVertices[i];
				}
			} else {
				for (i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deformArray[i] += prev + (nextVertices[i] - prev) * percent;
				}
			}
		} else {
			for (i = 0; i < vertexCount; i++) {
				float prev = prevVertices[i];
				deformArray[i] = prev + (nextVertices[i] - prev) * percent;
			}
		}
	} else {
		spVertexAttachment* vertexAttachment;
		switch (blend) {
		case SP_MIX_BLEND_SETUP:
			vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
			if (!vertexAttachment->bones) {
				float *setupVertices = vertexAttachment->vertices;
				for (i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i], setup = setupVertices[i];
					deformArray[i] = setup + (prev + (nextVertices[i] - prev) * percent - setup) * alpha;
				}
			} else {
				for (i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deformArray[i] = (prev + (nextVertices[i] - prev) * percent) * alpha;
				}
			}
			break;
		case SP_MIX_BLEND_FIRST:
		case SP_MIX_BLEND_REPLACE:
			for (i = 0; i < vertexCount; i++) {
				float prev = prevVertices[i];
				deformArray[i] += (prev + (nextVertices[i] - prev) * percent - deformArray[i]) * alpha;
			}
			break;
		case SP_MIX_BLEND_ADD:
			vertexAttachment = SUB_CAST(spVertexAttachment, slot->attachment);
			if (!vertexAttachment->bones) {
				float *setupVertices = vertexAttachment->vertices;
				for (i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deformArray[i] += (prev + (nextVertices[i] - prev) * percent - setupVertices[i]) * alpha;
				}
			} else {
				for (i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deformArray[i] += (prev + (nextVertices[i] - prev) * percent) * alpha;
				}
			}
		}
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spDeformTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_DEFORM << 27) + SUB_CAST(spVertexAttachment, SUB_CAST(spDeformTimeline, timeline)->attachment)->id + SUB_CAST(spDeformTimeline, timeline)->slotIndex;
}

void _spDeformTimeline_dispose (spTimeline* timeline) {
	spDeformTimeline* self = SUB_CAST(spDeformTimeline, timeline);
	int i;

	_spCurveTimeline_deinit(SUPER(self));

	for (i = 0; i < self->framesCount; ++i)
		FREE(self->frameVertices[i]);
	FREE(self->frameVertices);
	FREE(self->frames);
	FREE(self);
}

spDeformTimeline* spDeformTimeline_create (int framesCount, int frameVerticesCount) {
	spDeformTimeline* self = NEW(spDeformTimeline);
	_spCurveTimeline_init(SUPER(self), SP_TIMELINE_DEFORM, framesCount, _spDeformTimeline_dispose, _spDeformTimeline_apply, _spDeformTimeline_getPropertyId);
	CONST_CAST(int, self->framesCount) = framesCount;
	CONST_CAST(float*, self->frames) = CALLOC(float, self->framesCount);
	CONST_CAST(float**, self->frameVertices) = CALLOC(float*, framesCount);
	CONST_CAST(int, self->frameVerticesCount) = frameVerticesCount;
	return self;
}

void spDeformTimeline_setFrame (spDeformTimeline* self, int frameIndex, float time, float* vertices) {
	self->frames[frameIndex] = time;

	FREE(self->frameVertices[frameIndex]);
	if (!vertices)
		self->frameVertices[frameIndex] = 0;
	else {
		self->frameVertices[frameIndex] = MALLOC(float, self->frameVerticesCount);
		memcpy(CONST_CAST(float*, self->frameVertices[frameIndex]), vertices, self->frameVerticesCount * sizeof(float));
	}
}


/**/

/** Fires events for frames > lastTime and <= time. */
void _spEventTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
	int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	spEventTimeline* self = (spEventTimeline*)timeline;
	int frame;
	if (!firedEvents) return;

	if (lastTime > time) { /* Fire events after last time for looped animations. */
		_spEventTimeline_apply(timeline, skeleton, lastTime, (float)INT_MAX, firedEvents, eventsCount, alpha, blend, direction);
		lastTime = -1;
	} else if (lastTime >= self->frames[self->framesCount - 1]) /* Last time is after last frame. */
	return;
	if (time < self->frames[0]) return; /* Time is before first frame. */

	if (lastTime < self->frames[0])
		frame = 0;
	else {
		float frameTime;
		frame = binarySearch1(self->frames, self->framesCount, lastTime);
		frameTime = self->frames[frame];
		while (frame > 0) { /* Fire multiple events with the same frame. */
			if (self->frames[frame - 1] != frameTime) break;
			frame--;
		}
	}
	for (; frame < self->framesCount && time >= self->frames[frame]; ++frame) {
		firedEvents[*eventsCount] = self->events[frame];
		(*eventsCount)++;
	}
	UNUSED(direction);
}

int _spEventTimeline_getPropertyId (const spTimeline* timeline) {
	return SP_TIMELINE_EVENT << 24;
	UNUSED(timeline);
}

void _spEventTimeline_dispose (spTimeline* timeline) {
	spEventTimeline* self = SUB_CAST(spEventTimeline, timeline);
	int i;

	_spTimeline_deinit(timeline);

	for (i = 0; i < self->framesCount; ++i)
		spEvent_dispose(self->events[i]);
	FREE(self->events);
	FREE(self->frames);
	FREE(self);
}

spEventTimeline* spEventTimeline_create (int framesCount) {
	spEventTimeline* self = NEW(spEventTimeline);
	_spTimeline_init(SUPER(self), SP_TIMELINE_EVENT, _spEventTimeline_dispose, _spEventTimeline_apply, _spEventTimeline_getPropertyId);

	CONST_CAST(int, self->framesCount) = framesCount;
	CONST_CAST(float*, self->frames) = CALLOC(float, framesCount);
	CONST_CAST(spEvent**, self->events) = CALLOC(spEvent*, framesCount);

	return self;
}

void spEventTimeline_setFrame (spEventTimeline* self, int frameIndex, spEvent* event) {
	self->frames[frameIndex] = event->time;

	FREE(self->events[frameIndex]);
	self->events[frameIndex] = event;
}

/**/

void _spDrawOrderTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
	spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int i;
	int frame;
	const int* drawOrderToSetupIndex;
	spDrawOrderTimeline* self = (spDrawOrderTimeline*)timeline;

	if (direction == SP_MIX_DIRECTION_OUT && blend == SP_MIX_BLEND_SETUP) {
		memcpy(skeleton->drawOrder, skeleton->slots, self->slotsCount * sizeof(spSlot*));
		return;
	}

	if (time < self->frames[0]) {
		if (blend == SP_MIX_BLEND_SETUP || blend == SP_MIX_BLEND_FIRST) memcpy(skeleton->drawOrder, skeleton->slots, self->slotsCount * sizeof(spSlot*));
		return;
	}

	if (time >= self->frames[self->framesCount - 1]) /* Time is after last frame. */
		frame = self->framesCount - 1;
	else
		frame = binarySearch1(self->frames, self->framesCount, time) - 1;

	drawOrderToSetupIndex = self->drawOrders[frame];
	if (!drawOrderToSetupIndex)
		memcpy(skeleton->drawOrder, skeleton->slots, self->slotsCount * sizeof(spSlot*));
	else {
		for (i = 0; i < self->slotsCount; ++i)
			skeleton->drawOrder[i] = skeleton->slots[drawOrderToSetupIndex[i]];
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(alpha);
}

int _spDrawOrderTimeline_getPropertyId (const spTimeline* timeline) {
	return SP_TIMELINE_DRAWORDER << 24;
	UNUSED(timeline);
}

void _spDrawOrderTimeline_dispose (spTimeline* timeline) {
	spDrawOrderTimeline* self = SUB_CAST(spDrawOrderTimeline, timeline);
	int i;

	_spTimeline_deinit(timeline);

	for (i = 0; i < self->framesCount; ++i)
		FREE(self->drawOrders[i]);
	FREE(self->drawOrders);
	FREE(self->frames);
	FREE(self);
}

spDrawOrderTimeline* spDrawOrderTimeline_create (int framesCount, int slotsCount) {
	spDrawOrderTimeline* self = NEW(spDrawOrderTimeline);
	_spTimeline_init(SUPER(self), SP_TIMELINE_DRAWORDER, _spDrawOrderTimeline_dispose, _spDrawOrderTimeline_apply, _spDrawOrderTimeline_getPropertyId);

	CONST_CAST(int, self->framesCount) = framesCount;
	CONST_CAST(float*, self->frames) = CALLOC(float, framesCount);
	CONST_CAST(int**, self->drawOrders) = CALLOC(int*, framesCount);
	CONST_CAST(int, self->slotsCount) = slotsCount;

	return self;
}

void spDrawOrderTimeline_setFrame (spDrawOrderTimeline* self, int frameIndex, float time, const int* drawOrder) {
	self->frames[frameIndex] = time;

	FREE(self->drawOrders[frameIndex]);
	if (!drawOrder)
		self->drawOrders[frameIndex] = 0;
	else {
		self->drawOrders[frameIndex] = MALLOC(int, self->slotsCount);
		memcpy(CONST_CAST(int*, self->drawOrders[frameIndex]), drawOrder, self->slotsCount * sizeof(int));
	}
}

/**/

static const int IKCONSTRAINT_PREV_TIME = -6, IKCONSTRAINT_PREV_MIX = -5, IKCONSTRAINT_PREV_SOFTNESS = -4, IKCONSTRAINT_PREV_BEND_DIRECTION = -3, IKCONSTRAINT_PREV_COMPRESS = -2, IKCONSTRAINT_PREV_STRETCH = -1;
static const int IKCONSTRAINT_MIX = 1, IKCONSTRAINT_SOFTNESS = 2, IKCONSTRAINT_BEND_DIRECTION = 3, IKCONSTRAINT_COMPRESS = 4, IKCONSTRAINT_STRETCH = 5;

void _spIkConstraintTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
	spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int frame;
	float frameTime, percent, mix, softness;
	float *frames;
	int framesCount;
	spIkConstraint* constraint;
	spIkConstraintTimeline* self = (spIkConstraintTimeline*)timeline;

	constraint = skeleton->ikConstraints[self->ikConstraintIndex];
	if (!constraint->active) return;

	if (time < self->frames[0]) {
		switch (blend) {
			case SP_MIX_BLEND_SETUP:
				constraint->mix = constraint->data->mix;
				constraint->softness = constraint->data->softness;
				constraint->bendDirection = constraint->data->bendDirection;
				constraint->compress = constraint->data->compress;
				constraint->stretch = constraint->data->stretch;
				return;
			case SP_MIX_BLEND_FIRST:
				constraint->mix += (constraint->data->mix - constraint->mix) * alpha;
				constraint->softness += (constraint->data->softness - constraint->softness) * alpha;
				constraint->bendDirection = constraint->data->bendDirection;
				constraint->compress = constraint->data->compress;
				constraint->stretch = constraint->data->stretch;
			case SP_MIX_BLEND_REPLACE:
			case SP_MIX_BLEND_ADD:
				; /* to appease compiler */
		}
		return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	if (time >= frames[framesCount - IKCONSTRAINT_ENTRIES]) { /* Time is after last frame. */
		if (blend == SP_MIX_BLEND_SETUP) {
			constraint->mix = constraint->data->mix + (frames[framesCount + IKCONSTRAINT_PREV_MIX] - constraint->data->mix) * alpha;
			constraint->softness = constraint->data->softness
				+ (frames[framesCount + IKCONSTRAINT_PREV_SOFTNESS] - constraint->data->softness) * alpha;
			if (direction == SP_MIX_DIRECTION_OUT) {
				constraint->bendDirection = constraint->data->bendDirection;
				constraint->compress = constraint->data->compress;
				constraint->stretch = constraint->data->stretch;
			} else {
				constraint->bendDirection = (int)frames[framesCount + IKCONSTRAINT_PREV_BEND_DIRECTION];
				constraint->compress = frames[framesCount + IKCONSTRAINT_PREV_COMPRESS] ? 1 : 0;
				constraint->stretch = frames[framesCount + IKCONSTRAINT_PREV_STRETCH] ? 1 : 0;
			}
		} else {
			constraint->mix += (frames[framesCount + IKCONSTRAINT_PREV_MIX] - constraint->mix) * alpha;
			constraint->softness += (frames[framesCount + IKCONSTRAINT_PREV_SOFTNESS] - constraint->softness) * alpha;
			if (direction == SP_MIX_DIRECTION_IN) {
				constraint->bendDirection = (int)frames[framesCount + IKCONSTRAINT_PREV_BEND_DIRECTION];
				constraint->compress = frames[framesCount + IKCONSTRAINT_PREV_COMPRESS] ? 1 : 0;
				constraint->stretch = frames[framesCount + IKCONSTRAINT_PREV_STRETCH] ? 1 : 0;
			}
		}
		return;
	}

	/* Interpolate between the previous frame and the current frame. */
	frame = binarySearch(self->frames, self->framesCount, time, IKCONSTRAINT_ENTRIES);
	mix = self->frames[frame + IKCONSTRAINT_PREV_MIX];
	softness = frames[frame + IKCONSTRAINT_PREV_SOFTNESS];
	frameTime = self->frames[frame];
	percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / IKCONSTRAINT_ENTRIES - 1, 1 - (time - frameTime) / (self->frames[frame + IKCONSTRAINT_PREV_TIME] - frameTime));

	if (blend == SP_MIX_BLEND_SETUP) {
		constraint->mix = constraint->data->mix + (mix + (frames[frame + IKCONSTRAINT_MIX] - mix) * percent - constraint->data->mix) * alpha;
		constraint->softness = constraint->data->softness
			+ (softness + (frames[frame + IKCONSTRAINT_SOFTNESS] - softness) * percent - constraint->data->softness) * alpha;
		if (direction == SP_MIX_DIRECTION_OUT) {
			constraint->bendDirection = constraint->data->bendDirection;
			constraint->compress = constraint->data->compress;
			constraint->stretch = constraint->data->stretch;
		} else {
			constraint->bendDirection = (int)frames[frame + IKCONSTRAINT_PREV_BEND_DIRECTION];
			constraint->compress = frames[frame + IKCONSTRAINT_PREV_COMPRESS] ? 1 : 0;
			constraint->stretch = frames[frame + IKCONSTRAINT_PREV_STRETCH] ? 1 : 0;
		}
	} else {
		constraint->mix += (mix + (frames[frame + IKCONSTRAINT_MIX] - mix) * percent - constraint->mix) * alpha;
		constraint->softness += (softness + (frames[frame + IKCONSTRAINT_SOFTNESS] - softness) * percent - constraint->softness) * alpha;
		if (direction == SP_MIX_DIRECTION_IN) {
			constraint->bendDirection = (int)frames[frame + IKCONSTRAINT_PREV_BEND_DIRECTION];
			constraint->compress = frames[frame + IKCONSTRAINT_PREV_COMPRESS] ? 1 : 0;
			constraint->stretch = frames[frame + IKCONSTRAINT_PREV_STRETCH] ? 1 : 0;
		}
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
}

int _spIkConstraintTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_IKCONSTRAINT << 24) + SUB_CAST(spIkConstraintTimeline, timeline)->ikConstraintIndex;
}

spIkConstraintTimeline* spIkConstraintTimeline_create (int framesCount) {
	return (spIkConstraintTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_IKCONSTRAINT, IKCONSTRAINT_ENTRIES, _spIkConstraintTimeline_apply, _spIkConstraintTimeline_getPropertyId);
}

void spIkConstraintTimeline_setFrame (spIkConstraintTimeline* self, int frameIndex, float time, float mix, float softness,
	int bendDirection, int /*boolean*/ compress, int /*boolean*/ stretch
) {
	frameIndex *= IKCONSTRAINT_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + IKCONSTRAINT_MIX] = mix;
	self->frames[frameIndex + IKCONSTRAINT_SOFTNESS] = softness;
	self->frames[frameIndex + IKCONSTRAINT_BEND_DIRECTION] = (float)bendDirection;
	self->frames[frameIndex + IKCONSTRAINT_COMPRESS] = compress ? 1 : 0;
	self->frames[frameIndex + IKCONSTRAINT_STRETCH] = stretch ? 1 : 0;
}

/**/
static const int TRANSFORMCONSTRAINT_PREV_TIME = -5;
static const int TRANSFORMCONSTRAINT_PREV_ROTATE = -4;
static const int TRANSFORMCONSTRAINT_PREV_TRANSLATE = -3;
static const int TRANSFORMCONSTRAINT_PREV_SCALE = -2;
static const int TRANSFORMCONSTRAINT_PREV_SHEAR = -1;
static const int TRANSFORMCONSTRAINT_ROTATE = 1;
static const int TRANSFORMCONSTRAINT_TRANSLATE = 2;
static const int TRANSFORMCONSTRAINT_SCALE = 3;
static const int TRANSFORMCONSTRAINT_SHEAR = 4;

void _spTransformConstraintTimeline_apply (const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
	spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int frame;
	float frameTime, percent, rotate, translate, scale, shear;
	spTransformConstraint* constraint;
	spTransformConstraintTimeline* self = (spTransformConstraintTimeline*)timeline;
	float *frames;
	int framesCount;

	constraint = skeleton->transformConstraints[self->transformConstraintIndex];
	if (!constraint->active) return;

	if (time < self->frames[0]) {
		spTransformConstraintData* data = constraint->data;
		switch (blend) {
			case SP_MIX_BLEND_SETUP:
				constraint->rotateMix = data->rotateMix;
				constraint->translateMix = data->translateMix;
				constraint->scaleMix = data->scaleMix;
				constraint->shearMix = data->shearMix;
				return;
			case SP_MIX_BLEND_FIRST:
				constraint->rotateMix += (data->rotateMix - constraint->rotateMix) * alpha;
				constraint->translateMix += (data->translateMix - constraint->translateMix) * alpha;
				constraint->scaleMix += (data->scaleMix - constraint->scaleMix) * alpha;
				constraint->shearMix += (data->shearMix - constraint->shearMix) * alpha;
			case SP_MIX_BLEND_REPLACE:
			case SP_MIX_BLEND_ADD:
				; /* to appease compiler */
		}
		return;
		return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	if (time >= frames[framesCount - TRANSFORMCONSTRAINT_ENTRIES]) { /* Time is after last frame. */
		int i = framesCount;
		rotate = frames[i + TRANSFORMCONSTRAINT_PREV_ROTATE];
		translate = frames[i + TRANSFORMCONSTRAINT_PREV_TRANSLATE];
		scale = frames[i + TRANSFORMCONSTRAINT_PREV_SCALE];
		shear = frames[i + TRANSFORMCONSTRAINT_PREV_SHEAR];
	} else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(frames, framesCount, time, TRANSFORMCONSTRAINT_ENTRIES);
		rotate = frames[frame + TRANSFORMCONSTRAINT_PREV_ROTATE];
		translate = frames[frame + TRANSFORMCONSTRAINT_PREV_TRANSLATE];
		scale = frames[frame + TRANSFORMCONSTRAINT_PREV_SCALE];
		shear = frames[frame + TRANSFORMCONSTRAINT_PREV_SHEAR];
		frameTime = frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / TRANSFORMCONSTRAINT_ENTRIES - 1,
										1 - (time - frameTime) / (frames[frame + TRANSFORMCONSTRAINT_PREV_TIME] - frameTime));

		rotate += (frames[frame + TRANSFORMCONSTRAINT_ROTATE] - rotate) * percent;
		translate += (frames[frame + TRANSFORMCONSTRAINT_TRANSLATE] - translate) * percent;
		scale += (frames[frame + TRANSFORMCONSTRAINT_SCALE] - scale) * percent;
		shear += (frames[frame + TRANSFORMCONSTRAINT_SHEAR] - shear) * percent;
	}
	if (blend == SP_MIX_BLEND_SETUP) {
		spTransformConstraintData* data = constraint->data;
		constraint->rotateMix = data->rotateMix + (rotate - data->rotateMix) * alpha;
		constraint->translateMix = data->translateMix + (translate - data->translateMix) * alpha;
		constraint->scaleMix = data->scaleMix + (scale - data->scaleMix) * alpha;
		constraint->shearMix = data->shearMix + (shear - data->shearMix) * alpha;
	} else {
		constraint->rotateMix += (rotate - constraint->rotateMix) * alpha;
		constraint->translateMix += (translate - constraint->translateMix) * alpha;
		constraint->scaleMix += (scale - constraint->scaleMix) * alpha;
		constraint->shearMix += (shear - constraint->shearMix) * alpha;
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spTransformConstraintTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_TRANSFORMCONSTRAINT << 24) + SUB_CAST(spTransformConstraintTimeline, timeline)->transformConstraintIndex;
}

spTransformConstraintTimeline* spTransformConstraintTimeline_create (int framesCount) {
	return (spTransformConstraintTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_TRANSFORMCONSTRAINT,
		TRANSFORMCONSTRAINT_ENTRIES, _spTransformConstraintTimeline_apply, _spTransformConstraintTimeline_getPropertyId);
}

void spTransformConstraintTimeline_setFrame (spTransformConstraintTimeline* self, int frameIndex, float time, float rotateMix,
	float translateMix, float scaleMix, float shearMix
) {
	frameIndex *= TRANSFORMCONSTRAINT_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + TRANSFORMCONSTRAINT_ROTATE] = rotateMix;
	self->frames[frameIndex + TRANSFORMCONSTRAINT_TRANSLATE] = translateMix;
	self->frames[frameIndex + TRANSFORMCONSTRAINT_SCALE] = scaleMix;
	self->frames[frameIndex + TRANSFORMCONSTRAINT_SHEAR] = shearMix;
}

/**/

static const int PATHCONSTRAINTPOSITION_PREV_TIME = -2;
static const int PATHCONSTRAINTPOSITION_PREV_VALUE = -1;
static const int PATHCONSTRAINTPOSITION_VALUE = 1;

void _spPathConstraintPositionTimeline_apply(const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
	spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int frame;
	float frameTime, percent, position;
	spPathConstraint* constraint;
	spPathConstraintPositionTimeline* self = (spPathConstraintPositionTimeline*)timeline;
	float* frames;
	int framesCount;

	constraint = skeleton->pathConstraints[self->pathConstraintIndex];
	if (!constraint->active) return;

	if (time < self->frames[0]) {
		switch (blend) {
			case SP_MIX_BLEND_SETUP:
				constraint->position = constraint->data->position;
				return;
			case SP_MIX_BLEND_FIRST:
				constraint->position += (constraint->data->position - constraint->position) * alpha;
			case SP_MIX_BLEND_REPLACE:
			case SP_MIX_BLEND_ADD:
				; /* to appease compiler */
		}
		return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	if (time >= frames[framesCount - PATHCONSTRAINTPOSITION_ENTRIES]) /* Time is after last frame. */
		position = frames[framesCount + PATHCONSTRAINTPOSITION_PREV_VALUE];
	else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(frames, framesCount, time, PATHCONSTRAINTPOSITION_ENTRIES);
		position = frames[frame + PATHCONSTRAINTPOSITION_PREV_VALUE];
		frameTime = frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / PATHCONSTRAINTPOSITION_ENTRIES - 1,
										1 - (time - frameTime) / (frames[frame + PATHCONSTRAINTPOSITION_PREV_TIME] - frameTime));

		position += (frames[frame + PATHCONSTRAINTPOSITION_VALUE] - position) * percent;
	}
	if (blend == SP_MIX_BLEND_SETUP)
		constraint->position = constraint->data->position + (position - constraint->data->position) * alpha;
	else
		constraint->position += (position - constraint->position) * alpha;

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spPathConstraintPositionTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_PATHCONSTRAINTPOSITION << 24) + SUB_CAST(spPathConstraintPositionTimeline, timeline)->pathConstraintIndex;
}

spPathConstraintPositionTimeline* spPathConstraintPositionTimeline_create (int framesCount) {
	return (spPathConstraintPositionTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_PATHCONSTRAINTPOSITION,
		PATHCONSTRAINTPOSITION_ENTRIES, _spPathConstraintPositionTimeline_apply, _spPathConstraintPositionTimeline_getPropertyId);
}

void spPathConstraintPositionTimeline_setFrame (spPathConstraintPositionTimeline* self, int frameIndex, float time, float value) {
	frameIndex *= PATHCONSTRAINTPOSITION_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + PATHCONSTRAINTPOSITION_VALUE] = value;
}

/**/
static const int PATHCONSTRAINTSPACING_PREV_TIME = -2;
static const int PATHCONSTRAINTSPACING_PREV_VALUE = -1;
static const int PATHCONSTRAINTSPACING_VALUE = 1;

void _spPathConstraintSpacingTimeline_apply(const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
	spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int frame;
	float frameTime, percent, spacing;
	spPathConstraint* constraint;
	spPathConstraintSpacingTimeline* self = (spPathConstraintSpacingTimeline*)timeline;
	float* frames;
	int framesCount;

	constraint = skeleton->pathConstraints[self->pathConstraintIndex];
	if (!constraint->active) return;

	if (time < self->frames[0]) {
		switch (blend) {
			case SP_MIX_BLEND_SETUP:
				constraint->spacing = constraint->data->spacing;
				return;
			case SP_MIX_BLEND_FIRST:
				constraint->spacing += (constraint->data->spacing - constraint->spacing) * alpha;
			case SP_MIX_BLEND_REPLACE:
			case SP_MIX_BLEND_ADD:
				; /* to appease compiler */
		}
		return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	if (time >= frames[framesCount - PATHCONSTRAINTSPACING_ENTRIES]) /* Time is after last frame. */
		spacing = frames[framesCount + PATHCONSTRAINTSPACING_PREV_VALUE];
	else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(frames, framesCount, time, PATHCONSTRAINTSPACING_ENTRIES);
		spacing = frames[frame + PATHCONSTRAINTSPACING_PREV_VALUE];
		frameTime = frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / PATHCONSTRAINTSPACING_ENTRIES - 1,
										1 - (time - frameTime) / (frames[frame + PATHCONSTRAINTSPACING_PREV_TIME] - frameTime));

		spacing += (frames[frame + PATHCONSTRAINTSPACING_VALUE] - spacing) * percent;
	}

	if (blend == SP_MIX_BLEND_SETUP)
		constraint->spacing = constraint->data->spacing + (spacing - constraint->data->spacing) * alpha;
	else
		constraint->spacing += (spacing - constraint->spacing) * alpha;

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spPathConstraintSpacingTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_PATHCONSTRAINTSPACING << 24) + SUB_CAST(spPathConstraintSpacingTimeline, timeline)->pathConstraintIndex;
}

spPathConstraintSpacingTimeline* spPathConstraintSpacingTimeline_create (int framesCount) {
	return (spPathConstraintSpacingTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_PATHCONSTRAINTSPACING,
		PATHCONSTRAINTSPACING_ENTRIES, _spPathConstraintSpacingTimeline_apply, _spPathConstraintSpacingTimeline_getPropertyId);
}

void spPathConstraintSpacingTimeline_setFrame (spPathConstraintSpacingTimeline* self, int frameIndex, float time, float value) {
	frameIndex *= PATHCONSTRAINTSPACING_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + PATHCONSTRAINTSPACING_VALUE] = value;
}

/**/

static const int PATHCONSTRAINTMIX_PREV_TIME = -3;
static const int PATHCONSTRAINTMIX_PREV_ROTATE = -2;
static const int PATHCONSTRAINTMIX_PREV_TRANSLATE = -1;
static const int PATHCONSTRAINTMIX_ROTATE = 1;
static const int PATHCONSTRAINTMIX_TRANSLATE = 2;

void _spPathConstraintMixTimeline_apply(const spTimeline* timeline, spSkeleton* skeleton, float lastTime, float time,
	spEvent** firedEvents, int* eventsCount, float alpha, spMixBlend blend, spMixDirection direction
) {
	int frame;
	float frameTime, percent, rotate, translate;
	spPathConstraint* constraint;
	spPathConstraintMixTimeline* self = (spPathConstraintMixTimeline*)timeline;
	float* frames;
	int framesCount;

	constraint = skeleton->pathConstraints[self->pathConstraintIndex];
	if (!constraint->active) return;

	if (time < self->frames[0]) {
		switch (blend) {
			case SP_MIX_BLEND_SETUP:
				constraint->rotateMix = constraint->data->rotateMix;
				constraint->translateMix = constraint->data->translateMix;
				return;
			case SP_MIX_BLEND_FIRST:
				constraint->rotateMix += (constraint->data->rotateMix - constraint->rotateMix) * alpha;
				constraint->translateMix += (constraint->data->translateMix - constraint->translateMix) * alpha;
			case SP_MIX_BLEND_REPLACE:
			case SP_MIX_BLEND_ADD:
				; /* to appease compiler */
		}
		return;
	}

	frames = self->frames;
	framesCount = self->framesCount;
	if (time >= frames[framesCount - PATHCONSTRAINTMIX_ENTRIES]) { /* Time is after last frame. */
		rotate = frames[framesCount + PATHCONSTRAINTMIX_PREV_ROTATE];
		translate = frames[framesCount + PATHCONSTRAINTMIX_PREV_TRANSLATE];
	} else {
		/* Interpolate between the previous frame and the current frame. */
		frame = binarySearch(frames, framesCount, time, PATHCONSTRAINTMIX_ENTRIES);
		rotate = frames[frame + PATHCONSTRAINTMIX_PREV_ROTATE];
		translate = frames[frame + PATHCONSTRAINTMIX_PREV_TRANSLATE];
		frameTime = frames[frame];
		percent = spCurveTimeline_getCurvePercent(SUPER(self), frame / PATHCONSTRAINTMIX_ENTRIES - 1,
										1 - (time - frameTime) / (frames[frame + PATHCONSTRAINTMIX_PREV_TIME] - frameTime));

		rotate += (frames[frame + PATHCONSTRAINTMIX_ROTATE] - rotate) * percent;
		translate += (frames[frame + PATHCONSTRAINTMIX_TRANSLATE] - translate) * percent;
	}

	if (blend == SP_MIX_BLEND_SETUP) {
		constraint->rotateMix = constraint->data->rotateMix + (rotate - constraint->data->rotateMix) * alpha;
		constraint->translateMix = constraint->data->translateMix + (translate - constraint->data->translateMix) * alpha;
	} else {
		constraint->rotateMix += (rotate - constraint->rotateMix) * alpha;
		constraint->translateMix += (translate - constraint->translateMix) * alpha;
	}

	UNUSED(lastTime);
	UNUSED(firedEvents);
	UNUSED(eventsCount);
	UNUSED(direction);
}

int _spPathConstraintMixTimeline_getPropertyId (const spTimeline* timeline) {
	return (SP_TIMELINE_PATHCONSTRAINTMIX << 24) + SUB_CAST(spPathConstraintMixTimeline, timeline)->pathConstraintIndex;
}

spPathConstraintMixTimeline* spPathConstraintMixTimeline_create (int framesCount) {
	return (spPathConstraintMixTimeline*)_spBaseTimeline_create(framesCount, SP_TIMELINE_PATHCONSTRAINTMIX,
		PATHCONSTRAINTMIX_ENTRIES, _spPathConstraintMixTimeline_apply, _spPathConstraintMixTimeline_getPropertyId);
}

void spPathConstraintMixTimeline_setFrame (spPathConstraintMixTimeline* self, int frameIndex, float time, float rotateMix, float translateMix) {
	frameIndex *= PATHCONSTRAINTMIX_ENTRIES;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + PATHCONSTRAINTMIX_ROTATE] = rotateMix;
	self->frames[frameIndex + PATHCONSTRAINTMIX_TRANSLATE] = translateMix;
}
