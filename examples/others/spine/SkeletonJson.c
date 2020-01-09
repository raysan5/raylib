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

#include <spine/SkeletonJson.h>
#include <stdio.h>
#include "Json.h"
#include <spine/extension.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/Array.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define strdup _strdup
#endif

typedef struct {
	const char* parent;
	const char* skin;
	int slotIndex;
	spMeshAttachment* mesh;
	int inheritDeform;
} _spLinkedMesh;

typedef struct {
	spSkeletonJson super;
	int ownsLoader;

	int linkedMeshCount;
	int linkedMeshCapacity;
	_spLinkedMesh* linkedMeshes;
} _spSkeletonJson;

spSkeletonJson* spSkeletonJson_createWithLoader (spAttachmentLoader* attachmentLoader) {
	spSkeletonJson* self = SUPER(NEW(_spSkeletonJson));
	self->scale = 1;
	self->attachmentLoader = attachmentLoader;
	return self;
}

spSkeletonJson* spSkeletonJson_create (spAtlas* atlas) {
	spAtlasAttachmentLoader* attachmentLoader = spAtlasAttachmentLoader_create(atlas);
	spSkeletonJson* self = spSkeletonJson_createWithLoader(SUPER(attachmentLoader));
	SUB_CAST(_spSkeletonJson, self)->ownsLoader = 1;
	return self;
}

void spSkeletonJson_dispose (spSkeletonJson* self) {
	_spSkeletonJson* internal = SUB_CAST(_spSkeletonJson, self);
	if (internal->ownsLoader) spAttachmentLoader_dispose(self->attachmentLoader);
	FREE(internal->linkedMeshes);
	FREE(self->error);
	FREE(self);
}

void _spSkeletonJson_setError (spSkeletonJson* self, Json* root, const char* value1, const char* value2) {
	char message[256];
	int length;
	FREE(self->error);
	strcpy(message, value1);
	length = (int)strlen(value1);
	if (value2) strncat(message + length, value2, 255 - length);
	MALLOC_STR(self->error, message);
	if (root) Json_dispose(root);
}

static float toColor (const char* value, int index) {
	char digits[3];
	char *error;
	int color;

	if ((size_t)index >= strlen(value) / 2) return -1;
	value += index * 2;

	digits[0] = *value;
	digits[1] = *(value + 1);
	digits[2] = '\0';
	color = (int)strtoul(digits, &error, 16);
	if (*error != 0) return -1;
	return color / (float)255;
}

static void readCurve (Json* frame, spCurveTimeline* timeline, int frameIndex) {
	Json* curve = Json_getItem(frame, "curve");
	if (!curve) return;
	if (curve->type == Json_String && strcmp(curve->valueString, "stepped") == 0)
		spCurveTimeline_setStepped(timeline, frameIndex);
	else {
		float c1 = Json_getFloat(frame, "curve", 0);
		float c2 = Json_getFloat(frame, "c2", 0);
		float c3 = Json_getFloat(frame, "c3", 1);
		float c4 = Json_getFloat(frame, "c4", 1);
		spCurveTimeline_setCurve(timeline, frameIndex, c1, c2, c3, c4);
	}
}

static void _spSkeletonJson_addLinkedMesh (spSkeletonJson* self, spMeshAttachment* mesh, const char* skin, int slotIndex,
	const char* parent, int inheritDeform
) {
	_spLinkedMesh* linkedMesh;
	_spSkeletonJson* internal = SUB_CAST(_spSkeletonJson, self);

	if (internal->linkedMeshCount == internal->linkedMeshCapacity) {
		_spLinkedMesh* linkedMeshes;
		internal->linkedMeshCapacity *= 2;
		if (internal->linkedMeshCapacity < 8) internal->linkedMeshCapacity = 8;
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

static spAnimation* _spSkeletonJson_readAnimation (spSkeletonJson* self, Json* root, spSkeletonData *skeletonData) {
	int frameIndex;
	spAnimation* animation;
	Json* valueMap;
	int timelinesCount = 0;

	Json* bones = Json_getItem(root, "bones");
	Json* slots = Json_getItem(root, "slots");
	Json* ik = Json_getItem(root, "ik");
	Json* transform = Json_getItem(root, "transform");
	Json* paths = Json_getItem(root, "paths");
	Json* deformJson = Json_getItem(root, "deform");
	Json* drawOrderJson = Json_getItem(root, "drawOrder");
	Json* events = Json_getItem(root, "events");
	Json *boneMap, *slotMap, *constraintMap;
	if (!drawOrderJson) drawOrderJson = Json_getItem(root, "draworder");

	for (boneMap = bones ? bones->child : 0; boneMap; boneMap = boneMap->next)
		timelinesCount += boneMap->size;
	for (slotMap = slots ? slots->child : 0; slotMap; slotMap = slotMap->next)
		timelinesCount += slotMap->size;
	timelinesCount += ik ? ik->size : 0;
	timelinesCount += transform ? transform->size : 0;
	for (constraintMap = paths ? paths->child : 0; constraintMap; constraintMap = constraintMap->next)
		timelinesCount += constraintMap->size;
	for (constraintMap = deformJson ? deformJson->child : 0; constraintMap; constraintMap = constraintMap->next)
		for (slotMap = constraintMap->child; slotMap; slotMap = slotMap->next)
			timelinesCount += slotMap->size;
	if (drawOrderJson) ++timelinesCount;
	if (events) ++timelinesCount;

	animation = spAnimation_create(root->name, timelinesCount);
	animation->timelinesCount = 0;

	/* Slot timelines. */
	for (slotMap = slots ? slots->child : 0; slotMap; slotMap = slotMap->next) {
		Json *timelineMap;

		int slotIndex = spSkeletonData_findSlotIndex(skeletonData, slotMap->name);
		if (slotIndex == -1) {
			spAnimation_dispose(animation);
			_spSkeletonJson_setError(self, root, "Slot not found: ", slotMap->name);
			return 0;
		}

		for (timelineMap = slotMap->child; timelineMap; timelineMap = timelineMap->next) {
			if (strcmp(timelineMap->name, "attachment") == 0) {
				spAttachmentTimeline *timeline = spAttachmentTimeline_create(timelineMap->size);
				timeline->slotIndex = slotIndex;

				for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
					Json* name = Json_getItem(valueMap, "name");
					spAttachmentTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0),
						name->type == Json_NULL ? 0 : name->valueString);
				}
				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
				animation->duration = MAX(animation->duration, timeline->frames[timelineMap->size - 1]);

			} else if (strcmp(timelineMap->name, "color") == 0) {
				spColorTimeline *timeline = spColorTimeline_create(timelineMap->size);
				timeline->slotIndex = slotIndex;

				for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
					const char* s = Json_getString(valueMap, "color", 0);
					spColorTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), toColor(s, 0), toColor(s, 1),
						toColor(s, 2), toColor(s, 3));
					readCurve(valueMap, SUPER(timeline), frameIndex);
				}
				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
				animation->duration = MAX(animation->duration, timeline->frames[(timelineMap->size - 1) * COLOR_ENTRIES]);

			} else if (strcmp(timelineMap->name, "twoColor") == 0) {
				spTwoColorTimeline *timeline = spTwoColorTimeline_create(timelineMap->size);
				timeline->slotIndex = slotIndex;

				for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
					const char* s = Json_getString(valueMap, "light", 0);
					const char* ds = Json_getString(valueMap, "dark", 0);
					spTwoColorTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), toColor(s, 0), toColor(s, 1), toColor(s, 2),
						toColor(s, 3), toColor(ds, 0), toColor(ds, 1), toColor(ds, 2));
					readCurve(valueMap, SUPER(timeline), frameIndex);
				}
				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
				animation->duration = MAX(animation->duration, timeline->frames[(timelineMap->size - 1) * TWOCOLOR_ENTRIES]);

			} else {
				spAnimation_dispose(animation);
				_spSkeletonJson_setError(self, 0, "Invalid timeline type for a slot: ", timelineMap->name);
				return 0;
			}
		}
	}

	/* Bone timelines. */
	for (boneMap = bones ? bones->child : 0; boneMap; boneMap = boneMap->next) {
		Json *timelineMap;

		int boneIndex = spSkeletonData_findBoneIndex(skeletonData, boneMap->name);
		if (boneIndex == -1) {
			spAnimation_dispose(animation);
			_spSkeletonJson_setError(self, root, "Bone not found: ", boneMap->name);
			return 0;
		}

		for (timelineMap = boneMap->child; timelineMap; timelineMap = timelineMap->next) {
			if (strcmp(timelineMap->name, "rotate") == 0) {
				spRotateTimeline *timeline = spRotateTimeline_create(timelineMap->size);
				timeline->boneIndex = boneIndex;

				for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
					spRotateTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), Json_getFloat(valueMap, "angle", 0));
					readCurve(valueMap, SUPER(timeline), frameIndex);
				}
				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
				animation->duration = MAX(animation->duration, timeline->frames[(timelineMap->size - 1) * ROTATE_ENTRIES]);

			} else {
				int isScale = strcmp(timelineMap->name, "scale") == 0;
				int isTranslate = strcmp(timelineMap->name, "translate") == 0;
				int isShear = strcmp(timelineMap->name, "shear") == 0;
				if (isScale || isTranslate || isShear) {
					float defaultValue = 0;
					float timelineScale = isTranslate ? self->scale: 1;
					spTranslateTimeline *timeline = 0;
					if (isScale) {
						timeline = spScaleTimeline_create(timelineMap->size);
						defaultValue = 1;
					}
					else if (isTranslate) timeline = spTranslateTimeline_create(timelineMap->size);
					else if (isShear) timeline = spShearTimeline_create(timelineMap->size);
					timeline->boneIndex = boneIndex;

					for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
						spTranslateTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0),
							Json_getFloat(valueMap, "x", defaultValue) * timelineScale,
							Json_getFloat(valueMap, "y", defaultValue) * timelineScale);
						readCurve(valueMap, SUPER(timeline), frameIndex);
					}
					animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
					animation->duration = MAX(animation->duration, timeline->frames[(timelineMap->size - 1) * TRANSLATE_ENTRIES]);

				} else {
					spAnimation_dispose(animation);
					_spSkeletonJson_setError(self, 0, "Invalid timeline type for a bone: ", timelineMap->name);
					return 0;
				}
			}
		}
	}

	/* IK constraint timelines. */
	for (constraintMap = ik ? ik->child : 0; constraintMap; constraintMap = constraintMap->next) {
		spIkConstraintData* constraint = spSkeletonData_findIkConstraint(skeletonData, constraintMap->name);
		spIkConstraintTimeline* timeline = spIkConstraintTimeline_create(constraintMap->size);
		for (frameIndex = 0; frameIndex < skeletonData->ikConstraintsCount; ++frameIndex) {
			if (constraint == skeletonData->ikConstraints[frameIndex]) {
				timeline->ikConstraintIndex = frameIndex;
				break;
			}
		}
		for (valueMap = constraintMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
			spIkConstraintTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), Json_getFloat(valueMap, "mix", 1), Json_getFloat(valueMap, "softness", 0) * self->scale,
					Json_getInt(valueMap, "bendPositive", 1) ? 1 : -1, Json_getInt(valueMap, "compress", 0) ? 1 : 0, Json_getInt(valueMap, "stretch", 0) ? 1 : 0);
			readCurve(valueMap, SUPER(timeline), frameIndex);
		}
		animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
		animation->duration = MAX(animation->duration, timeline->frames[(constraintMap->size - 1) * IKCONSTRAINT_ENTRIES]);
	}

	/* Transform constraint timelines. */
	for (constraintMap = transform ? transform->child : 0; constraintMap; constraintMap = constraintMap->next) {
		spTransformConstraintData* constraint = spSkeletonData_findTransformConstraint(skeletonData, constraintMap->name);
		spTransformConstraintTimeline* timeline = spTransformConstraintTimeline_create(constraintMap->size);
		for (frameIndex = 0; frameIndex < skeletonData->transformConstraintsCount; ++frameIndex) {
			if (constraint == skeletonData->transformConstraints[frameIndex]) {
				timeline->transformConstraintIndex = frameIndex;
				break;
			}
		}
		for (valueMap = constraintMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
			spTransformConstraintTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), Json_getFloat(valueMap, "rotateMix", 1),
					Json_getFloat(valueMap, "translateMix", 1), Json_getFloat(valueMap, "scaleMix", 1), Json_getFloat(valueMap, "shearMix", 1));
			readCurve(valueMap, SUPER(timeline), frameIndex);
		}
		animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
		animation->duration = MAX(animation->duration, timeline->frames[(constraintMap->size - 1) * TRANSFORMCONSTRAINT_ENTRIES]);
	}

	/** Path constraint timelines. */
	for(constraintMap = paths ? paths->child : 0; constraintMap; constraintMap = constraintMap->next ) {
		int constraintIndex, i;
		Json* timelineMap;

		spPathConstraintData* data = spSkeletonData_findPathConstraint(skeletonData, constraintMap->name);
		if (!data) {
			spAnimation_dispose(animation);
			_spSkeletonJson_setError(self, root, "Path constraint not found: ", constraintMap->name);
			return 0;
		}
		for (i = 0; i < skeletonData->pathConstraintsCount; i++) {
			if (skeletonData->pathConstraints[i] == data) {
				constraintIndex = i;
				break;
			}
		}

		for (timelineMap = constraintMap->child; timelineMap; timelineMap = timelineMap->next) {
			const char* timelineName = timelineMap->name;
			if (strcmp(timelineName, "position") == 0 || strcmp(timelineName, "spacing") == 0) {
				spPathConstraintPositionTimeline* timeline;
				float timelineScale = 1;
				if (strcmp(timelineName, "spacing") == 0) {
					timeline = (spPathConstraintPositionTimeline*)spPathConstraintSpacingTimeline_create(timelineMap->size);
					if (data->spacingMode == SP_SPACING_MODE_LENGTH || data->spacingMode == SP_SPACING_MODE_FIXED) timelineScale = self->scale;
				} else {
					timeline = spPathConstraintPositionTimeline_create(timelineMap->size);
					if (data->positionMode == SP_POSITION_MODE_FIXED) timelineScale = self->scale;
				}
				timeline->pathConstraintIndex = constraintIndex;
				for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
					spPathConstraintPositionTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), Json_getFloat(valueMap, timelineName, 0) * timelineScale);
					readCurve(valueMap, SUPER(timeline), frameIndex);
				}
				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
				animation->duration = MAX(animation->duration, timeline->frames[(timelineMap->size - 1) * PATHCONSTRAINTPOSITION_ENTRIES]);
			} else if (strcmp(timelineName, "mix") == 0) {
				spPathConstraintMixTimeline* timeline = spPathConstraintMixTimeline_create(timelineMap->size);
				timeline->pathConstraintIndex = constraintIndex;
				for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
					spPathConstraintMixTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0),
						Json_getFloat(valueMap, "rotateMix", 1), Json_getFloat(valueMap, "translateMix", 1));
					readCurve(valueMap, SUPER(timeline), frameIndex);
				}
				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
				animation->duration = MAX(animation->duration, timeline->frames[(timelineMap->size - 1) * PATHCONSTRAINTMIX_ENTRIES]);
			}
		}
	}

	/* Deform timelines. */
	for (constraintMap = deformJson ? deformJson->child : 0; constraintMap; constraintMap = constraintMap->next) {
		spSkin* skin = spSkeletonData_findSkin(skeletonData, constraintMap->name);
		for (slotMap = constraintMap->child; slotMap; slotMap = slotMap->next) {
			int slotIndex = spSkeletonData_findSlotIndex(skeletonData, slotMap->name);
			Json* timelineMap;
			for (timelineMap = slotMap->child; timelineMap; timelineMap = timelineMap->next) {
				float* tempDeform;
				spDeformTimeline *timeline;
				int weighted, deformLength;

				spVertexAttachment* attachment = SUB_CAST(spVertexAttachment, spSkin_getAttachment(skin, slotIndex, timelineMap->name));
				if (!attachment) {
					spAnimation_dispose(animation);
					_spSkeletonJson_setError(self, 0, "Attachment not found: ", timelineMap->name);
					return 0;
				}
				weighted = attachment->bones != 0;
				deformLength = weighted ? attachment->verticesCount / 3 * 2 : attachment->verticesCount;
				tempDeform = MALLOC(float, deformLength);

				timeline = spDeformTimeline_create(timelineMap->size, deformLength);
				timeline->slotIndex = slotIndex;
				timeline->attachment = SUPER(attachment);

				for (valueMap = timelineMap->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
					float* deform;
					Json* vertices = Json_getItem(valueMap, "vertices");
					if (!vertices) {
						if (weighted) {
							deform = tempDeform;
							memset(deform, 0, sizeof(float) * deformLength);
						} else
							deform = attachment->vertices;
					} else {
						int v, start = Json_getInt(valueMap, "offset", 0);
						Json* vertex;
						deform = tempDeform;
						memset(deform, 0, sizeof(float) * start);
						if (self->scale == 1) {
							for (vertex = vertices->child, v = start; vertex; vertex = vertex->next, ++v)
								deform[v] = vertex->valueFloat;
						} else {
							for (vertex = vertices->child, v = start; vertex; vertex = vertex->next, ++v)
								deform[v] = vertex->valueFloat * self->scale;
						}
						memset(deform + v, 0, sizeof(float) * (deformLength - v));
						if (!weighted) {
							float* verticesValues = attachment->vertices;
							for (v = 0; v < deformLength; ++v)
								deform[v] += verticesValues[v];
						}
					}
					spDeformTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), deform);
					readCurve(valueMap, SUPER(timeline), frameIndex);
				}
				FREE(tempDeform);

				animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
				animation->duration = MAX(animation->duration, timeline->frames[timelineMap->size - 1]);
			}
		}
	}

	/* Draw order timeline. */
	if (drawOrderJson) {
		spDrawOrderTimeline* timeline = spDrawOrderTimeline_create(drawOrderJson->size, skeletonData->slotsCount);
		for (valueMap = drawOrderJson->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
			int ii;
			int* drawOrder = 0;
			Json* offsets = Json_getItem(valueMap, "offsets");
			if (offsets) {
				Json* offsetMap;
				int* unchanged = MALLOC(int, skeletonData->slotsCount - offsets->size);
				int originalIndex = 0, unchangedIndex = 0;

				drawOrder = MALLOC(int, skeletonData->slotsCount);
				for (ii = skeletonData->slotsCount - 1; ii >= 0; --ii)
					drawOrder[ii] = -1;

				for (offsetMap = offsets->child; offsetMap; offsetMap = offsetMap->next) {
					int slotIndex = spSkeletonData_findSlotIndex(skeletonData, Json_getString(offsetMap, "slot", 0));
					if (slotIndex == -1) {
						spAnimation_dispose(animation);
						_spSkeletonJson_setError(self, 0, "Slot not found: ", Json_getString(offsetMap, "slot", 0));
						return 0;
					}
					/* Collect unchanged items. */
					while (originalIndex != slotIndex)
						unchanged[unchangedIndex++] = originalIndex++;
					/* Set changed items. */
					drawOrder[originalIndex + Json_getInt(offsetMap, "offset", 0)] = originalIndex;
					originalIndex++;
				}
				/* Collect remaining unchanged items. */
				while (originalIndex < skeletonData->slotsCount)
					unchanged[unchangedIndex++] = originalIndex++;
				/* Fill in unchanged items. */
				for (ii = skeletonData->slotsCount - 1; ii >= 0; ii--)
					if (drawOrder[ii] == -1) drawOrder[ii] = unchanged[--unchangedIndex];
				FREE(unchanged);
			}
			spDrawOrderTimeline_setFrame(timeline, frameIndex, Json_getFloat(valueMap, "time", 0), drawOrder);
			FREE(drawOrder);
		}
		animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
		animation->duration = MAX(animation->duration, timeline->frames[drawOrderJson->size - 1]);
	}

	/* Event timeline. */
	if (events) {
		spEventTimeline* timeline = spEventTimeline_create(events->size);
		for (valueMap = events->child, frameIndex = 0; valueMap; valueMap = valueMap->next, ++frameIndex) {
			spEvent* event;
			const char* stringValue;
			spEventData* eventData = spSkeletonData_findEvent(skeletonData, Json_getString(valueMap, "name", 0));
			if (!eventData) {
				spAnimation_dispose(animation);
				_spSkeletonJson_setError(self, 0, "Event not found: ", Json_getString(valueMap, "name", 0));
				return 0;
			}
			event = spEvent_create(Json_getFloat(valueMap, "time", 0), eventData);
			event->intValue = Json_getInt(valueMap, "int", eventData->intValue);
			event->floatValue = Json_getFloat(valueMap, "float", eventData->floatValue);
			stringValue = Json_getString(valueMap, "string", eventData->stringValue);
			if (stringValue) MALLOC_STR(event->stringValue, stringValue);
			if (eventData->audioPath) {
				event->volume = Json_getFloat(valueMap, "volume", 1);
				event->balance = Json_getFloat(valueMap, "volume", 0);
			}
			spEventTimeline_setFrame(timeline, frameIndex, event);
		}
		animation->timelines[animation->timelinesCount++] = SUPER_CAST(spTimeline, timeline);
		animation->duration = MAX(animation->duration, timeline->frames[events->size - 1]);
	}

	return animation;
}

static void _readVertices (spSkeletonJson* self, Json* attachmentMap, spVertexAttachment* attachment, int verticesLength) {
	Json* entry;
	float* vertices;
	int i, n, nn, entrySize;
	spFloatArray* weights;
	spIntArray* bones;

	attachment->worldVerticesLength = verticesLength;

	entry = Json_getItem(attachmentMap, "vertices");
	entrySize = entry->size;
	vertices = MALLOC(float, entrySize);
	for (entry = entry->child, i = 0; entry; entry = entry->next, ++i)
		vertices[i] = entry->valueFloat;

	if (verticesLength == entrySize) {
		if (self->scale != 1)
			for (i = 0; i < entrySize; ++i)
				vertices[i] *= self->scale;
		attachment->verticesCount = verticesLength;
		attachment->vertices = vertices;

		attachment->bonesCount = 0;
		attachment->bones = 0;
		return;
	}

	weights = spFloatArray_create(verticesLength * 3 * 3);
	bones = spIntArray_create(verticesLength * 3);

	for (i = 0, n = entrySize; i < n;) {
		int boneCount = (int)vertices[i++];
		spIntArray_add(bones, boneCount);
		for (nn = i + boneCount * 4; i < nn; i += 4) {
			spIntArray_add(bones, (int)vertices[i]);
			spFloatArray_add(weights, vertices[i + 1] * self->scale);
			spFloatArray_add(weights, vertices[i + 2] * self->scale);
			spFloatArray_add(weights, vertices[i + 3]);
		}
	}

	attachment->verticesCount = weights->size;
	attachment->vertices = weights->items;
	FREE(weights);
	attachment->bonesCount = bones->size;
	attachment->bones = bones->items;
	FREE(bones);

	FREE(vertices);
}

spSkeletonData* spSkeletonJson_readSkeletonDataFile (spSkeletonJson* self, const char* path) {
	int length;
	spSkeletonData* skeletonData;
	const char* json = _spUtil_readFile(path, &length);
	if (length == 0 || !json) {
		_spSkeletonJson_setError(self, 0, "Unable to read skeleton file: ", path);
		return 0;
	}
	skeletonData = spSkeletonJson_readSkeletonData(self, json);
	FREE(json);
	return skeletonData;
}

spSkeletonData* spSkeletonJson_readSkeletonData (spSkeletonJson* self, const char* json) {
	int i, ii;
	spSkeletonData* skeletonData;
	Json *root, *skeleton, *bones, *boneMap, *ik, *transform, *pathJson, *slots, *skins, *animations, *events;
	_spSkeletonJson* internal = SUB_CAST(_spSkeletonJson, self);

	FREE(self->error);
	CONST_CAST(char*, self->error) = 0;
	internal->linkedMeshCount = 0;

	root = Json_create(json);

	if (!root) {
		_spSkeletonJson_setError(self, 0, "Invalid skeleton JSON: ", Json_getError());
		return 0;
	}

	skeletonData = spSkeletonData_create();

	skeleton = Json_getItem(root, "skeleton");
	if (skeleton) {
		MALLOC_STR(skeletonData->hash, Json_getString(skeleton, "hash", 0));
		MALLOC_STR(skeletonData->version, Json_getString(skeleton, "spine", 0));
        if (strcmp(skeletonData->version, "3.8.75") == 0) {
            spSkeletonData_dispose(skeletonData);
            _spSkeletonJson_setError(self, root, "Unsupported skeleton data, please export with a newer version of Spine.", "");
            return 0;
        }
		skeletonData->x = Json_getFloat(skeleton, "x", 0);
		skeletonData->y = Json_getFloat(skeleton, "y", 0);
		skeletonData->width = Json_getFloat(skeleton, "width", 0);
		skeletonData->height = Json_getFloat(skeleton, "height", 0);
	}

	/* Bones. */
	bones = Json_getItem(root, "bones");
	skeletonData->bones = MALLOC(spBoneData*, bones->size);
	for (boneMap = bones->child, i = 0; boneMap; boneMap = boneMap->next, ++i) {
		spBoneData* data;
		const char* transformMode;

		spBoneData* parent = 0;
		const char* parentName = Json_getString(boneMap, "parent", 0);
		if (parentName) {
			parent = spSkeletonData_findBone(skeletonData, parentName);
			if (!parent) {
				spSkeletonData_dispose(skeletonData);
				_spSkeletonJson_setError(self, root, "Parent bone not found: ", parentName);
				return 0;
			}
		}

		data = spBoneData_create(skeletonData->bonesCount, Json_getString(boneMap, "name", 0), parent);
		data->length = Json_getFloat(boneMap, "length", 0) * self->scale;
		data->x = Json_getFloat(boneMap, "x", 0) * self->scale;
		data->y = Json_getFloat(boneMap, "y", 0) * self->scale;
		data->rotation = Json_getFloat(boneMap, "rotation", 0);
		data->scaleX = Json_getFloat(boneMap, "scaleX", 1);
		data->scaleY = Json_getFloat(boneMap, "scaleY", 1);
		data->shearX = Json_getFloat(boneMap, "shearX", 0);
		data->shearY = Json_getFloat(boneMap, "shearY", 0);
		transformMode = Json_getString(boneMap, "transform", "normal");
		data->transformMode = SP_TRANSFORMMODE_NORMAL;
		if (strcmp(transformMode, "normal") == 0) data->transformMode = SP_TRANSFORMMODE_NORMAL;
		else if (strcmp(transformMode, "onlyTranslation") == 0) data->transformMode = SP_TRANSFORMMODE_ONLYTRANSLATION;
		else if (strcmp(transformMode, "noRotationOrReflection") == 0) data->transformMode = SP_TRANSFORMMODE_NOROTATIONORREFLECTION;
		else if (strcmp(transformMode, "noScale") == 0) data->transformMode = SP_TRANSFORMMODE_NOSCALE;
		else if (strcmp(transformMode, "noScaleOrReflection") == 0) data->transformMode = SP_TRANSFORMMODE_NOSCALEORREFLECTION;
		data->skinRequired = Json_getInt(boneMap, "skin", 0) ? 1 : 0;

		skeletonData->bones[i] = data;
		skeletonData->bonesCount++;
	}

	/* Slots. */
	slots = Json_getItem(root, "slots");
	if (slots) {
		Json *slotMap;
		skeletonData->slotsCount = slots->size;
		skeletonData->slots = MALLOC(spSlotData*, slots->size);
		for (slotMap = slots->child, i = 0; slotMap; slotMap = slotMap->next, ++i) {
			spSlotData* data;
			const char* color;
			const char* dark;
			Json *item;

			const char* boneName = Json_getString(slotMap, "bone", 0);
			spBoneData* boneData = spSkeletonData_findBone(skeletonData, boneName);
			if (!boneData) {
				spSkeletonData_dispose(skeletonData);
				_spSkeletonJson_setError(self, root, "Slot bone not found: ", boneName);
				return 0;
			}

			data = spSlotData_create(i, Json_getString(slotMap, "name", 0), boneData);

			color = Json_getString(slotMap, "color", 0);
			if (color) {
				spColor_setFromFloats(&data->color,
					toColor(color, 0),
					toColor(color, 1),
					toColor(color, 2),
					toColor(color, 3));
			}

			dark = Json_getString(slotMap, "dark", 0);
			if (dark) {
				data->darkColor = spColor_create();
				spColor_setFromFloats(data->darkColor,
					toColor(dark, 0),
					toColor(dark, 1),
					toColor(dark, 2),
					toColor(dark, 3));
			}

			item = Json_getItem(slotMap, "attachment");
			if (item) spSlotData_setAttachmentName(data, item->valueString);

			item = Json_getItem(slotMap, "blend");
			if (item) {
				if (strcmp(item->valueString, "additive") == 0)
					data->blendMode = SP_BLEND_MODE_ADDITIVE;
				else if (strcmp(item->valueString, "multiply") == 0)
					data->blendMode = SP_BLEND_MODE_MULTIPLY;
				else if (strcmp(item->valueString, "screen") == 0)
					data->blendMode = SP_BLEND_MODE_SCREEN;
			}

			skeletonData->slots[i] = data;
		}
	}

	/* IK constraints. */
	ik = Json_getItem(root, "ik");
	if (ik) {
		Json *constraintMap;
		skeletonData->ikConstraintsCount = ik->size;
		skeletonData->ikConstraints = MALLOC(spIkConstraintData*, ik->size);
		for (constraintMap = ik->child, i = 0; constraintMap; constraintMap = constraintMap->next, ++i) {
			const char* targetName;

			spIkConstraintData* data = spIkConstraintData_create(Json_getString(constraintMap, "name", 0));
			data->order = Json_getInt(constraintMap, "order", 0);
			data->skinRequired = Json_getInt(constraintMap, "skin", 0) ? 1 : 0;

			boneMap = Json_getItem(constraintMap, "bones");
			data->bonesCount = boneMap->size;
			data->bones = MALLOC(spBoneData*, boneMap->size);
			for (boneMap = boneMap->child, ii = 0; boneMap; boneMap = boneMap->next, ++ii) {
				data->bones[ii] = spSkeletonData_findBone(skeletonData, boneMap->valueString);
				if (!data->bones[ii]) {
					spSkeletonData_dispose(skeletonData);
					_spSkeletonJson_setError(self, root, "IK bone not found: ", boneMap->valueString);
					return 0;
				}
			}

			targetName = Json_getString(constraintMap, "target", 0);
			data->target = spSkeletonData_findBone(skeletonData, targetName);
			if (!data->target) {
				spSkeletonData_dispose(skeletonData);
				_spSkeletonJson_setError(self, root, "Target bone not found: ", targetName);
				return 0;
			}

			data->bendDirection = Json_getInt(constraintMap, "bendPositive", 1) ? 1 : -1;
			data->compress = Json_getInt(constraintMap, "compress", 0) ? 1 : 0;
			data->stretch = Json_getInt(constraintMap, "stretch", 0) ? 1 : 0;
			data->uniform = Json_getInt(constraintMap, "uniform", 0) ? 1 : 0;
			data->mix = Json_getFloat(constraintMap, "mix", 1);
			data->softness = Json_getFloat(constraintMap, "softness", 0) * self->scale;

			skeletonData->ikConstraints[i] = data;
		}
	}

	/* Transform constraints. */
	transform = Json_getItem(root, "transform");
	if (transform) {
		Json *constraintMap;
		skeletonData->transformConstraintsCount = transform->size;
		skeletonData->transformConstraints = MALLOC(spTransformConstraintData*, transform->size);
		for (constraintMap = transform->child, i = 0; constraintMap; constraintMap = constraintMap->next, ++i) {
			const char* name;

			spTransformConstraintData* data = spTransformConstraintData_create(Json_getString(constraintMap, "name", 0));
			data->order = Json_getInt(constraintMap, "order", 0);
			data->skinRequired = Json_getInt(constraintMap, "skin", 0) ? 1 : 0;

			boneMap = Json_getItem(constraintMap, "bones");
			data->bonesCount = boneMap->size;
			CONST_CAST(spBoneData**, data->bones) = MALLOC(spBoneData*, boneMap->size);
			for (boneMap = boneMap->child, ii = 0; boneMap; boneMap = boneMap->next, ++ii) {
				data->bones[ii] = spSkeletonData_findBone(skeletonData, boneMap->valueString);
				if (!data->bones[ii]) {
					spSkeletonData_dispose(skeletonData);
					_spSkeletonJson_setError(self, root, "Transform bone not found: ", boneMap->valueString);
					return 0;
				}
			}

			name = Json_getString(constraintMap, "target", 0);
			data->target = spSkeletonData_findBone(skeletonData, name);
			if (!data->target) {
				spSkeletonData_dispose(skeletonData);
				_spSkeletonJson_setError(self, root, "Target bone not found: ", name);
				return 0;
			}

			data->local = Json_getInt(constraintMap, "local", 0);
			data->relative = Json_getInt(constraintMap, "relative", 0);
			data->offsetRotation = Json_getFloat(constraintMap, "rotation", 0);
			data->offsetX = Json_getFloat(constraintMap, "x", 0) * self->scale;
			data->offsetY = Json_getFloat(constraintMap, "y", 0) * self->scale;
			data->offsetScaleX = Json_getFloat(constraintMap, "scaleX", 0);
			data->offsetScaleY = Json_getFloat(constraintMap, "scaleY", 0);
			data->offsetShearY = Json_getFloat(constraintMap, "shearY", 0);

			data->rotateMix = Json_getFloat(constraintMap, "rotateMix", 1);
			data->translateMix = Json_getFloat(constraintMap, "translateMix", 1);
			data->scaleMix = Json_getFloat(constraintMap, "scaleMix", 1);
			data->shearMix = Json_getFloat(constraintMap, "shearMix", 1);

			skeletonData->transformConstraints[i] = data;
		}
	}

	/* Path constraints */
	pathJson = Json_getItem(root, "path");
	if (pathJson) {
		Json *constraintMap;
		skeletonData->pathConstraintsCount = pathJson->size;
		skeletonData->pathConstraints = MALLOC(spPathConstraintData*, pathJson->size);
		for (constraintMap = pathJson->child, i = 0; constraintMap; constraintMap = constraintMap->next, ++i) {
			const char* name;
			const char* item;

			spPathConstraintData* data = spPathConstraintData_create(Json_getString(constraintMap, "name", 0));
			data->order = Json_getInt(constraintMap, "order", 0);
			data->skinRequired = Json_getInt(constraintMap, "skin", 0) ? 1 : 0;

			boneMap = Json_getItem(constraintMap, "bones");
			data->bonesCount = boneMap->size;
			CONST_CAST(spBoneData**, data->bones) = MALLOC(spBoneData*, boneMap->size);
			for (boneMap = boneMap->child, ii = 0; boneMap; boneMap = boneMap->next, ++ii) {
				data->bones[ii] = spSkeletonData_findBone(skeletonData, boneMap->valueString);
				if (!data->bones[ii]) {
					spSkeletonData_dispose(skeletonData);
					_spSkeletonJson_setError(self, root, "Path bone not found: ", boneMap->valueString);
					return 0;
				}
			}

			name = Json_getString(constraintMap, "target", 0);
			data->target = spSkeletonData_findSlot(skeletonData, name);
			if (!data->target) {
				spSkeletonData_dispose(skeletonData);
				_spSkeletonJson_setError(self, root, "Target slot not found: ", name);
				return 0;
			}

			item = Json_getString(constraintMap, "positionMode", "percent");
			if (strcmp(item, "fixed") == 0) data->positionMode = SP_POSITION_MODE_FIXED;
			else if (strcmp(item, "percent") == 0) data->positionMode = SP_POSITION_MODE_PERCENT;

			item = Json_getString(constraintMap, "spacingMode", "length");
			if (strcmp(item, "length") == 0) data->spacingMode = SP_SPACING_MODE_LENGTH;
			else if (strcmp(item, "fixed") == 0) data->spacingMode = SP_SPACING_MODE_FIXED;
			else if (strcmp(item, "percent") == 0) data->spacingMode = SP_SPACING_MODE_PERCENT;

			item = Json_getString(constraintMap, "rotateMode", "tangent");
			if (strcmp(item, "tangent") == 0) data->rotateMode = SP_ROTATE_MODE_TANGENT;
			else if (strcmp(item, "chain") == 0) data->rotateMode = SP_ROTATE_MODE_CHAIN;
			else if (strcmp(item, "chainScale") == 0) data->rotateMode = SP_ROTATE_MODE_CHAIN_SCALE;

			data->offsetRotation = Json_getFloat(constraintMap, "rotation", 0);
			data->position = Json_getFloat(constraintMap, "position", 0);
			if (data->positionMode == SP_POSITION_MODE_FIXED) data->position *= self->scale;
			data->spacing = Json_getFloat(constraintMap, "spacing", 0);
			if (data->spacingMode == SP_SPACING_MODE_LENGTH || data->spacingMode == SP_SPACING_MODE_FIXED) data->spacing *= self->scale;
			data->rotateMix = Json_getFloat(constraintMap, "rotateMix", 1);
			data->translateMix = Json_getFloat(constraintMap, "translateMix", 1);

			skeletonData->pathConstraints[i] = data;
		}
	}

	/* Skins. */
	skins = Json_getItem(root, "skins");
	if (skins) {
		Json *skinMap;
		skeletonData->skins = MALLOC(spSkin*, skins->size);
		for (skinMap = skins->child, i = 0; skinMap; skinMap = skinMap->next, ++i) {
			Json *attachmentsMap;
			Json *curves;
			Json *skinPart;
			spSkin *skin = spSkin_create(Json_getString(skinMap, "name", ""));

			skinPart = Json_getItem(skinMap, "bones");
			if (skinPart) {
				for(skinPart = skinPart->child; skinPart; skinPart = skinPart->next) {
					spBoneData* bone = spSkeletonData_findBone(skeletonData, skinPart->valueString);
					if (!bone) {
						spSkeletonData_dispose(skeletonData);
						_spSkeletonJson_setError(self, root, "Skin bone constraint not found: ", skinPart->valueString);
						return 0;
					}
					spBoneDataArray_add(skin->bones, bone);
				}
			}

			skinPart = Json_getItem(skinMap, "ik");
			if (skinPart) {
				for(skinPart = skinPart->child; skinPart; skinPart = skinPart->next) {
					spIkConstraintData* constraint = spSkeletonData_findIkConstraint(skeletonData, skinPart->valueString);
					if (!constraint) {
						spSkeletonData_dispose(skeletonData);
						_spSkeletonJson_setError(self, root, "Skin IK constraint not found: ", skinPart->valueString);
						return 0;
					}
					spIkConstraintDataArray_add(skin->ikConstraints, constraint);
				}
			}

			skinPart = Json_getItem(skinMap, "path");
			if (skinPart) {
				for(skinPart = skinPart->child; skinPart; skinPart = skinPart->next) {
					spPathConstraintData* constraint = spSkeletonData_findPathConstraint(skeletonData, skinPart->valueString);
					if (!constraint) {
						spSkeletonData_dispose(skeletonData);
						_spSkeletonJson_setError(self, root, "Skin path constraint not found: ", skinPart->valueString);
						return 0;
					}
					spPathConstraintDataArray_add(skin->pathConstraints, constraint);
				}
			}

			skinPart = Json_getItem(skinMap, "transform");
			if (skinPart) {
				for(skinPart = skinPart->child; skinPart; skinPart = skinPart->next) {
					spTransformConstraintData* constraint = spSkeletonData_findTransformConstraint(skeletonData, skinPart->valueString);
					if (!constraint) {
						spSkeletonData_dispose(skeletonData);
						_spSkeletonJson_setError(self, root, "Skin transform constraint not found: ", skinPart->valueString);
						return 0;
					}
					spTransformConstraintDataArray_add(skin->transformConstraints, constraint);
				}
			}

			skeletonData->skins[skeletonData->skinsCount++] = skin;
			if (strcmp(skin->name, "default") == 0) skeletonData->defaultSkin = skin;

            attachmentsMap = Json_getItem(skinMap, "attachments")->child;

			for (attachmentsMap = Json_getItem(skinMap, "attachments")->child; attachmentsMap; attachmentsMap = attachmentsMap->next) {
				spSlotData* slot = spSkeletonData_findSlot(skeletonData, attachmentsMap->name);
				Json *attachmentMap;

				for (attachmentMap = attachmentsMap->child; attachmentMap; attachmentMap = attachmentMap->next) {
					spAttachment* attachment;
					const char* skinAttachmentName = attachmentMap->name;
					const char* attachmentName = Json_getString(attachmentMap, "name", skinAttachmentName);
					const char* path = Json_getString(attachmentMap, "path", attachmentName);
					const char* color;
					Json* entry;

					const char* typeString = Json_getString(attachmentMap, "type", "region");
					spAttachmentType type;
					if (strcmp(typeString, "region") == 0) type = SP_ATTACHMENT_REGION;
					else if (strcmp(typeString, "mesh") == 0) type = SP_ATTACHMENT_MESH;
					else if (strcmp(typeString, "linkedmesh") == 0) type = SP_ATTACHMENT_LINKED_MESH;
					else if (strcmp(typeString, "boundingbox") == 0) type = SP_ATTACHMENT_BOUNDING_BOX;
					else if (strcmp(typeString, "path") == 0) type = SP_ATTACHMENT_PATH;
					else if	(strcmp(typeString, "clipping") == 0) type = SP_ATTACHMENT_CLIPPING;
					else if	(strcmp(typeString, "point") == 0) type = SP_ATTACHMENT_POINT;
					else {
						spSkeletonData_dispose(skeletonData);
						_spSkeletonJson_setError(self, root, "Unknown attachment type: ", typeString);
						return 0;
					}

					attachment = spAttachmentLoader_createAttachment(self->attachmentLoader, skin, type, attachmentName, path);
					if (!attachment) {
						if (self->attachmentLoader->error1) {
							spSkeletonData_dispose(skeletonData);
							_spSkeletonJson_setError(self, root, self->attachmentLoader->error1, self->attachmentLoader->error2);
							return 0;
						}
						continue;
					}

					switch (attachment->type) {
					case SP_ATTACHMENT_REGION: {
						spRegionAttachment* region = SUB_CAST(spRegionAttachment, attachment);
						if (path) MALLOC_STR(region->path, path);
						region->x = Json_getFloat(attachmentMap, "x", 0) * self->scale;
						region->y = Json_getFloat(attachmentMap, "y", 0) * self->scale;
						region->scaleX = Json_getFloat(attachmentMap, "scaleX", 1);
						region->scaleY = Json_getFloat(attachmentMap, "scaleY", 1);
						region->rotation = Json_getFloat(attachmentMap, "rotation", 0);
						region->width = Json_getFloat(attachmentMap, "width", 32) * self->scale;
						region->height = Json_getFloat(attachmentMap, "height", 32) * self->scale;

						color = Json_getString(attachmentMap, "color", 0);
						if (color) {
							spColor_setFromFloats(&region->color,
								toColor(color, 0),
								toColor(color, 1),
								toColor(color, 2),
								toColor(color, 3));
						}

						spRegionAttachment_updateOffset(region);

						spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
						break;
					}
					case SP_ATTACHMENT_MESH:
					case SP_ATTACHMENT_LINKED_MESH: {
						spMeshAttachment* mesh = SUB_CAST(spMeshAttachment, attachment);

						MALLOC_STR(mesh->path, path);

						color = Json_getString(attachmentMap, "color", 0);
						if (color) {
							spColor_setFromFloats(&mesh->color,
								toColor(color, 0),
								toColor(color, 1),
								toColor(color, 2),
								toColor(color, 3));
						}

						mesh->width = Json_getFloat(attachmentMap, "width", 32) * self->scale;
						mesh->height = Json_getFloat(attachmentMap, "height", 32) * self->scale;

						entry = Json_getItem(attachmentMap, "parent");
						if (!entry) {
							int verticesLength;
							entry = Json_getItem(attachmentMap, "triangles");
							mesh->trianglesCount = entry->size;
							mesh->triangles = MALLOC(unsigned short, entry->size);
							for (entry = entry->child, ii = 0; entry; entry = entry->next, ++ii)
								mesh->triangles[ii] = (unsigned short)entry->valueInt;

							entry = Json_getItem(attachmentMap, "uvs");
							verticesLength = entry->size;
							mesh->regionUVs = MALLOC(float, verticesLength);
							for (entry = entry->child, ii = 0; entry; entry = entry->next, ++ii)
								mesh->regionUVs[ii] = entry->valueFloat;

							_readVertices(self, attachmentMap, SUPER(mesh), verticesLength);

							spMeshAttachment_updateUVs(mesh);

							mesh->hullLength = Json_getInt(attachmentMap, "hull", 0);

							entry = Json_getItem(attachmentMap, "edges");
							if (entry) {
								mesh->edgesCount = entry->size;
								mesh->edges = MALLOC(int, entry->size);
								for (entry = entry->child, ii = 0; entry; entry = entry->next, ++ii)
									mesh->edges[ii] = entry->valueInt;
							}

							spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
						} else {
							int inheritDeform = Json_getInt(attachmentMap, "deform", 1);
							_spSkeletonJson_addLinkedMesh(self, SUB_CAST(spMeshAttachment, attachment),
								Json_getString(attachmentMap, "skin", 0), slot->index, entry->valueString, inheritDeform);
						}
						break;
					}
					case SP_ATTACHMENT_BOUNDING_BOX: {
						spBoundingBoxAttachment* box = SUB_CAST(spBoundingBoxAttachment, attachment);
						int vertexCount = Json_getInt(attachmentMap, "vertexCount", 0) << 1;
						_readVertices(self, attachmentMap, SUPER(box), vertexCount);
						box->super.verticesCount = vertexCount;
						spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
						break;
					}
					case SP_ATTACHMENT_PATH: {
						spPathAttachment* pathAttachment = SUB_CAST(spPathAttachment, attachment);
						int vertexCount = 0;
						pathAttachment->closed = Json_getInt(attachmentMap, "closed", 0);
						pathAttachment->constantSpeed = Json_getInt(attachmentMap, "constantSpeed", 1);
						vertexCount = Json_getInt(attachmentMap, "vertexCount", 0);
						_readVertices(self, attachmentMap, SUPER(pathAttachment), vertexCount << 1);

						pathAttachment->lengthsLength = vertexCount / 3;
						pathAttachment->lengths = MALLOC(float, pathAttachment->lengthsLength);

						curves = Json_getItem(attachmentMap, "lengths");
						for (curves = curves->child, ii = 0; curves; curves = curves->next, ++ii)
							pathAttachment->lengths[ii] = curves->valueFloat * self->scale;
						break;
					}
					case SP_ATTACHMENT_POINT: {
						spPointAttachment* point = SUB_CAST(spPointAttachment, attachment);
						point->x = Json_getFloat(attachmentMap, "x", 0) * self->scale;
						point->y = Json_getFloat(attachmentMap, "y", 0) * self->scale;
						point->rotation = Json_getFloat(attachmentMap, "rotation", 0);

						color = Json_getString(attachmentMap, "color", 0);
						if (color) {
							spColor_setFromFloats(&point->color,
								toColor(color, 0),
								toColor(color, 1),
								toColor(color, 2),
								toColor(color, 3));
						}
						break;
					}
					case SP_ATTACHMENT_CLIPPING: {
						spClippingAttachment* clip = SUB_CAST(spClippingAttachment, attachment);
						int vertexCount = 0;
						const char* end = Json_getString(attachmentMap, "end", 0);
						if (end) {
							spSlotData* endSlot = spSkeletonData_findSlot(skeletonData, end);
							clip->endSlot = endSlot;
						}
						vertexCount = Json_getInt(attachmentMap, "vertexCount", 0) << 1;
						_readVertices(self, attachmentMap, SUPER(clip), vertexCount);
						spAttachmentLoader_configureAttachment(self->attachmentLoader, attachment);
						break;
					}
					}

					spSkin_setAttachment(skin, slot->index, skinAttachmentName, attachment);
				}
			}
		}
	}

	/* Linked meshes. */
	for (i = 0; i < internal->linkedMeshCount; i++) {
		spAttachment* parent;
		_spLinkedMesh* linkedMesh = internal->linkedMeshes + i;
		spSkin* skin = !linkedMesh->skin ? skeletonData->defaultSkin : spSkeletonData_findSkin(skeletonData, linkedMesh->skin);
		if (!skin) {
			spSkeletonData_dispose(skeletonData);
			_spSkeletonJson_setError(self, 0, "Skin not found: ", linkedMesh->skin);
			return 0;
		}
		parent = spSkin_getAttachment(skin, linkedMesh->slotIndex, linkedMesh->parent);
		if (!parent) {
			spSkeletonData_dispose(skeletonData);
			_spSkeletonJson_setError(self, 0, "Parent mesh not found: ", linkedMesh->parent);
			return 0;
		}
		linkedMesh->mesh->super.deformAttachment = linkedMesh->inheritDeform ? SUB_CAST(spVertexAttachment, parent) : SUB_CAST(spVertexAttachment, linkedMesh->mesh);
		spMeshAttachment_setParentMesh(linkedMesh->mesh, SUB_CAST(spMeshAttachment, parent));
		spMeshAttachment_updateUVs(linkedMesh->mesh);
		spAttachmentLoader_configureAttachment(self->attachmentLoader, SUPER(SUPER(linkedMesh->mesh)));
	}

	/* Events. */
	events = Json_getItem(root, "events");
	if (events) {
		Json *eventMap;
		const char* stringValue;
		const char* audioPath;
		skeletonData->eventsCount = events->size;
		skeletonData->events = MALLOC(spEventData*, events->size);
		for (eventMap = events->child, i = 0; eventMap; eventMap = eventMap->next, ++i) {
			spEventData* eventData = spEventData_create(eventMap->name);
			eventData->intValue = Json_getInt(eventMap, "int", 0);
			eventData->floatValue = Json_getFloat(eventMap, "float", 0);
			stringValue = Json_getString(eventMap, "string", 0);
			if (stringValue) MALLOC_STR(eventData->stringValue, stringValue);
			audioPath = Json_getString(eventMap, "audio", 0);
			if (audioPath) {
				MALLOC_STR(eventData->audioPath, audioPath);
				eventData->volume = Json_getFloat(eventMap, "volume", 1);
				eventData->balance = Json_getFloat(eventMap, "balance", 0);
			}
			skeletonData->events[i] = eventData;
		}
	}

	/* Animations. */
	animations = Json_getItem(root, "animations");
	if (animations) {
		Json *animationMap;
		skeletonData->animations = MALLOC(spAnimation*, animations->size);
		for (animationMap = animations->child; animationMap; animationMap = animationMap->next) {
			spAnimation* animation = _spSkeletonJson_readAnimation(self, animationMap, skeletonData);
			if (!animation) {
				spSkeletonData_dispose(skeletonData);
				return 0;
			}
			skeletonData->animations[skeletonData->animationsCount++] = animation;
		}
	}

	Json_dispose(root);
	return skeletonData;
}
