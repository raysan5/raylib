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

#include <spine/TransformConstraint.h>
#include <spine/Skeleton.h>
#include <spine/extension.h>

spTransformConstraint* spTransformConstraint_create (spTransformConstraintData* data, const spSkeleton* skeleton) {
	int i;
	spTransformConstraint* self = NEW(spTransformConstraint);
	CONST_CAST(spTransformConstraintData*, self->data) = data;
	self->rotateMix = data->rotateMix;
	self->translateMix = data->translateMix;
	self->scaleMix = data->scaleMix;
	self->shearMix = data->shearMix;
	self->bonesCount = data->bonesCount;
	CONST_CAST(spBone**, self->bones) = MALLOC(spBone*, self->bonesCount);
	for (i = 0; i < self->bonesCount; ++i)
		self->bones[i] = spSkeleton_findBone(skeleton, self->data->bones[i]->name);
	self->target = spSkeleton_findBone(skeleton, self->data->target->name);
	return self;
}

void spTransformConstraint_dispose (spTransformConstraint* self) {
	FREE(self->bones);
	FREE(self);
}

void _spTransformConstraint_applyAbsoluteWorld (spTransformConstraint* self) {
	float rotateMix = self->rotateMix, translateMix = self->translateMix, scaleMix = self->scaleMix, shearMix = self->shearMix;
	spBone* target = self->target;
	float ta = target->a, tb = target->b, tc = target->c, td = target->d;
	float degRadReflect = ta * td - tb * tc > 0 ? DEG_RAD : -DEG_RAD;
	float offsetRotation = self->data->offsetRotation * degRadReflect, offsetShearY = self->data->offsetShearY * degRadReflect;
	int /*bool*/ modified;
	int i;
	float a, b, c, d, r, cosine, sine, x, y, s, ts, by;
	for (i = 0; i < self->bonesCount; ++i) {
		spBone* bone = self->bones[i];
		modified = 0;

		if (rotateMix != 0) {
			a = bone->a, b = bone->b, c = bone->c, d = bone->d;
			r = ATAN2(tc, ta) - ATAN2(c, a) + offsetRotation;
			if (r > PI) r -= PI2;
			else if (r < -PI) r += PI2;
			r *= rotateMix;
			cosine = COS(r);
			sine = SIN(r);
			CONST_CAST(float, bone->a) = cosine * a - sine * c;
			CONST_CAST(float, bone->b) = cosine * b - sine * d;
			CONST_CAST(float, bone->c) = sine * a + cosine * c;
			CONST_CAST(float, bone->d) = sine * b + cosine * d;
			modified = 1;
		}

		if (translateMix != 0) {
			spBone_localToWorld(target, self->data->offsetX, self->data->offsetY, &x, &y);
			CONST_CAST(float, bone->worldX) += (x - bone->worldX) * translateMix;
			CONST_CAST(float, bone->worldY) += (y - bone->worldY) * translateMix;
			modified = 1;
		}

		if (scaleMix > 0) {
			s = SQRT(bone->a * bone->a + bone->c * bone->c);
			ts = SQRT(ta * ta + tc * tc);
			if (s > 0.00001f) s = (s + (ts - s + self->data->offsetScaleX) * scaleMix) / s;
			CONST_CAST(float, bone->a) *= s;
			CONST_CAST(float, bone->c) *= s;
			s = SQRT(bone->b * bone->b + bone->d * bone->d);
			ts = SQRT(tb * tb + td * td);
			if (s > 0.00001f) s = (s + (ts - s + self->data->offsetScaleY) * scaleMix) / s;
			CONST_CAST(float, bone->b) *= s;
			CONST_CAST(float, bone->d) *= s;
			modified = 1;
		}

		if (shearMix > 0) {
			b = bone->b, d = bone->d;
			by = ATAN2(d, b);
			r = ATAN2(td, tb) - ATAN2(tc, ta) - (by - ATAN2(bone->c, bone->a));
			s = SQRT(b * b + d * d);
			if (r > PI) r -= PI2;
			else if (r < -PI) r += PI2;
			r = by + (r + offsetShearY) * shearMix;
			CONST_CAST(float, bone->b) = COS(r) * s;
			CONST_CAST(float, bone->d) = SIN(r) * s;
			modified = 1;
		}

		if (modified) CONST_CAST(int, bone->appliedValid) = 0;
	}
}

void _spTransformConstraint_applyRelativeWorld (spTransformConstraint* self) {
	float rotateMix = self->rotateMix, translateMix = self->translateMix, scaleMix = self->scaleMix, shearMix = self->shearMix;
	spBone* target = self->target;
	float ta = target->a, tb = target->b, tc = target->c, td = target->d;
	float degRadReflect = ta * td - tb * tc > 0 ? DEG_RAD : -DEG_RAD;
	float offsetRotation = self->data->offsetRotation * degRadReflect, offsetShearY = self->data->offsetShearY * degRadReflect;
	int /*bool*/ modified;
	int i;
	float a, b, c, d, r, cosine, sine, x, y, s;
	for (i = 0; i < self->bonesCount; ++i) {
		spBone* bone = self->bones[i];
		modified = 0;

		if (rotateMix != 0) {
			a = bone->a, b = bone->b, c = bone->c, d = bone->d;
			r = ATAN2(tc, ta) + offsetRotation;
			if (r > PI) r -= PI2;
			else if (r < -PI) r += PI2;
			r *= rotateMix;
			cosine = COS(r);
			sine = SIN(r);
			CONST_CAST(float, bone->a) = cosine * a - sine * c;
			CONST_CAST(float, bone->b) = cosine * b - sine * d;
			CONST_CAST(float, bone->c) = sine * a + cosine * c;
			CONST_CAST(float, bone->d) = sine * b + cosine * d;
			modified = 1;
		}

		if (translateMix != 0) {
			spBone_localToWorld(target, self->data->offsetX, self->data->offsetY, &x, &y);
			CONST_CAST(float, bone->worldX) += (x * translateMix);
			CONST_CAST(float, bone->worldY) += (y * translateMix);
			modified = 1;
		}

		if (scaleMix > 0) {
			s = (SQRT(ta * ta + tc * tc) - 1 + self->data->offsetScaleX) * scaleMix + 1;
			CONST_CAST(float, bone->a) *= s;
			CONST_CAST(float, bone->c) *= s;
			s = (SQRT(tb * tb + td * td) - 1 + self->data->offsetScaleY) * scaleMix + 1;
			CONST_CAST(float, bone->b) *= s;
			CONST_CAST(float, bone->d) *= s;
			modified = 1;
		}

		if (shearMix > 0) {
			r = ATAN2(td, tb) - ATAN2(tc, ta);
			if (r > PI) r -= PI2;
			else if (r < -PI) r += PI2;
			b = bone->b, d = bone->d;
			r = ATAN2(d, b) + (r - PI / 2 + offsetShearY) * shearMix;
			s = SQRT(b * b + d * d);
			CONST_CAST(float, bone->b) = COS(r) * s;
			CONST_CAST(float, bone->d) = SIN(r) * s;
			modified = 1;
		}

		if (modified) CONST_CAST(int, bone->appliedValid) = 0;
	}
}

void _spTransformConstraint_applyAbsoluteLocal (spTransformConstraint* self) {
	float rotateMix = self->rotateMix, translateMix = self->translateMix, scaleMix = self->scaleMix, shearMix = self->shearMix;
	spBone* target = self->target;
	int i;
	float rotation, r, x, y, scaleX, scaleY, shearY;

	if (!target->appliedValid) spBone_updateAppliedTransform(target);
	for (i = 0; i < self->bonesCount; ++i) {
		spBone* bone = self->bones[i];
		if (!bone->appliedValid) spBone_updateAppliedTransform(bone);

		rotation = bone->arotation;
		if (rotateMix != 0) {
			r = target->arotation - rotation + self->data->offsetRotation;
			r -= (16384 - (int)(16384.499999999996 - r / 360)) * 360;
			rotation += r * rotateMix;
		}

		x = bone->ax, y = bone->ay;
		if (translateMix != 0) {
			x += (target->ax - x + self->data->offsetX) * translateMix;
			y += (target->ay - y + self->data->offsetY) * translateMix;
		}

		scaleX = bone->ascaleX, scaleY = bone->ascaleY;
		if (scaleMix != 0) {
			if (scaleX > 0.00001) scaleX = (scaleX + (target->ascaleX - scaleX + self->data->offsetScaleX) * scaleMix) / scaleX;
			if (scaleY > 0.00001) scaleY = (scaleY + (target->ascaleY - scaleY + self->data->offsetScaleY) * scaleMix) / scaleY;
		}

		shearY = bone->ashearY;
		if (shearMix != 0) {
			r = target->ashearY - shearY + self->data->offsetShearY;
			r -= (16384 - (int)(16384.499999999996 - r / 360)) * 360;
			bone->shearY += r * shearMix;
		}

		spBone_updateWorldTransformWith(bone, x, y, rotation, scaleX, scaleY, bone->ashearX, shearY);
	}
}

void _spTransformConstraint_applyRelativeLocal (spTransformConstraint* self) {
	float rotateMix = self->rotateMix, translateMix = self->translateMix, scaleMix = self->scaleMix, shearMix = self->shearMix;
	spBone* target = self->target;
	int i;
	float rotation, x, y, scaleX, scaleY, shearY;

	if (!target->appliedValid) spBone_updateAppliedTransform(target);

	for (i = 0; i < self->bonesCount; ++i) {
		spBone* bone = self->bones[i];
		if (!bone->appliedValid) spBone_updateAppliedTransform(bone);

		rotation = bone->arotation;
		if (rotateMix != 0) rotation += (target->arotation + self->data->offsetRotation) * rotateMix;

		x = bone->ax;
		y = bone->ay;
		if (translateMix != 0) {
			x += (target->ax + self->data->offsetX) * translateMix;
			y += (target->ay + self->data->offsetY) * translateMix;
		}

		scaleX = bone->ascaleX;
		scaleY = bone->ascaleY;
		if (scaleMix != 0) {
			if (scaleX > 0.00001f) scaleX *= ((target->ascaleX - 1 + self->data->offsetScaleX) * scaleMix) + 1;
			if (scaleY > 0.00001f) scaleY *= ((target->ascaleY - 1 + self->data->offsetScaleY) * scaleMix) + 1;
		}

		shearY = bone->ashearY;
		if (shearMix != 0) shearY += (target->ashearY + self->data->offsetShearY) * shearMix;

		spBone_updateWorldTransformWith(bone, x, y, rotation, scaleX, scaleY, bone->ashearX, shearY);
	}
}

void spTransformConstraint_apply (spTransformConstraint* self) {
	if (self->data->local) {
		if (self->data->relative)
			_spTransformConstraint_applyRelativeLocal(self);
		else
			_spTransformConstraint_applyAbsoluteLocal(self);

	} else {
		if (self->data->relative)
			_spTransformConstraint_applyRelativeWorld(self);
		else
			_spTransformConstraint_applyAbsoluteWorld(self);
	}
}
