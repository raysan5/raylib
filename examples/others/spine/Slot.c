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

#include <spine/Slot.h>
#include <spine/extension.h>

typedef struct {
	spSlot super;
	float attachmentTime;
} _spSlot;

spSlot* spSlot_create (spSlotData* data, spBone* bone) {
	spSlot* self = SUPER(NEW(_spSlot));
	CONST_CAST(spSlotData*, self->data) = data;
	CONST_CAST(spBone*, self->bone) = bone;
	spColor_setFromFloats(&self->color, 1, 1, 1, 1);
	self->darkColor = data->darkColor == 0 ? 0 : spColor_create();
	spSlot_setToSetupPose(self);
	return self;
}

void spSlot_dispose (spSlot* self) {
	FREE(self->deform);
	FREE(self->darkColor);
	FREE(self);
}

void spSlot_setAttachment (spSlot* self, spAttachment* attachment) {
	if (attachment == self->attachment) return;
	CONST_CAST(spAttachment*, self->attachment) = attachment;
	SUB_CAST(_spSlot, self)->attachmentTime = self->bone->skeleton->time;
	self->deformCount = 0;
}

void spSlot_setAttachmentTime (spSlot* self, float time) {
	SUB_CAST(_spSlot, self)->attachmentTime = self->bone->skeleton->time - time;
}

float spSlot_getAttachmentTime (const spSlot* self) {
	return self->bone->skeleton->time - SUB_CAST(_spSlot, self) ->attachmentTime;
}

void spSlot_setToSetupPose (spSlot* self) {
	spColor_setFromColor(&self->color, &self->data->color);
	if (self->darkColor) spColor_setFromColor(self->darkColor, self->data->darkColor);

	if (!self->data->attachmentName)
		spSlot_setAttachment(self, 0);
	else {
		spAttachment* attachment = spSkeleton_getAttachmentForSlotIndex(
			self->bone->skeleton, self->data->index, self->data->attachmentName);
		CONST_CAST(spAttachment*, self->attachment) = 0;
		spSlot_setAttachment(self, attachment);
	}
}
