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

#ifndef SPINE_IKCONSTRAINT_H_
#define SPINE_IKCONSTRAINT_H_

#include <spine/dll.h>
#include <spine/IkConstraintData.h>
#include <spine/Bone.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spSkeleton;

typedef struct spIkConstraint {
	spIkConstraintData* const data;

	int bonesCount;
	spBone** bones;

	spBone* target;
	int bendDirection;
	int /*boolean*/ compress;
	int /*boolean*/ stretch;
	float mix;
	float softness;

	int /*boolean*/ active;

#ifdef __cplusplus
	spIkConstraint() :
		data(0),
		bonesCount(0),
		bones(0),
		target(0),
		bendDirection(0),
		stretch(0),
		mix(0),
		softness(0),
		active(0) {
	}
#endif
} spIkConstraint;

SP_API spIkConstraint* spIkConstraint_create (spIkConstraintData* data, const struct spSkeleton* skeleton);
SP_API void spIkConstraint_dispose (spIkConstraint* self);

SP_API void spIkConstraint_apply (spIkConstraint* self);

SP_API void spIkConstraint_apply1 (spBone* bone, float targetX, float targetY, int /*boolean*/ compress, int /*boolean*/ stretch, int /*boolean*/ uniform, float alpha);
SP_API void spIkConstraint_apply2 (spBone* parent, spBone* child, float targetX, float targetY, int bendDirection, int /*boolean*/ stretch, float softness, float alpha);

#ifdef SPINE_SHORT_NAMES
typedef spIkConstraint IkConstraint;
#define IkConstraint_create(...) spIkConstraint_create(__VA_ARGS__)
#define IkConstraint_dispose(...) spIkConstraint_dispose(__VA_ARGS__)
#define IkConstraint_apply(...) spIkConstraint_apply(__VA_ARGS__)
#define IkConstraint_apply1(...) spIkConstraint_apply1(__VA_ARGS__)
#define IkConstraint_apply2(...) spIkConstraint_apply2(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SPINE_IKCONSTRAINT_H_ */
