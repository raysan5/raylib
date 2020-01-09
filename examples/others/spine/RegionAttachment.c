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

#include <spine/RegionAttachment.h>
#include <spine/extension.h>

typedef enum {
	BLX = 0, BLY, ULX, ULY, URX, URY, BRX, BRY
} spVertexIndex;

void _spRegionAttachment_dispose (spAttachment* attachment) {
	spRegionAttachment* self = SUB_CAST(spRegionAttachment, attachment);
	_spAttachment_deinit(attachment);
	FREE(self->path);
	FREE(self);
}

spAttachment* _spRegionAttachment_copy (spAttachment* attachment) {
	spRegionAttachment* self = SUB_CAST(spRegionAttachment, attachment);
	spRegionAttachment* copy = spRegionAttachment_create(attachment->name);
	copy->regionWidth = self->regionWidth;
	copy->regionHeight = self->regionHeight;
	copy->regionOffsetX = self->regionOffsetX;
	copy->regionOffsetY = self->regionOffsetY;
	copy->regionOriginalWidth = self->regionOriginalWidth;
	copy->regionOriginalHeight = self->regionOriginalHeight;
	copy->rendererObject = self->rendererObject;
	MALLOC_STR(copy->path, self->path);
	copy->x = self->x;
	copy->y = self->y;
	copy->scaleX = self->scaleX;
	copy->scaleY = self->scaleY;
	copy->rotation = self->rotation;
	copy->width = self->width;
	copy->height = self->height;
	memcpy(copy->uvs, self->uvs, sizeof(float) * 8);
	memcpy(copy->offset, self->offset, sizeof(float) * 8);
	spColor_setFromColor(&copy->color, &self->color);
	return SUPER(copy);
}

spRegionAttachment* spRegionAttachment_create (const char* name) {
	spRegionAttachment* self = NEW(spRegionAttachment);
	self->scaleX = 1;
	self->scaleY = 1;
	spColor_setFromFloats(&self->color, 1, 1, 1, 1);
	_spAttachment_init(SUPER(self), name, SP_ATTACHMENT_REGION, _spRegionAttachment_dispose, _spRegionAttachment_copy);
	return self;
}

void spRegionAttachment_setUVs (spRegionAttachment* self, float u, float v, float u2, float v2, int/*bool*/rotate) {
	if (rotate) {
		self->uvs[URX] = u;
		self->uvs[URY] = v2;
		self->uvs[BRX] = u;
		self->uvs[BRY] = v;
		self->uvs[BLX] = u2;
		self->uvs[BLY] = v;
		self->uvs[ULX] = u2;
		self->uvs[ULY] = v2;
	} else {
		self->uvs[ULX] = u;
		self->uvs[ULY] = v2;
		self->uvs[URX] = u;
		self->uvs[URY] = v;
		self->uvs[BRX] = u2;
		self->uvs[BRY] = v;
		self->uvs[BLX] = u2;
		self->uvs[BLY] = v2;
	}
}

void spRegionAttachment_updateOffset (spRegionAttachment* self) {
	float regionScaleX = self->width / self->regionOriginalWidth * self->scaleX;
	float regionScaleY = self->height / self->regionOriginalHeight * self->scaleY;
	float localX = -self->width / 2 * self->scaleX + self->regionOffsetX * regionScaleX;
	float localY = -self->height / 2 * self->scaleY + self->regionOffsetY * regionScaleY;
	float localX2 = localX + self->regionWidth * regionScaleX;
	float localY2 = localY + self->regionHeight * regionScaleY;
	float radians = self->rotation * DEG_RAD;
	float cosine = COS(radians), sine = SIN(radians);
	float localXCos = localX * cosine + self->x;
	float localXSin = localX * sine;
	float localYCos = localY * cosine + self->y;
	float localYSin = localY * sine;
	float localX2Cos = localX2 * cosine + self->x;
	float localX2Sin = localX2 * sine;
	float localY2Cos = localY2 * cosine + self->y;
	float localY2Sin = localY2 * sine;
	self->offset[BLX] = localXCos - localYSin;
	self->offset[BLY] = localYCos + localXSin;
	self->offset[ULX] = localXCos - localY2Sin;
	self->offset[ULY] = localY2Cos + localXSin;
	self->offset[URX] = localX2Cos - localY2Sin;
	self->offset[URY] = localY2Cos + localX2Sin;
	self->offset[BRX] = localX2Cos - localYSin;
	self->offset[BRY] = localYCos + localX2Sin;
}

void spRegionAttachment_computeWorldVertices (spRegionAttachment* self, spBone* bone, float* vertices, int offset, int stride) {
	const float* offsets = self->offset;
	float x = bone->worldX, y = bone->worldY;
	float offsetX, offsetY;

	offsetX = offsets[BRX];
	offsetY = offsets[BRY];
	vertices[offset] = offsetX * bone->a + offsetY * bone->b + x; /* br */
	vertices[offset + 1] = offsetX * bone->c + offsetY * bone->d + y;
	offset += stride;

	offsetX = offsets[BLX];
	offsetY = offsets[BLY];
	vertices[offset] = offsetX * bone->a + offsetY * bone->b + x; /* bl */
	vertices[offset + 1] = offsetX * bone->c + offsetY * bone->d + y;
	offset += stride;

	offsetX = offsets[ULX];
	offsetY = offsets[ULY];
	vertices[offset] = offsetX * bone->a + offsetY * bone->b + x; /* ul */
	vertices[offset + 1] = offsetX * bone->c + offsetY * bone->d + y;
	offset += stride;

	offsetX = offsets[URX];
	offsetY = offsets[URY];
	vertices[offset] = offsetX * bone->a + offsetY * bone->b + x; /* ur */
	vertices[offset + 1] = offsetX * bone->c + offsetY * bone->d + y;
}
