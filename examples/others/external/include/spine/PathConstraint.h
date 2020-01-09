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

#ifndef SPINE_PATHCONSTRAINT_H_
#define SPINE_PATHCONSTRAINT_H_

#include <spine/dll.h>
#include <spine/PathConstraintData.h>
#include <spine/Bone.h>
#include <spine/Slot.h>
#include "PathAttachment.h"

#ifdef __cplusplus
extern "C" {
#endif

struct spSkeleton;

typedef struct spPathConstraint {
	spPathConstraintData* const data;
	int bonesCount;
	spBone** const bones;
	spSlot* target;
	float position, spacing, rotateMix, translateMix;

	int spacesCount;
	float* spaces;

	int positionsCount;
	float* positions;

	int worldCount;
	float* world;

	int curvesCount;
	float* curves;

	int lengthsCount;
	float* lengths;

	float segments[10];

	int /*boolean*/ active;

#ifdef __cplusplus
	spPathConstraint() :
		data(0),
		bonesCount(0),
		bones(0),
		target(0),
		position(0),
		spacing(0),
		rotateMix(0),
		translateMix(0),
		spacesCount(0),
		spaces(0),
		positionsCount(0),
		positions(0),
		worldCount(0),
		world(0),
		curvesCount(0),
		curves(0),
		lengthsCount(0),
		lengths(0),
		active(0) {
	}
#endif
} spPathConstraint;

#define SP_PATHCONSTRAINT_

SP_API spPathConstraint* spPathConstraint_create (spPathConstraintData* data, const struct spSkeleton* skeleton);
SP_API void spPathConstraint_dispose (spPathConstraint* self);

SP_API void spPathConstraint_apply (spPathConstraint* self);
SP_API float* spPathConstraint_computeWorldPositions(spPathConstraint* self, spPathAttachment* path, int spacesCount, int/*bool*/ tangents, int/*bool*/percentPosition, int/**/percentSpacing);

#ifdef SPINE_SHORT_NAMES
typedef spPathConstraint PathConstraint;
#define PathConstraint_create(...) spPathConstraint_create(__VA_ARGS__)
#define PathConstraint_dispose(...) spPathConstraint_dispose(__VA_ARGS__)
#define PathConstraint_apply(...) spPathConstraint_apply(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SPINE_PATHCONSTRAINT_H_ */
