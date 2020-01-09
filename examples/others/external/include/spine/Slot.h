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

#ifndef SPINE_SLOT_H_
#define SPINE_SLOT_H_

#include <spine/dll.h>
#include <spine/Bone.h>
#include <spine/Attachment.h>
#include <spine/SlotData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spSlot {
	spSlotData* const data;
	spBone* const bone;
	spColor color;
	spColor* darkColor;
	spAttachment* const attachment;

	int deformCapacity;
	int deformCount;
	float* deform;

#ifdef __cplusplus
	spSlot() :
		data(0),
		bone(0),
		color(),
		darkColor(0),
		attachment(0),
		deformCapacity(0),
		deformCount(0),
		deform(0) {
	}
#endif
} spSlot;

SP_API spSlot* spSlot_create (spSlotData* data, spBone* bone);
SP_API void spSlot_dispose (spSlot* self);

/* @param attachment May be 0 to clear the attachment for the slot. */
SP_API void spSlot_setAttachment (spSlot* self, spAttachment* attachment);

SP_API void spSlot_setAttachmentTime (spSlot* self, float time);
SP_API float spSlot_getAttachmentTime (const spSlot* self);

SP_API void spSlot_setToSetupPose (spSlot* self);

#ifdef SPINE_SHORT_NAMES
typedef spSlot Slot;
#define Slot_create(...) spSlot_create(__VA_ARGS__)
#define Slot_dispose(...) spSlot_dispose(__VA_ARGS__)
#define Slot_setAttachment(...) spSlot_setAttachment(__VA_ARGS__)
#define Slot_setAttachmentTime(...) spSlot_setAttachmentTime(__VA_ARGS__)
#define Slot_getAttachmentTime(...) spSlot_getAttachmentTime(__VA_ARGS__)
#define Slot_setToSetupPose(...) spSlot_setToSetupPose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SLOT_H_ */
