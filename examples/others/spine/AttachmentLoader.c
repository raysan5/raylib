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

#include <spine/AttachmentLoader.h>
#include <stdio.h>
#include <spine/extension.h>

typedef struct _spAttachmentLoaderVtable {
	spAttachment* (*createAttachment) (spAttachmentLoader* self, spSkin* skin, spAttachmentType type, const char* name, const char* path);
	void (*configureAttachment) (spAttachmentLoader* self, spAttachment*);
	void (*disposeAttachment) (spAttachmentLoader* self, spAttachment*);
	void (*dispose) (spAttachmentLoader* self);
} _spAttachmentLoaderVtable;

void _spAttachmentLoader_init (spAttachmentLoader* self,
	void (*dispose) (spAttachmentLoader* self),
	spAttachment* (*createAttachment) (spAttachmentLoader* self, spSkin* skin, spAttachmentType type, const char* name,
		const char* path),
	void (*configureAttachment) (spAttachmentLoader* self, spAttachment*),
	void (*disposeAttachment) (spAttachmentLoader* self, spAttachment*)
) {
	CONST_CAST(_spAttachmentLoaderVtable*, self->vtable) = NEW(_spAttachmentLoaderVtable);
	VTABLE(spAttachmentLoader, self)->dispose = dispose;
	VTABLE(spAttachmentLoader, self)->createAttachment = createAttachment;
	VTABLE(spAttachmentLoader, self)->configureAttachment = configureAttachment;
	VTABLE(spAttachmentLoader, self)->disposeAttachment = disposeAttachment;
}

void _spAttachmentLoader_deinit (spAttachmentLoader* self) {
	FREE(self->vtable);
	FREE(self->error1);
	FREE(self->error2);
}

void spAttachmentLoader_dispose (spAttachmentLoader* self) {
	VTABLE(spAttachmentLoader, self)->dispose(self);
	FREE(self);
}

spAttachment* spAttachmentLoader_createAttachment (spAttachmentLoader* self, spSkin* skin, spAttachmentType type, const char* name,
		const char* path) {
	FREE(self->error1);
	FREE(self->error2);
	self->error1 = 0;
	self->error2 = 0;
	return VTABLE(spAttachmentLoader, self)->createAttachment(self, skin, type, name, path);
}

void spAttachmentLoader_configureAttachment (spAttachmentLoader* self, spAttachment* attachment) {
	if (!VTABLE(spAttachmentLoader, self)->configureAttachment) return;
	VTABLE(spAttachmentLoader, self)->configureAttachment(self, attachment);
}

void spAttachmentLoader_disposeAttachment (spAttachmentLoader* self, spAttachment* attachment) {
	if (!VTABLE(spAttachmentLoader, self)->disposeAttachment) return;
	VTABLE(spAttachmentLoader, self)->disposeAttachment(self, attachment);
}

void _spAttachmentLoader_setError (spAttachmentLoader* self, const char* error1, const char* error2) {
	FREE(self->error1);
	FREE(self->error2);
	MALLOC_STR(self->error1, error1);
	MALLOC_STR(self->error2, error2);
}

void _spAttachmentLoader_setUnknownTypeError (spAttachmentLoader* self, spAttachmentType type) {
	char buffer[16];
	sprintf(buffer, "%d", type);
	_spAttachmentLoader_setError(self, "Unknown attachment type: ", buffer);
}
