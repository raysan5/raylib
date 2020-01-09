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

#include <spine/Attachment.h>
#include <spine/extension.h>
#include <spine/Slot.h>

typedef struct _spAttachmentVtable {
	void (*dispose) (spAttachment* self);
	spAttachment* (*copy) (spAttachment* self);
} _spAttachmentVtable;

void _spAttachment_init (spAttachment* self, const char* name, spAttachmentType type, /**/
		void (*dispose) (spAttachment* self), spAttachment* (*copy) (spAttachment* self)) {

	CONST_CAST(_spAttachmentVtable*, self->vtable) = NEW(_spAttachmentVtable);
	VTABLE(spAttachment, self)->dispose = dispose;
	VTABLE(spAttachment, self)->copy = copy;

	MALLOC_STR(self->name, name);
	CONST_CAST(spAttachmentType, self->type) = type;
}

void _spAttachment_deinit (spAttachment* self) {
	if (self->attachmentLoader) spAttachmentLoader_disposeAttachment(self->attachmentLoader, self);
	FREE(self->vtable);
	FREE(self->name);
}

spAttachment* spAttachment_copy (spAttachment* self) {
	return VTABLE(spAttachment, self) ->copy(self);
}

void spAttachment_dispose (spAttachment* self) {
	self->refCount--;
	if (self->refCount <= 0)
		VTABLE(spAttachment, self) ->dispose(self);
}
