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

#include <spine/VertexEffect.h>
#include <spine/extension.h>

void _spJitterVertexEffect_begin(spVertexEffect* self, spSkeleton* skeleton) {
	UNUSED(self);
	UNUSED(skeleton);
}

void _spJitterVertexEffect_transform(spVertexEffect* self, float* x, float* y, float* u, float* v, spColor* light, spColor* dark) {
	spJitterVertexEffect* internal = (spJitterVertexEffect*)self;
	float jitterX = internal->jitterX;
	float jitterY = internal->jitterY;
	(*x) += _spMath_randomTriangular(-jitterX, jitterY);
	(*y) += _spMath_randomTriangular(-jitterX, jitterY);
	UNUSED(u);
	UNUSED(v);
	UNUSED(light);
	UNUSED(dark);
}

void _spJitterVertexEffect_end(spVertexEffect* self) {
	UNUSED(self);
}

spJitterVertexEffect* spJitterVertexEffect_create(float jitterX, float jitterY) {
	spJitterVertexEffect* effect = CALLOC(spJitterVertexEffect, 1);
	effect->super.begin = _spJitterVertexEffect_begin;
	effect->super.transform = _spJitterVertexEffect_transform;
	effect->super.end = _spJitterVertexEffect_end;
	effect->jitterX = jitterX;
	effect->jitterY = jitterY;
	return effect;
}

void spJitterVertexEffect_dispose(spJitterVertexEffect* effect) {
	FREE(effect);
}

void _spSwirlVertexEffect_begin(spVertexEffect* self, spSkeleton* skeleton) {
	spSwirlVertexEffect* internal = (spSwirlVertexEffect*)self;
	internal->worldX = skeleton->x + internal->centerX;
	internal->worldY = skeleton->y + internal->centerY;
}

void _spSwirlVertexEffect_transform(spVertexEffect* self, float* positionX, float* positionY, float* u, float* v, spColor* light, spColor* dark) {
	spSwirlVertexEffect* internal = (spSwirlVertexEffect*)self;
	float radAngle = internal->angle * DEG_RAD;
	float x = *positionX - internal->worldX;
	float y = *positionY - internal->worldY;
	float dist = SQRT(x * x + y * y);
	if (dist < internal->radius) {
		float theta = _spMath_interpolate(_spMath_pow2_apply, 0, radAngle, (internal->radius - dist) / internal->radius);
		float cosine = COS(theta);
		float sine = SIN(theta);
		(*positionX) = cosine * x - sine * y + internal->worldX;
		(*positionY) = sine * x + cosine * y + internal->worldY;
	}
	UNUSED(self);
	UNUSED(u);
	UNUSED(v);
	UNUSED(light);
	UNUSED(dark);
}

void _spSwirlVertexEffect_end(spVertexEffect* self) {
	UNUSED(self);
}

spSwirlVertexEffect* spSwirlVertexEffect_create(float radius) {
	spSwirlVertexEffect* effect = CALLOC(spSwirlVertexEffect, 1);
	effect->super.begin = _spSwirlVertexEffect_begin;
	effect->super.transform = _spSwirlVertexEffect_transform;
	effect->super.end = _spSwirlVertexEffect_end;
	effect->radius = radius;
	return effect;
}

void spSwirlVertexEffect_dispose(spSwirlVertexEffect* effect) {
	FREE(effect);
}
