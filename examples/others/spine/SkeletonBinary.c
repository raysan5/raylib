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

#include <spine/SkeletonBinary.h>
#include <stdio.h>
#include <spine/extension.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/Animation.h>
#include <spine/Array.h>

typedef struct {
	const unsigned char* cursor;
	const unsigned char* end;
} _dataInput;

typedef struct {
	const char* parent;
	const char* skin;
	int slotIndex;
	spMeshAttachment* mesh;
	int inheritDeform;
} _spLinkedMesh;

typedef struct {
	spSkeletonBinary super;
	int ownsLoader;

	int linkedMeshCount;
	int linkedMeshCapacity;
	_spLinkedMesh* linkedMeshes;
} _spSkeletonBinary;

spSkeletonBinary* spSkeletonBinary_createWithLoader (spAttachmentLoader* attachmentLoader) {
	spSkeletonBinary* self = SUPER(NEW(_spSkeletonBinary));
	self->scale = 1;
	self->attachmentLoader = attachmentLoader;
	return self;
}

spSkeletonBinary* spSkeletonBinary_create (spAtlas* atlas) {
	spAtlasAttachmentLoader* attachmentLoader = spAtlasAttachmentLoader_create(atlas);
	spSkeletonBinary* self = spSkeletonBinary_createWithLoader(SUPER(attachmentLoader));
	SUB_CAST(_spSkeletonBinary, self)->ownsLoader = 1;
	return self;
}

void spSkeletonBinary_dispose (spSkeletonBinary* self) {
	_spSkeletonBinary* internal = SUB_CAST(_spSkeletonBinary, self);
	if (internal->ownsLoader) spAttachmentLoader_dispose(self->attachmentLoader);
	FREE(internal->linkedMeshes);
	FREE(self->error);
	FREE(self);
}

void _spSkeletonBinary_setError (spSkeletonBinary* self, const char* value1, const char* value2) {
	char message[256];
	int length;
	FREE(self->error);
	strcpy(message, value1);
	length = (int)strlen(value1);
	if (value2) strncat(message + length, value2, 255 - length);
	MALLOC_STR(self->error, message);
}

static unsigned char readByte (_dataInput* input) {
	return *input->cursor++;
}

static signed char readSByte (_dataInput* input) {
	return (signed char)readByte(input);
}

static int readBoolean (_dataInput* input) {
	return readByte(input) != 0;
}

static int readInt (_dataInput* input) {
	int result = readByte(input);
	result <<= 8;
	result |= readByte(input);
	result <<= 8;
	result |= readByte(input);
	result <<= 8;
	result |= readByte(input);
	return result;
}

static int readVarint (_dataInput* input, int/*bool*/optimizePositive) {
	unsigned char b = readByte(input);
	int value = b & 0x7F;
	if (b & 0x80) {
		b = readByte(input);
		value |= (b & 0x7F) << 7;
		if (b & 0x80) {
			b = readByte(input);
			value |= (b & 0x7F) << 14;
			if (b & 0x80) {
				b = readByte(input);
				value |= (b & 0x7F) << 21;
				if (b & 0x80) value |= (readByte(input) & 0x7F) << 28;
			}
		}
	}
	if (!optimizePositive) value = (((unsigned int)value >> 1) ^ -(value & 1));
	return value;
}

float readFloat (_dataInput* input) {
	union {
		int intValue;
		float floatValue;
	} intToFloat;
	intToFloat.intValue = readInt(input);
	return intToFloat.floatValue;
}

char* readString (_dataInput* input) {
	int length = readVarint(input, 1);
	char *string;
	if (length == 0) {
		return 0;
	}
	string = MALLOC(char, length);
	memcpy(string, input->cursor, length - 1);
	input->cursor += length - 1;
	string[length - 1] = '\0';
	return string;
}

static char* readStringRef(_dataInput* input, spSkeletonData* skeletonData) {
	int index = readVarint(input, 1);
	return index == 0 ? 0 : skeletonData->strings[index - 1];
}

static void readColor (_dataInput* input, float *r, float *g, float *b, float *a) {
	*r = readByte(input) / 255.0f;
	*g = readByte(input) / 255.0f;
	*b = readByte(input) / 255.0f;
	*a = readByte(input) / 255.0f;
}

#define ATTACHMENT_REGION 0
#define ATTACHMENT_BOUNDING_BOX 1
#define ATTACHMENT_MESH 2
#define ATTACHMENT_LINKED_MESH 3
#define ATTACHMENT_PATH 4

#define BLEND_MODE_NORMAL 0
#define BLEND_MODE_ADDITIVE 1
#define BLEND_MODE_MULTIPLY 2
#define BLEND_MODE_SCREEN 3

#define CURVE_LINEAR 0
#define CURVE_STEPPED 1
#define CURVE_BEZIER 2

#define BONE_ROTATE 0
#define BONE_TRANSLATE 1
#define BONE_SCALE 2
#define BONE_SHEAR 3

#define SLOT_ATTACHMENT 0
#define SLOT_COLOR 1
#define SLOT_TWO_COLOR 2

#define PATH_POSITION 0
#define PATH_SPACING 1
#define PATH_MIX 2

#define PATH_POSITION_FIXED 0
#define PATH_POSITION_PERCENT 1

#define PATH_SPACING_LENGTH 0
#define PATH_SPACING_FIXED 1
#define PATH_SPACING_PERCENT 2

#define PATH_ROTATE_TANGENT 0
#define PATH_ROTATE_CHAIN 1
#define PATH_ROTATE_CHAIN_SCALE 2

static void readCurve (_dataInput* input, spCurveTimeline* timeline, int frameIndex) {
	switch (readByte(input)) {
	case CURVE_STEPPED: {
		spCurveTimeline_setStepped(timeline, frameIndex);
		break;
	}
	case CURVE_BEZIER: {
		float cx1 = readFloat(input);
		float cy1 = readFloat(input);
		float cx2 = readFloat(input);
		float cy2 = readFloat(input);
		spCurveTimeline_setCurve(timeline, frameIndex, cx1, cy1, cx2, cy2);
		break;
	}
	}
}

static void _spSkeletonBinary_addLinkedMesh (spSkeletonBinary* self, spMeshAttachment* mesh,
		const char* skin, int slotIndex, const char* parent, int inheritDeform) {
	_spLinkedMesh* linkedMesh;
	_spSkeletonBinary* internal = SUB_CAST(_spSkeletonBinary, self);

	if (internal->linkedMeshCount == internal->linkedMeshCapacity) {
		_spLinkedMesh* linkedMeshes;
		internal->linkedMeshCapacity *= 2;
		if (internal->linkedMeshCapacity < 8) internal->linkedMeshCapacity = 8;
		/* TODO Why not realloc? */
		linkedMeshes = MALLOC(_spLinkedMesh, internal->linkedMeshCapacity);
		memcpy(linkedMeshes, internal->linkedMeshes, sizeof(_spLinkedMesh) * internal->linkedMeshCount);
		FREE(internal->linkedMeshes);
		internal->linkedMeshes = linkedMeshes;
	}

	linkedMesh = internal->linkedMeshes + internal->linkedMeshCount++;
	linkedMesh->mesh = mesh;
	linkedMesh->skin = skin;
	linkedMesh->slotIndex = slotIndex;
	linkedMesh->parent = parent;
	linkedMesh->inheritDeform = inheritDeform;
}

_SP_ARRAY_DECLARE_TYPE(spTimelineArray, spTimeline*)
_SP_ARRAY_IMPLEMENT_TYPE(spTimelineArray, spTimeline*)

static spAnimation* _spSkeletonBinary_readAnimation (spSkeletonBinary* self, const char* name,
		_dataInput* input, spSkeletonData *skeletonData) {
	spTimelineArray* timelines = spTimelineArray_create(18);
	float duration = 0;
	int i, n, ii, nn, iii, nnn;
	int frameIndex;
	int drawOrderCount, eventCount;
	spAnimation* animation;

	/* Slot timelines. */
	for (i = 0, n = readVarint(input, 1); i < n; ++i) {
		int slotIndex = readVarint(input, 1);
		for (ii = 0, nn = readVarint(input, 1); ii < nn; ++ii) {
			unsigned char timelineType = readByte(input);
			int frameCount = readVarint(input, 1);
			switch (timelineType) {
			case SLOT_ATTACHMENT: {
				spAttachmentTimeline* timeline = spAttachmentTimeline_create(frameCount);
				timeline->slotIndex = slotIndex;
				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					const char* attachmentName = readStringRef(input, skeletonData);
					/* TODO Avoid copying of attachmentName inside */
					spAttachmentTimeline_setFrame(timeline, frameIndex, time, attachmentName);
				}
				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[frameCount - 1]);
				break;
			}
			case SLOT_COLOR: {
				spColorTimeline* timeline = spColorTimeline_create(frameCount);
				timeline->slotIndex = slotIndex;
				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					float r, g, b, a;
					readColor(input, &r, &g, &b, &a);
					spColorTimeline_setFrame(timeline, frameIndex, time, r, g, b, a);
					if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
				}
				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[(frameCount - 1) * COLOR_ENTRIES]);
				break;
			}
			case SLOT_TWO_COLOR: {
				spTwoColorTimeline* timeline = spTwoColorTimeline_create(frameCount);
				timeline->slotIndex = slotIndex;
				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					float r, g, b, a;
					float r2, g2, b2, a2;
					readColor(input, &r, &g, &b, &a);
					readColor(input, &a2, &r2, &g2, &b2);
					spTwoColorTimeline_setFrame(timeline, frameIndex, time, r, g, b, a, r2, g2, b2);
					if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
				}
				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[(frameCount - 1) * TWOCOLOR_ENTRIES]);
				break;
			}
			default: {
				for (iii = 0; iii < timelines->size; ++iii)
					spTimeline_dispose(timelines->items[iii]);
				spTimelineArray_dispose(timelines);
				_spSkeletonBinary_setError(self, "Invalid timeline type for a slot: ", skeletonData->slots[slotIndex]->name);
				return 0;
			}
			}
		}
	}

	/* Bone timelines. */
	for (i = 0, n = readVarint(input, 1); i < n; ++i) {
		int boneIndex = readVarint(input, 1);
		for (ii = 0, nn = readVarint(input, 1); ii < nn; ++ii) {
			unsigned char timelineType = readByte(input);
			int frameCount = readVarint(input, 1);
			switch (timelineType) {
			case BONE_ROTATE: {
				spRotateTimeline *timeline = spRotateTimeline_create(frameCount);
				timeline->boneIndex = boneIndex;
				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					float degrees = readFloat(input);
					spRotateTimeline_setFrame(timeline, frameIndex, time, degrees);
					if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
				}
				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[(frameCount - 1) * ROTATE_ENTRIES]);
				break;
			}
			case BONE_TRANSLATE:
			case BONE_SCALE:
			case BONE_SHEAR: {
				float timelineScale = 1;
				spTranslateTimeline *timeline = 0;
				switch (timelineType) {
					case BONE_SCALE:
						timeline = spScaleTimeline_create(frameCount);
						break;
					case BONE_SHEAR:
						timeline = spShearTimeline_create(frameCount);
						break;
					case BONE_TRANSLATE:
						timeline = spTranslateTimeline_create(frameCount);
						timelineScale = self->scale;
						break;
					default:
						break;
				}
				timeline->boneIndex = boneIndex;
				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					float x = readFloat(input) * timelineScale;
					float y = readFloat(input) * timelineScale;
					spTranslateTimeline_setFrame(timeline, frameIndex, time, x, y);
					if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
				}
				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[(frameCount - 1) * TRANSLATE_ENTRIES]);
				break;
			}
			default: {
				for (iii = 0; iii < timelines->size; ++iii)
					spTimeline_dispose(timelines->items[iii]);
				spTimelineArray_dispose(timelines);
				_spSkeletonBinary_setError(self, "Invalid timeline type for a bone: ", skeletonData->bones[boneIndex]->name);
				return 0;
			}
			}
		}
	}

	/* IK constraint timelines. */
	for (i = 0, n = readVarint(input, 1); i < n; ++i) {
		int index = readVarint(input, 1);
		int frameCount = readVarint(input, 1);
		spIkConstraintTimeline* timeline = spIkConstraintTimeline_create(frameCount);
		timeline->ikConstraintIndex = index;
		for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			float time = readFloat(input);
			float mix = readFloat(input);
			float softness = readFloat(input);
			signed char bendDirection = readSByte(input);
			int compress = readBoolean(input);
			int stretch = readBoolean(input);
			spIkConstraintTimeline_setFrame(timeline, frameIndex, time, mix, softness, bendDirection, compress, stretch);
			if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
		}
		spTimelineArray_add(timelines, (spTimeline*)timeline);
		duration = MAX(duration, timeline->frames[(frameCount - 1) * IKCONSTRAINT_ENTRIES]);
	}

	/* Transform constraint timelines. */
	for (i = 0, n = readVarint(input, 1); i < n; ++i) {
		int index = readVarint(input, 1);
		int frameCount = readVarint(input, 1);
		spTransformConstraintTimeline* timeline = spTransformConstraintTimeline_create(frameCount);
		timeline->transformConstraintIndex = index;
		for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			float time = readFloat(input);
			float rotateMix = readFloat(input);
			float translateMix = readFloat(input);
			float scaleMix = readFloat(input);
			float shearMix = readFloat(input);
			spTransformConstraintTimeline_setFrame(timeline, frameIndex, time, rotateMix, translateMix,
				scaleMix, shearMix);
			if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
		}
		spTimelineArray_add(timelines, (spTimeline*)timeline);
		duration = MAX(duration, timeline->frames[(frameCount - 1) * TRANSFORMCONSTRAINT_ENTRIES]);
	}

	/* Path constraint timelines. */
	for (i = 0, n = readVarint(input, 1); i < n; ++i) {
		int index = readVarint(input, 1);
		spPathConstraintData* data = skeletonData->pathConstraints[index];
		for (ii = 0, nn = readVarint(input, 1); ii < nn; ++ii) {
			unsigned char timelineType = readByte(input);
			int frameCount = readVarint(input, 1);
			switch (timelineType) {
			case PATH_POSITION:
			case PATH_SPACING: {
				spPathConstraintPositionTimeline* timeline = 0;
				float timelineScale = 1;
				if (timelineType == PATH_SPACING) {
					timeline = (spPathConstraintPositionTimeline*)spPathConstraintSpacingTimeline_create(frameCount);
					if (data->spacingMode == SP_SPACING_MODE_LENGTH || data->spacingMode == SP_SPACING_MODE_FIXED)
						timelineScale = self->scale;
				} else {
					timeline = spPathConstraintPositionTimeline_create(frameCount);
					if (data->positionMode == SP_POSITION_MODE_FIXED)
						timelineScale = self->scale;
				}
				timeline->pathConstraintIndex = index;
				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					float value = readFloat(input) * timelineScale;
					spPathConstraintPositionTimeline_setFrame(timeline, frameIndex, time, value);
					if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
				}
				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[(frameCount - 1) * PATHCONSTRAINTPOSITION_ENTRIES]);
				break;
			}
			case PATH_MIX: {
				spPathConstraintMixTimeline* timeline = spPathConstraintMixTimeline_create(frameCount);
				timeline->pathConstraintIndex = index;
				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					float rotateMix = readFloat(input);
					float translateMix = readFloat(input);
					spPathConstraintMixTimeline_setFrame(timeline, frameIndex, time, rotateMix, translateMix);
					if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
				}
				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[(frameCount - 1) * PATHCONSTRAINTMIX_ENTRIES]);
			}
			}
		}
	}

	/* Deform timelines. */
	for (i = 0, n = readVarint(input, 1); i < n; ++i) {
		spSkin* skin = skeletonData->skins[readVarint(input, 1)];
		for (ii = 0, nn = readVarint(input, 1); ii < nn; ++ii) {
			int slotIndex = readVarint(input, 1);
			for (iii = 0, nnn = readVarint(input, 1); iii < nnn; ++iii) {
				float* tempDeform;
				spDeformTimeline *timeline;
				int weighted, deformLength;
				const char* attachmentName = readStringRef(input, skeletonData);
				int frameCount;

				spVertexAttachment* attachment = SUB_CAST(spVertexAttachment,
					spSkin_getAttachment(skin, slotIndex, attachmentName));
				if (!attachment) {
					for (i = 0; i < timelines->size; ++i)
						spTimeline_dispose(timelines->items[i]);
					spTimelineArray_dispose(timelines);
					_spSkeletonBinary_setError(self, "Attachment not found: ", attachmentName);
					return 0;
				}

				weighted = attachment->bones != 0;
				deformLength = weighted ? attachment->verticesCount / 3 * 2 : attachment->verticesCount;
				tempDeform = MALLOC(float, deformLength);

				frameCount = readVarint(input, 1);
				timeline = spDeformTimeline_create(frameCount, deformLength);
				timeline->slotIndex = slotIndex;
				timeline->attachment = SUPER(attachment);

				for (frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					float time = readFloat(input);
					float* deform;
					int end = readVarint(input, 1);
					if (!end) {
						if (weighted) {
							deform = tempDeform;
							memset(deform, 0, sizeof(float) * deformLength);
						} else
							deform = attachment->vertices;
					} else {
						int v, start = readVarint(input, 1);
						deform = tempDeform;
						memset(deform, 0, sizeof(float) * start);
						end += start;
						if (self->scale == 1) {
							for (v = start; v < end; ++v)
								deform[v] = readFloat(input);
						} else {
							for (v = start; v < end; ++v)
								deform[v] = readFloat(input) * self->scale;
						}
						memset(deform + v, 0, sizeof(float) * (deformLength - v));
						if (!weighted) {
							float* vertices = attachment->vertices;
							for (v = 0; v < deformLength; ++v)
								deform[v] += vertices[v];
						}
					}
					spDeformTimeline_setFrame(timeline, frameIndex, time, deform);
					if (frameIndex < frameCount - 1) readCurve(input, SUPER(timeline), frameIndex);
				}
				FREE(tempDeform);

				spTimelineArray_add(timelines, (spTimeline*)timeline);
				duration = MAX(duration, timeline->frames[frameCount - 1]);
			}
		}
	}

	/* Draw order timeline. */
	drawOrderCount = readVarint(input, 1);
	if (drawOrderCount) {
		spDrawOrderTimeline* timeline = spDrawOrderTimeline_create(drawOrderCount, skeletonData->slotsCount);
		for (i = 0; i < drawOrderCount; ++i) {
			float time = readFloat(input);
			int offsetCount = readVarint(input, 1);
			int* drawOrder = MALLOC(int, skeletonData->slotsCount);
			int* unchanged = MALLOC(int, skeletonData->slotsCount - offsetCount);
			int originalIndex = 0, unchangedIndex = 0;
			memset(drawOrder, -1, sizeof(int) * skeletonData->slotsCount);
			for (ii = 0; ii < offsetCount; ++ii) {
				int slotIndex = readVarint(input, 1);
				/* Collect unchanged items. */
				while (originalIndex != slotIndex)
					unchanged[unchangedIndex++] = originalIndex++;
				/* Set changed items. */
				drawOrder[originalIndex + readVarint(input, 1)] = originalIndex;
				++originalIndex;
			}
			/* Collect remaining unchanged items. */
			while (originalIndex < skeletonData->slotsCount)
				unchanged[unchangedIndex++] = originalIndex++;
			/* Fill in unchanged items. */
			for (ii = skeletonData->slotsCount - 1; ii >= 0; ii--)
				if (drawOrder[ii] == -1) drawOrder[ii] = unchanged[--unchangedIndex];
			FREE(unchanged);
			/* TODO Avoid copying of drawOrder inside */
			spDrawOrderTimeline_setFrame(timeline, i, time, drawOrder);
			FREE(drawOrder);
		}
		spTimelineArray_add(timelines, (spTimeline*)timeline);
		duration = MAX(duration, timeline->frames[drawOrderCount - 1]);
	}

	/* Event timeline. */
	eventCount = readVarint(input, 1);
	if (eventCount) {
		spEventTimeline* timeline = spEventTimeline_create(eventCount);
		for (i = 0; i < eventCount; ++i) {
			float time = readFloat(input);
			spEventData* eventData = skeletonData->events[readVarint(input, 1)];
			spEvent* event = spEvent_create(time, eventData);
			event->intValue = readVarint(input, 0);
			event->floatValue = readFloat(input);
			if (readBoolean(input))
				event->stringValue = readString(input);
			else
				MALLOC_STR(event->stringValue, eventData->stringValue);
			if (eventData->audioPath) {
				event->volume = readFloat(input);
				event->balance = readFloat(input);
			}
			spEventTimeline_setFrame(timeline, i, event);
		}
		spTimelineArray_add(timelines, (spTimeline*)timeline);
		duration = MAX(duration, timeline->frames[eventCount - 1]);
	}

	animation = spAnimation_create(name, 0);
	FREE(animation->timelines);
	animation->duration = duration;
	animation->timelinesCount = timelines->size;
	animation->timelines = timelines->items;
	FREE(timelines);
	return animation;
}

static float* _readFloatArray(_dataInput *input, int n, float scale) {
	float* array = MALLOC(float, n);
	int i;
	if (scale == 1)
		for (i = 0; i < n; ++i)
			array[i] = readFloat(input);
	else
		for (i = 0; i < n; ++i)
			array[i] = readFloat(input) * scale;
	return array;
}

static short* _readShortArray(_dataInput *input, int *length) {
	int n = readVarint(input, 1);
	short* array = MALLOC(short, n);
	int i;
	*length = n;
	for (i = 0; i < n; ++i) {
		array[i] = readByte(input) << 8;
		array[i] |= readByte(input);
	}
	return array;
}

static void _readVertices(spSkeletonBinary* self, _dataInput* input, spVertexAttachment* attachment,
		int vertexCount) {
	int i, ii;
	int verticesLength = vertexCount << 1;
	spFloatArray* weights = spFloatArray_create(8);
	spIntArray* bones = spIntArray_create(8);

	attachment->worldVerticesLength = verticesLength;

	if (!readBoolean(input)) {
		attachment->verticesCount = verticesLength;
		attachment->vertices = _readFloatArray(input, verticesLength, self->scale);
		attachment->bonesCount = 0;
		attachment->bones = 0;
		spFloatArray_dispose(weights);
		spIntArray_dispose(bones);
		return;
	}

	spFloatArray_ensureCapacity(weights, verticesLength * 3 * 3);
	spIntArray_ensureCapacity(bones, verticesLength * 3);

	for (i = 0; i < vertexCount; ++i) {
		int boneCount = readVarint(input, 1);
		spIntArray_add(bones, boneCount);
		for (ii = 0; ii < boneCount; ++ii) {
			spIntArray_add(bones, readVarint(input, 1));
			spFloatArray_add(weights, readFloat(input) * self->scale);
			spFloatArray_add(weights, readFloat(input) * self->scale);
			spFloatArray_add(weights, readFloat(input));
		}
	}

	attachment->verticesCount = weights->size;
	attachment->vertices = weights->items;
	FREE(weights);

	attachment->bonesCount = bones->size;
	attachment->bones = bones->items;
	FREE(bones);
}

spAttachment* spSkeletonBinary_readAttachment(spSkeletonBinary* self, _dataInput* input,
		spSkin* skin, int slotIndex, const char* attachmentName, spSkeletonData* skeletonData, int/*bool*/ nonessential) {
	int i;
	spAttachmentType type;
	const char* name = readStringRef(input, skeletonData);
	if (!name) name = attachmentName;

	type = (spAttachmentType)readByte(input);

	switch (type) {
	case SP_ATTACHMENT_REGION: {
		const char* path = readStringRef(input, skeletonData);
		spAttachment* attachment;
		spRegionAttachment* region;
		if (!path) MALLOC_STR(path, name);
		else {
			const char* tmp = 0;
			MALLOC_STR(tmp, path);
			path = tmp;
		}
		attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, name, path);
		region = SUB_CAST(spRegionAttachment, attachment);
		region->path = path;
		region->rotation = readFloat(input);
		region->x = readFloat(input) * self->scale;
		region->y = readFloat(input) * self->scale;
		region->scaleX = readFloat(input);
		region->scaleY = readFloat(input);
		region->width = readFloat(input) * self->scale;
		region->height = readFloat(input) * self->scale;
		readColor(input, &region->color.r, &region->color.g, &region->color.b, &region->color.a);
		spRegionAttachment_updateOffset(region);
		spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
		return attachment;
	}
	case SP_ATTACHMENT_BOUNDING_BOX: {
		int vertexCount = readVarint(input, 1);
		spAttachment* attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, name, 0);
		_readVertices(self, input, SUB_CAST(spVertexAttachment, attachment), vertexCount);
		if (nonessential) readInt(input); /* Skip color. */
		spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
		return attachment;
	}
	case SP_ATTACHMENT_MESH: {
		int vertexCount;
		spAttachment* attachment;
		spMeshAttachment* mesh;
		const char* path = readStringRef(input, skeletonData);
		if (!path) MALLOC_STR(path, name);
		else {
			const char* tmp = 0;
			MALLOC_STR(tmp, path);
			path = tmp;
		}
		attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, name, path);
		mesh = SUB_CAST(spMeshAttachment, attachment);
		mesh->path = path;
		readColor(input, &mesh->color.r, &mesh->color.g, &mesh->color.b, &mesh->color.a);
		vertexCount = readVarint(input, 1);
		mesh->regionUVs = _readFloatArray(input, vertexCount << 1, 1);
		mesh->triangles = (unsigned short*)_readShortArray(input, &mesh->trianglesCount);
		_readVertices(self, input, SUPER(mesh), vertexCount);
		spMeshAttachment_updateUVs(mesh);
		mesh->hullLength = readVarint(input, 1) << 1;
		if (nonessential) {
			mesh->edges = (int*)_readShortArray(input, &mesh->edgesCount);
			mesh->width = readFloat(input) * self->scale;
			mesh->height = readFloat(input) * self->scale;
		} else {
			mesh->edges = 0;
			mesh->width = 0;
			mesh->height = 0;
		}
		spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
		return attachment;
	}
	case SP_ATTACHMENT_LINKED_MESH: {
		const char* skinName;
		const char* parent;
		spAttachment* attachment;
		spMeshAttachment* mesh;
		int inheritDeform;
		const char* path = readStringRef(input, skeletonData);
		if (!path) MALLOC_STR(path, name);
		else {
			const char* tmp = 0;
			MALLOC_STR(tmp, path);
			path = tmp;
		}
		attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, name, path);
		mesh = SUB_CAST(spMeshAttachment, attachment);
		mesh->path = path;
		readColor(input, &mesh->color.r, &mesh->color.g, &mesh->color.b, &mesh->color.a);
		skinName = readStringRef(input, skeletonData);
		parent = readStringRef(input, skeletonData);
		inheritDeform = readBoolean(input);
		if (nonessential) {
			mesh->width = readFloat(input) * self->scale;
			mesh->height = readFloat(input) * self->scale;
		}
		_spSkeletonBinary_addLinkedMesh(self, mesh, skinName, slotIndex, parent, inheritDeform);
		return attachment;
	}
	case SP_ATTACHMENT_PATH: {
		spAttachment* attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, name, 0);
		spPathAttachment* path = SUB_CAST(spPathAttachment, attachment);
		int vertexCount = 0;
		path->closed = readBoolean(input);
		path->constantSpeed = readBoolean(input);
		vertexCount = readVarint(input, 1);
		_readVertices(self, input, SUPER(path), vertexCount);
		path->lengthsLength = vertexCount / 3;
		path->lengths = MALLOC(float, path->lengthsLength);
		for (i = 0; i < path->lengthsLength; ++i) {
			path->lengths[i] = readFloat(input) * self->scale;
		}
		if (nonessential) readInt(input); /* Skip color. */
		spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
		return attachment;
	}
	case SP_ATTACHMENT_POINT: {
		spAttachment* attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, name, 0);
		spPointAttachment* point = SUB_CAST(spPointAttachment, attachment);
		point->rotation = readFloat(input);
		point->x = readFloat(input) * self->scale;
		point->y = readFloat(input) * self->scale;

		if (nonessential) {
			readColor(input, &point->color.r, &point->color.g, &point->color.b, &point->color.a);
		}
		spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
		return attachment;
	}
	case SP_ATTACHMENT_CLIPPING: {
		int endSlotIndex = readVarint(input, 1);
		int vertexCount = readVarint(input, 1);
		spAttachment* attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, name, 0);
		spClippingAttachment* clip = SUB_CAST(spClippingAttachment, attachment);
		_readVertices(self, input, SUB_CAST(spVertexAttachment, attachment), vertexCount);
		if (nonessential) readInt(input); /* Skip color. */
		clip->endSlot = skeletonData->slots[endSlotIndex];
		spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
		return attachment;
	}
	}

	return 0;
}

spSkin* spSkeletonBinary_readSkin(spSkeletonBinary* self, _dataInput* input, int/*bool*/ defaultSkin,
		spSkeletonData* skeletonData, int/*bool*/ nonessential) {
	spSkin *skin;
	int i, n, ii, nn, slotCount;

	if (defaultSkin) {
		slotCount = readVarint(input, 1);
		if (slotCount == 0) return 0;
		skin = spSkin_create("default");
	} else {
		skin = spSkin_create(readStringRef(input, skeletonData));
		for (i = 0, n = readVarint(input, 1); i < n; i++)
			spBoneDataArray_add(skin->bones, skeletonData->bones[readVarint(input, 1)]);

		for (i = 0, n = readVarint(input, 1); i < n; i++)
			spIkConstraintDataArray_add(skin->ikConstraints, skeletonData->ikConstraints[readVarint(input, 1)]);

		for (i = 0, n = readVarint(input, 1); i < n; i++)
			spTransformConstraintDataArray_add(skin->transformConstraints, skeletonData->transformConstraints[readVarint(input, 1)]);

		for (i = 0, n = readVarint(input, 1); i < n; i++)
			spPathConstraintDataArray_add(skin->pathConstraints, skeletonData->pathConstraints[readVarint(input, 1)]);

		slotCount = readVarint(input, 1);
	}

	for (i = 0; i < slotCount; ++i) {
		int slotIndex = readVarint(input, 1);
		for (ii = 0, nn = readVarint(input, 1); ii < nn; ++ii) {
			const char* name = readStringRef(input, skeletonData);
			spAttachment* attachment = spSkeletonBinary_readAttachment(self, input, skin, slotIndex, name, skeletonData, nonessential);
			if (attachment) spSkin_setAttachment(skin, slotIndex, name, attachment);
		}
	}
	return skin;
}

spSkeletonData* spSkeletonBinary_readSkeletonDataFile (spSkeletonBinary* self, const char* path) {
	int length;
	spSkeletonData* skeletonData;
	const char* binary = _spUtil_readFile(path, &length);
	if (length == 0 || !binary) {
		_spSkeletonBinary_setError(self, "Unable to read skeleton file: ", path);
		return 0;
	}
	skeletonData = spSkeletonBinary_readSkeletonData(self, (unsigned char*)binary, length);
	FREE(binary);
	return skeletonData;
}

spSkeletonData* spSkeletonBinary_readSkeletonData (spSkeletonBinary* self, const unsigned char* binary,
		const int length) {
	int i, n, ii, nonessential;
	spSkeletonData* skeletonData;
	_spSkeletonBinary* internal = SUB_CAST(_spSkeletonBinary, self);

	_dataInput* input = NEW(_dataInput);
	input->cursor = binary;
	input->end = binary + length;

	FREE(self->error);
	CONST_CAST(char*, self->error) = 0;
	internal->linkedMeshCount = 0;

	skeletonData = spSkeletonData_create();

	skeletonData->hash = readString(input);
	if (!strlen(skeletonData->hash)) {
		FREE(skeletonData->hash);
		skeletonData->hash = 0;
	}

	skeletonData->version = readString(input);
	if (!strlen(skeletonData->version)) {
		FREE(skeletonData->version);
		skeletonData->version = 0;
	}
    if (strcmp(skeletonData->version, "3.8.75") == 0) {
        FREE(input);
        spSkeletonData_dispose(skeletonData);
        _spSkeletonBinary_setError(self, "Unsupported skeleton data, please export with a newer version of Spine.", "");
        return 0;
    }

	skeletonData->x = readFloat(input);
	skeletonData->y = readFloat(input);
	skeletonData->width = readFloat(input);
	skeletonData->height = readFloat(input);

	nonessential = readBoolean(input);

	if (nonessential) {
		/* Skip images path & fps */
		readFloat(input);
		FREE(readString(input));
		FREE(readString(input));
	}

	skeletonData->stringsCount = n = readVarint(input, 1);
	skeletonData->strings = MALLOC(char*, skeletonData->stringsCount);
	for (i = 0; i < n; i++) {
		skeletonData->strings[i] = readString(input);
	}

	/* Bones. */
	skeletonData->bonesCount = readVarint(input, 1);
	skeletonData->bones = MALLOC(spBoneData*, skeletonData->bonesCount);
	for (i = 0; i < skeletonData->bonesCount; ++i) {
		spBoneData* data;
		int mode;
		const char* name = readString(input);
		spBoneData* parent = i == 0 ? 0 : skeletonData->bones[readVarint(input, 1)];
		/* TODO Avoid copying of name */
		data = spBoneData_create(i, name, parent);
		FREE(name);
		data->rotation = readFloat(input);
		data->x = readFloat(input) * self->scale;
		data->y = readFloat(input) * self->scale;
		data->scaleX = readFloat(input);
		data->scaleY = readFloat(input);
		data->shearX = readFloat(input);
		data->shearY = readFloat(input);
		data->length = readFloat(input) * self->scale;
		mode = readVarint(input, 1);
		switch (mode) {
		case 0: data->transformMode = SP_TRANSFORMMODE_NORMAL; break;
		case 1: data->transformMode = SP_TRANSFORMMODE_ONLYTRANSLATION; break;
		case 2: data->transformMode = SP_TRANSFORMMODE_NOROTATIONORREFLECTION; break;
		case 3: data->transformMode = SP_TRANSFORMMODE_NOSCALE; break;
		case 4: data->transformMode = SP_TRANSFORMMODE_NOSCALEORREFLECTION; break;
		}
		data->skinRequired = readBoolean(input);
		if (nonessential) readInt(input); /* Skip bone color. */
		skeletonData->bones[i] = data;
	}

	/* Slots. */
	skeletonData->slotsCount = readVarint(input, 1);
	skeletonData->slots = MALLOC(spSlotData*, skeletonData->slotsCount);
	for (i = 0; i < skeletonData->slotsCount; ++i) {
		int r, g, b, a;
		const char* attachmentName;
		const char* slotName = readString(input);
		spBoneData* boneData = skeletonData->bones[readVarint(input, 1)];
		/* TODO Avoid copying of slotName */
		spSlotData* slotData = spSlotData_create(i, slotName, boneData);
		FREE(slotName);
		readColor(input, &slotData->color.r, &slotData->color.g, &slotData->color.b, &slotData->color.a);
		a = readByte(input);
		r = readByte(input);
		g = readByte(input);
		b = readByte(input);
		if (!(r == 0xff && g == 0xff && b == 0xff && a == 0xff)) {
			slotData->darkColor = spColor_create();
			spColor_setFromFloats(slotData->darkColor, r / 255.0f, g / 255.0f, b / 255.0f, 1);
		}
		attachmentName = readStringRef(input, skeletonData);
		if (attachmentName) MALLOC_STR(slotData->attachmentName, attachmentName);
		else slotData->attachmentName = 0;
		slotData->blendMode = (spBlendMode)readVarint(input, 1);
		skeletonData->slots[i] = slotData;
	}

	/* IK constraints. */
	skeletonData->ikConstraintsCount = readVarint(input, 1);
	skeletonData->ikConstraints = MALLOC(spIkConstraintData*, skeletonData->ikConstraintsCount);
	for (i = 0; i < skeletonData->ikConstraintsCount; ++i) {
		const char* name = readString(input);
		/* TODO Avoid copying of name */
		spIkConstraintData* data = spIkConstraintData_create(name);
		data->order = readVarint(input, 1);
		data->skinRequired = readBoolean(input);
		FREE(name);
		data->bonesCount = readVarint(input, 1);
		data->bones = MALLOC(spBoneData*, data->bonesCount);
		for (ii = 0; ii < data->bonesCount; ++ii)
			data->bones[ii] = skeletonData->bones[readVarint(input, 1)];
		data->target = skeletonData->bones[readVarint(input, 1)];
		data->mix = readFloat(input);
		data->softness = readFloat(input);
		data->bendDirection = readSByte(input);
		data->compress = readBoolean(input);
		data->stretch = readBoolean(input);
		data->uniform = readBoolean(input);
		skeletonData->ikConstraints[i] = data;
	}

	/* Transform constraints. */
	skeletonData->transformConstraintsCount = readVarint(input, 1);
	skeletonData->transformConstraints = MALLOC(
			spTransformConstraintData*, skeletonData->transformConstraintsCount);
	for (i = 0; i < skeletonData->transformConstraintsCount; ++i) {
		const char* name = readString(input);
		/* TODO Avoid copying of name */
		spTransformConstraintData* data = spTransformConstraintData_create(name);
		data->order = readVarint(input, 1);
		data->skinRequired = readBoolean(input);
		FREE(name);
		data->bonesCount = readVarint(input, 1);
		CONST_CAST(spBoneData**, data->bones) = MALLOC(spBoneData*, data->bonesCount);
		for (ii = 0; ii < data->bonesCount; ++ii)
			data->bones[ii] = skeletonData->bones[readVarint(input, 1)];
		data->target = skeletonData->bones[readVarint(input, 1)];
		data->local = readBoolean(input);
		data->relative = readBoolean(input);
		data->offsetRotation = readFloat(input);
		data->offsetX = readFloat(input) * self->scale;
		data->offsetY = readFloat(input) * self->scale;
		data->offsetScaleX = readFloat(input);
		data->offsetScaleY = readFloat(input);
		data->offsetShearY = readFloat(input);
		data->rotateMix = readFloat(input);
		data->translateMix = readFloat(input);
		data->scaleMix = readFloat(input);
		data->shearMix = readFloat(input);
		skeletonData->transformConstraints[i] = data;
	}

	/* Path constraints */
	skeletonData->pathConstraintsCount = readVarint(input, 1);
	skeletonData->pathConstraints = MALLOC(spPathConstraintData*, skeletonData->pathConstraintsCount);
	for (i = 0; i < skeletonData->pathConstraintsCount; ++i) {
		const char* name = readString(input);
		/* TODO Avoid copying of name */
		spPathConstraintData* data = spPathConstraintData_create(name);
		data->order = readVarint(input, 1);
		data->skinRequired = readBoolean(input);
		FREE(name);
		data->bonesCount = readVarint(input, 1);
		CONST_CAST(spBoneData**, data->bones) = MALLOC(spBoneData*, data->bonesCount);
		for (ii = 0; ii < data->bonesCount; ++ii)
			data->bones[ii] = skeletonData->bones[readVarint(input, 1)];
		data->target = skeletonData->slots[readVarint(input, 1)];
		data->positionMode = (spPositionMode)readVarint(input, 1);
		data->spacingMode = (spSpacingMode)readVarint(input, 1);
		data->rotateMode = (spRotateMode)readVarint(input, 1);
		data->offsetRotation = readFloat(input);
		data->position = readFloat(input);
		if (data->positionMode == SP_POSITION_MODE_FIXED) data->position *= self->scale;
		data->spacing = readFloat(input);
		if (data->spacingMode == SP_SPACING_MODE_LENGTH || data->spacingMode == SP_SPACING_MODE_FIXED) data->spacing *= self->scale;
		data->rotateMix = readFloat(input);
		data->translateMix = readFloat(input);
		skeletonData->pathConstraints[i] = data;
	}

	/* Default skin. */
	skeletonData->defaultSkin = spSkeletonBinary_readSkin(self, input, -1, skeletonData, nonessential);
	skeletonData->skinsCount = readVarint(input, 1);

	if (skeletonData->defaultSkin)
		++skeletonData->skinsCount;

	skeletonData->skins = MALLOC(spSkin*, skeletonData->skinsCount);

	if (skeletonData->defaultSkin)
		skeletonData->skins[0] = skeletonData->defaultSkin;

	/* Skins. */
	for (i = skeletonData->defaultSkin ? 1 : 0; i < skeletonData->skinsCount; ++i) {
		skeletonData->skins[i] = spSkeletonBinary_readSkin(self, input, 0, skeletonData, nonessential);
	}

	/* Linked meshes. */
	for (i = 0; i < internal->linkedMeshCount; ++i) {
		_spLinkedMesh* linkedMesh = internal->linkedMeshes + i;
		spSkin* skin = !linkedMesh->skin ? skeletonData->defaultSkin : spSkeletonData_findSkin(skeletonData, linkedMesh->skin);
		spAttachment* parent;
		if (!skin) {
			FREE(input);
			spSkeletonData_dispose(skeletonData);
			_spSkeletonBinary_setError(self, "Skin not found: ", linkedMesh->skin);
			return 0;
		}
		parent = spSkin_getAttachment(skin, linkedMesh->slotIndex, linkedMesh->parent);
		if (!parent) {
			FREE(input);
			spSkeletonData_dispose(skeletonData);
			_spSkeletonBinary_setError(self, "Parent mesh not found: ", linkedMesh->parent);
			return 0;
		}
		linkedMesh->mesh->super.deformAttachment = linkedMesh->inheritDeform ? SUB_CAST(spVertexAttachment, parent) : SUB_CAST(spVertexAttachment, linkedMesh->mesh);
		spMeshAttachment_setParentMesh(linkedMesh->mesh, SUB_CAST(spMeshAttachment, parent));
		spMeshAttachment_updateUVs(linkedMesh->mesh);
		spAttachmentLoader_configureAttachment(self->attachmentLoader, SUPER(SUPER(linkedMesh->mesh)));
	}

	/* Events. */
	skeletonData->eventsCount = readVarint(input, 1);
	skeletonData->events = MALLOC(spEventData*, skeletonData->eventsCount);
	for (i = 0; i < skeletonData->eventsCount; ++i) {
		const char* name = readStringRef(input, skeletonData);
		spEventData* eventData = spEventData_create(name);
		eventData->intValue = readVarint(input, 0);
		eventData->floatValue = readFloat(input);
		eventData->stringValue = readString(input);
		eventData->audioPath = readString(input);
		if (eventData->audioPath) {
			eventData->volume = readFloat(input);
			eventData->balance = readFloat(input);
		}
		skeletonData->events[i] = eventData;
	}

	/* Animations. */
	skeletonData->animationsCount = readVarint(input, 1);
	skeletonData->animations = MALLOC(spAnimation*, skeletonData->animationsCount);
	for (i = 0; i < skeletonData->animationsCount; ++i) {
		const char* name = readString(input);
		spAnimation* animation = _spSkeletonBinary_readAnimation(self, name, input, skeletonData);
		FREE(name);
		if (!animation) {
			FREE(input);
			spSkeletonData_dispose(skeletonData);
			return 0;
		}
		skeletonData->animations[i] = animation;
	}

	FREE(input);
	return skeletonData;
}
