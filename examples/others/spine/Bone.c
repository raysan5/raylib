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

#include <spine/Bone.h>
#include <spine/extension.h>
#include <stdio.h>
static int yDown;

void spBone_setYDown (int value) {
	yDown = value;
}

int spBone_isYDown () {
	return yDown;
}

spBone* spBone_create (spBoneData* data, spSkeleton* skeleton, spBone* parent) {
	spBone* self = NEW(spBone);
	CONST_CAST(spBoneData*, self->data) = data;
	CONST_CAST(spSkeleton*, self->skeleton) = skeleton;
	CONST_CAST(spBone*, self->parent) = parent;
	CONST_CAST(float, self->a) = 1.0f;
	CONST_CAST(float, self->d) = 1.0f;
	spBone_setToSetupPose(self);
	return self;
}

void spBone_dispose (spBone* self) {
	FREE(self->children);
	FREE(self);
}

void spBone_updateWorldTransform (spBone* self) {
	spBone_updateWorldTransformWith(self, self->x, self->y, self->rotation, self->scaleX, self->scaleY, self->shearX, self->shearY);
}

void spBone_updateWorldTransformWith (spBone* self, float x, float y, float rotation, float scaleX, float scaleY, float shearX, float shearY) {
	float cosine, sine;
	float pa, pb, pc, pd;
	spBone* parent = self->parent;
	float sx = self->skeleton->scaleX;
	float sy = self->skeleton->scaleY * (spBone_isYDown() ? -1 : 1);

	self->ax = x;
	self->ay = y;
	self->arotation = rotation;
	self->ascaleX = scaleX;
	self->ascaleY = scaleY;
	self->ashearX = shearX;
	self->ashearY = shearY;
	self->appliedValid = 1;

	if (!parent) { /* Root bone. */
		float rotationY = rotation + 90 + shearY;
		CONST_CAST(float, self->a) = COS_DEG(rotation + shearX) * scaleX * sx;
		CONST_CAST(float, self->b) = COS_DEG(rotationY) * scaleY * sx;
		CONST_CAST(float, self->c) = SIN_DEG(rotation + shearX) * scaleX * sy;
		CONST_CAST(float, self->d) = SIN_DEG(rotationY) * scaleY * sy;
		CONST_CAST(float, self->worldX) = x * sx + self->skeleton->x;
		CONST_CAST(float, self->worldY) = y * sy + self->skeleton->y;
		return;
	}

	pa = parent->a;
	pb = parent->b;
	pc = parent->c;
	pd = parent->d;

	CONST_CAST(float, self->worldX) = pa * x + pb * y + parent->worldX;
	CONST_CAST(float, self->worldY) = pc * x + pd * y + parent->worldY;

	switch (self->data->transformMode) {
	case SP_TRANSFORMMODE_NORMAL: {
		float rotationY = rotation + 90 + shearY;
		float la = COS_DEG(rotation + shearX) * scaleX;
		float lb = COS_DEG(rotationY) * scaleY;
		float lc = SIN_DEG(rotation + shearX) * scaleX;
		float ld = SIN_DEG(rotationY) * scaleY;
		CONST_CAST(float, self->a) = pa * la + pb * lc;
		CONST_CAST(float, self->b) = pa * lb + pb * ld;
		CONST_CAST(float, self->c) = pc * la + pd * lc;
		CONST_CAST(float, self->d) = pc * lb + pd * ld;
		return;
	}
	case SP_TRANSFORMMODE_ONLYTRANSLATION: {
		float rotationY = rotation + 90 + shearY;
		CONST_CAST(float, self->a) = COS_DEG(rotation + shearX) * scaleX;
		CONST_CAST(float, self->b) = COS_DEG(rotationY) * scaleY;
		CONST_CAST(float, self->c) = SIN_DEG(rotation + shearX) * scaleX;
		CONST_CAST(float, self->d) = SIN_DEG(rotationY) * scaleY;
		break;
	}
	case SP_TRANSFORMMODE_NOROTATIONORREFLECTION: {
		float s = pa * pa + pc * pc;
		float prx, rx, ry, la, lb, lc, ld;
		if (s > 0.0001f) {
			s = ABS(pa * pd - pb * pc) / s;
			pb = pc * s;
			pd = pa * s;
			prx = ATAN2(pc, pa) * RAD_DEG;
		} else {
			pa = 0;
			pc = 0;
			prx = 90 - ATAN2(pd, pb) * RAD_DEG;
		}
		rx = rotation + shearX - prx;
		ry = rotation + shearY - prx + 90;
		la = COS_DEG(rx) * scaleX;
		lb = COS_DEG(ry) * scaleY;
		lc = SIN_DEG(rx) * scaleX;
		ld = SIN_DEG(ry) * scaleY;
		CONST_CAST(float, self->a) = pa * la - pb * lc;
		CONST_CAST(float, self->b) = pa * lb - pb * ld;
		CONST_CAST(float, self->c) = pc * la + pd * lc;
		CONST_CAST(float, self->d) = pc * lb + pd * ld;
		break;
	}
	case SP_TRANSFORMMODE_NOSCALE:
	case SP_TRANSFORMMODE_NOSCALEORREFLECTION: {
		float za, zc, s;
		float r, zb, zd, la, lb, lc, ld;
		cosine = COS_DEG(rotation); sine = SIN_DEG(rotation);
		za = (pa * cosine + pb * sine) / sx;
		zc = (pc * cosine + pd * sine) / sy;
		s = SQRT(za * za + zc * zc);
		if (s > 0.00001f) s = 1 / s;
		za *= s;
		zc *= s;
		s = SQRT(za * za + zc * zc);
		if (self->data->transformMode == SP_TRANSFORMMODE_NOSCALE && (pa * pd - pb * pc < 0) != (sx < 0 != sy < 0))
			s = -s;
		r = PI / 2 + ATAN2(zc, za);
		zb = COS(r) * s;
		zd = SIN(r) * s;
		la = COS_DEG(shearX) * scaleX;
		lb = COS_DEG(90 + shearY) * scaleY;
		lc = SIN_DEG(shearX) * scaleX;
		ld = SIN_DEG(90 + shearY) * scaleY;
		CONST_CAST(float, self->a) = za * la + zb * lc;
		CONST_CAST(float, self->b) = za * lb + zb * ld;
		CONST_CAST(float, self->c) = zc * la + zd * lc;
		CONST_CAST(float, self->d) = zc * lb + zd * ld;
		break;
	}
	}

	CONST_CAST(float, self->a) *= sx;
	CONST_CAST(float, self->b) *= sx;
	CONST_CAST(float, self->c) *= sy;
	CONST_CAST(float, self->d) *= sy;
}

void spBone_setToSetupPose (spBone* self) {
	self->x = self->data->x;
	self->y = self->data->y;
	self->rotation = self->data->rotation;
	self->scaleX = self->data->scaleX;
	self->scaleY = self->data->scaleY;
	self->shearX = self->data->shearX;
	self->shearY = self->data->shearY;
}

float spBone_getWorldRotationX (spBone* self) {
	return ATAN2(self->c, self->a) * RAD_DEG;
}

float spBone_getWorldRotationY (spBone* self) {
	return ATAN2(self->d, self->b) * RAD_DEG;
}

float spBone_getWorldScaleX (spBone* self) {
	return SQRT(self->a * self->a + self->c * self->c);
}

float spBone_getWorldScaleY (spBone* self) {
	return SQRT(self->b * self->b + self->d * self->d);
}

/** Computes the individual applied transform values from the world transform. This can be useful to perform processing using
 * the applied transform after the world transform has been modified directly (eg, by a constraint).
 * <p>
 * Some information is ambiguous in the world transform, such as -1,-1 scale versus 180 rotation. */
void spBone_updateAppliedTransform (spBone* self) {
	spBone* parent = self->parent;
	self->appliedValid = 1;
	if (!parent) {
		self->ax = self->worldX;
		self->ay = self->worldY;
		self->arotation = ATAN2(self->c, self->a) * RAD_DEG;
		self->ascaleX = SQRT(self->a * self->a + self->c * self->c);
		self->ascaleY = SQRT(self->b * self->b + self->d * self->d);
		self->ashearX = 0;
		self->ashearY = ATAN2(self->a * self->b + self->c * self->d, self->a * self->d - self->b * self->c) * RAD_DEG;
	} else {
		float pa = parent->a, pb = parent->b, pc = parent->c, pd = parent->d;
		float pid = 1 / (pa * pd - pb * pc);
		float dx = self->worldX - parent->worldX, dy = self->worldY - parent->worldY;
		float ia = pid * pd;
		float id = pid * pa;
		float ib = pid * pb;
		float ic = pid * pc;
		float ra = ia * self->a - ib * self->c;
		float rb = ia * self->b - ib * self->d;
		float rc = id * self->c - ic * self->a;
		float rd = id * self->d - ic * self->b;
		self->ax = (dx * pd * pid - dy * pb * pid);
		self->ay = (dy * pa * pid - dx * pc * pid);
		self->ashearX = 0;
		self->ascaleX = SQRT(ra * ra + rc * rc);
		if (self->ascaleX > 0.0001f) {
			float det = ra * rd - rb * rc;
			self->ascaleY = det / self->ascaleX;
			self->ashearY = ATAN2(ra * rb + rc * rd, det) * RAD_DEG;
			self->arotation = ATAN2(rc, ra) * RAD_DEG;
		} else {
			self->ascaleX = 0;
			self->ascaleY = SQRT(rb * rb + rd * rd);
			self->ashearY = 0;
			self->arotation = 90 - ATAN2(rd, rb) * RAD_DEG;
		}
	}
}

void spBone_worldToLocal (spBone* self, float worldX, float worldY, float* localX, float* localY) {
	float a = self->a, b = self->b, c = self->c, d = self->d;
	float invDet = 1 / (a * d - b * c);
	float x = worldX - self->worldX, y = worldY - self->worldY;
	*localX = (x * d * invDet - y * b * invDet);
	*localY = (y * a * invDet - x * c * invDet);
}

void spBone_localToWorld (spBone* self, float localX, float localY, float* worldX, float* worldY) {
	float x = localX, y = localY;
	*worldX = x * self->a + y * self->b + self->worldX;
	*worldY = x * self->c + y * self->d + self->worldY;
}

float spBone_worldToLocalRotation (spBone* self, float worldRotation) {
	float sine, cosine;
	sine = SIN_DEG(worldRotation);
	cosine = COS_DEG(worldRotation);
	return ATAN2(self->a * sine - self->c * cosine, self->d * cosine - self->b * sine) * RAD_DEG + self->rotation - self->shearX;
}

float spBone_localToWorldRotation (spBone* self, float localRotation) {
	float sine, cosine;
	localRotation -= self->rotation - self->shearX;
	sine = SIN_DEG(localRotation);
	cosine = COS_DEG(localRotation);
	return ATAN2(cosine * self->c + sine * self->d, cosine * self->a + sine * self->b) * RAD_DEG;
}

void spBone_rotateWorld (spBone* self, float degrees) {
	float a = self->a, b = self->b, c = self->c, d = self->d;
	float cosine = COS_DEG(degrees), sine = SIN_DEG(degrees);
	CONST_CAST(float, self->a) = cosine * a - sine * c;
	CONST_CAST(float, self->b) = cosine * b - sine * d;
	CONST_CAST(float, self->c) = sine * a + cosine * c;
	CONST_CAST(float, self->d) = sine * b + cosine * d;
	CONST_CAST(int, self->appliedValid) = 0;
}
