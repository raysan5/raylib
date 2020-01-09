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

#ifndef SPINE_VERTEXEFFECT_H_
#define SPINE_VERTEXEFFECT_H_

#include <spine/dll.h>
#include <spine/Skeleton.h>
#include <spine/Color.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spVertexEffect;

typedef void (*spVertexEffectBegin)(struct spVertexEffect *self, spSkeleton *skeleton);

typedef void (*spVertexEffectTransform)(struct spVertexEffect *self, float *x, float *y, float *u, float *v,
	spColor *light, spColor *dark);

typedef void (*spVertexEffectEnd)(struct spVertexEffect *self);

typedef struct spVertexEffect {
	spVertexEffectBegin begin;
	spVertexEffectTransform transform;
	spVertexEffectEnd end;
} spVertexEffect;

typedef struct spJitterVertexEffect {
	spVertexEffect super;
	float jitterX;
	float jitterY;
} spJitterVertexEffect;

typedef struct spSwirlVertexEffect {
	spVertexEffect super;
	float centerX;
	float centerY;
	float radius;
	float angle;
	float worldX;
	float worldY;
} spSwirlVertexEffect;

SP_API spJitterVertexEffect *spJitterVertexEffect_create(float jitterX, float jitterY);

SP_API void spJitterVertexEffect_dispose(spJitterVertexEffect *effect);

SP_API spSwirlVertexEffect *spSwirlVertexEffect_create(float radius);

SP_API void spSwirlVertexEffect_dispose(spSwirlVertexEffect *effect);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_VERTEX_EFFECT_H_ */
