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

#include <spine/PathConstraint.h>
#include <spine/Skeleton.h>
#include <spine/extension.h>

#define PATHCONSTRAINT_NONE -1
#define PATHCONSTRAINT_BEFORE -2
#define PATHCONSTRAINT_AFTER -3
#define EPSILON 0.00001f

spPathConstraint* spPathConstraint_create (spPathConstraintData* data, const spSkeleton* skeleton) {
	int i;
	spPathConstraint *self = NEW(spPathConstraint);
	CONST_CAST(spPathConstraintData*, self->data) = data;
	self->bonesCount = data->bonesCount;
	CONST_CAST(spBone**, self->bones) = MALLOC(spBone*, self->bonesCount);
	for (i = 0; i < self->bonesCount; ++i)
		self->bones[i] = spSkeleton_findBone(skeleton, self->data->bones[i]->name);
	self->target = spSkeleton_findSlot(skeleton, self->data->target->name);
	self->position = data->position;
	self->spacing = data->spacing;
	self->rotateMix = data->rotateMix;
	self->translateMix = data->translateMix;
	self->spacesCount = 0;
	self->spaces = 0;
	self->positionsCount = 0;
	self->positions = 0;
	self->worldCount = 0;
	self->world = 0;
	self->curvesCount = 0;
	self->curves = 0;
	self->lengthsCount = 0;
	self->lengths = 0;
	return self;
}

void spPathConstraint_dispose (spPathConstraint* self) {
	FREE(self->bones);
	FREE(self->spaces);
	if (self->positions) FREE(self->positions);
	if (self->world) FREE(self->world);
	if (self->curves) FREE(self->curves);
	if (self->lengths) FREE(self->lengths);
	FREE(self);
}

void spPathConstraint_apply (spPathConstraint* self) {
	int i, p, n;
	float length, setupLength, x, y, dx, dy, s;
	float* spaces, *lengths, *positions;
	float spacing;
	float boneX, boneY, offsetRotation;
	int/*bool*/tip;
	float rotateMix = self->rotateMix, translateMix = self->translateMix;
	int/*bool*/ translate = translateMix > 0, rotate = rotateMix > 0;
	int lengthSpacing;
	spPathAttachment* attachment = (spPathAttachment*)self->target->attachment;
	spPathConstraintData* data = self->data;
	int percentSpacing = data->spacingMode == SP_SPACING_MODE_PERCENT;
	spRotateMode rotateMode = data->rotateMode;
	int tangents = rotateMode == SP_ROTATE_MODE_TANGENT, scale = rotateMode == SP_ROTATE_MODE_CHAIN_SCALE;
	int boneCount = self->bonesCount, spacesCount = tangents ? boneCount : boneCount + 1;
	spBone** bones = self->bones;
	spBone* pa;

	if (!translate && !rotate) return;
	if ((attachment == 0) || (attachment->super.super.type != SP_ATTACHMENT_PATH)) return;

	if (self->spacesCount != spacesCount) {
		if (self->spaces) FREE(self->spaces);
		self->spaces = MALLOC(float, spacesCount);
		self->spacesCount = spacesCount;
	}
	spaces = self->spaces;
	spaces[0] = 0;
	lengths = 0;
	spacing = self->spacing;
	if (scale || !percentSpacing) {
		if (scale) {
			if (self->lengthsCount != boneCount) {
				if (self->lengths) FREE(self->lengths);
				self->lengths = MALLOC(float, boneCount);
				self->lengthsCount = boneCount;
			}
			lengths = self->lengths;
		}
		lengthSpacing = data->spacingMode == SP_SPACING_MODE_LENGTH;
		for (i = 0, n = spacesCount - 1; i < n;) {
			spBone *bone = bones[i];
			setupLength = bone->data->length;
			if (setupLength < EPSILON) {
				if (scale) lengths[i] = 0;
				spaces[++i] = 0;
			} else if (percentSpacing) {
				if (scale) {
					x = setupLength * bone->a, y = setupLength * bone->c;
					length = SQRT(x * x + y * y);
					lengths[i] = length;
				}
				spaces[++i] = spacing;
			} else {
				x = setupLength * bone->a, y = setupLength * bone->c;
				length = SQRT(x * x + y * y);
				if (scale) lengths[i] = length;
				spaces[++i] = (lengthSpacing ? setupLength + spacing : spacing) * length / setupLength;
			}
		}
	} else {
		for (i = 1; i < spacesCount; i++) {
			spaces[i] = spacing;
		}
	}

	positions = spPathConstraint_computeWorldPositions(self, attachment, spacesCount, tangents,
		data->positionMode == SP_POSITION_MODE_PERCENT, percentSpacing);
	boneX = positions[0], boneY = positions[1], offsetRotation = self->data->offsetRotation;
	tip = 0;
	if (offsetRotation == 0)
		tip = rotateMode == SP_ROTATE_MODE_CHAIN;
	else {
		tip = 0;
		pa = self->target->bone;
		offsetRotation *= pa->a * pa->d - pa->b * pa->c > 0 ? DEG_RAD : -DEG_RAD;
	}
	for (i = 0, p = 3; i < boneCount; i++, p += 3) {
		spBone* bone = bones[i];
		CONST_CAST(float, bone->worldX) += (boneX - bone->worldX) * translateMix;
		CONST_CAST(float, bone->worldY) += (boneY - bone->worldY) * translateMix;
		x = positions[p], y = positions[p + 1], dx = x - boneX, dy = y - boneY;
		if (scale) {
			length = lengths[i];
			if (length != 0) {
				s = (SQRT(dx * dx + dy * dy) / length - 1) * rotateMix + 1;
				CONST_CAST(float, bone->a) *= s;
				CONST_CAST(float, bone->c) *= s;
			}
		}
		boneX = x;
		boneY = y;
		if (rotate) {
			float a = bone->a, b = bone->b, c = bone->c, d = bone->d, r, cosine, sine;
			if (tangents)
				r = positions[p - 1];
			else if (spaces[i + 1] == 0)
				r = positions[p + 2];
			else
				r = ATAN2(dy, dx);
			r -= ATAN2(c, a) - offsetRotation * DEG_RAD;
			if (tip) {
				cosine = COS(r);
				sine = SIN(r);
				length = bone->data->length;
				boneX += (length * (cosine * a - sine * c) - dx) * rotateMix;
				boneY += (length * (sine * a + cosine * c) - dy) * rotateMix;
			} else
				r += offsetRotation;
			if (r > PI)
				r -= PI2;
			else if (r < -PI)
				r += PI2;
			r *= rotateMix;
			cosine = COS(r);
			sine = SIN(r);
			CONST_CAST(float, bone->a) = cosine * a - sine * c;
			CONST_CAST(float, bone->b) = cosine * b - sine * d;
			CONST_CAST(float, bone->c) = sine * a + cosine * c;
			CONST_CAST(float, bone->d) = sine * b + cosine * d;
		}
		CONST_CAST(int, bone->appliedValid) = -1;
	}
}

static void _addBeforePosition(float p, float* temp, int i, float* out, int o) {
	float x1 = temp[i], y1 = temp[i + 1], dx = temp[i + 2] - x1, dy = temp[i + 3] - y1, r = ATAN2(dy, dx);
	out[o] = x1 + p * COS(r);
	out[o + 1] = y1 + p * SIN(r);
	out[o + 2] = r;
}

static void _addAfterPosition (float p, float* temp, int i, float* out, int o) {
	float x1 = temp[i + 2], y1 = temp[i + 3], dx = x1 - temp[i], dy = y1 - temp[i + 1], r = ATAN2(dy, dx);
	out[o] = x1 + p * COS(r);
	out[o + 1] = y1 + p * SIN(r);
	out[o + 2] = r;
}

/* Need to pass 0 as an argument, so VC++ doesn't error with C2124 */
static int _isNan(float value, float zero) {
	float _nan = (float)0.0 / zero;
	return 0 == memcmp((void*)&value, (void*)&_nan, sizeof(value));
}

static void _addCurvePosition (float p, float x1, float y1, float cx1, float cy1, float cx2, float cy2, float x2, float y2,
		float* out, int o, int/*bool*/tangents) {
	float tt, ttt, u, uu, uuu;
	float ut, ut3, uut3, utt3;
	float x, y;
	if (p == 0 || _isNan(p, 0)) {
		out[o] = x1;
		out[o + 1] = y1;
		out[o + 2] = ATAN2(cy1 - y1, cx1 - x1);
		return;
	}
	tt = p * p, ttt = tt * p, u = 1 - p, uu = u * u, uuu = uu * u;
	ut = u * p, ut3 = ut * 3, uut3 = u * ut3, utt3 = ut3 * p;
	x = x1 * uuu + cx1 * uut3 + cx2 * utt3 + x2 * ttt, y = y1 * uuu + cy1 * uut3 + cy2 * utt3 + y2 * ttt;
	out[o] = x;
	out[o + 1] = y;
	if (tangents) {
		if (p < 0.001)
			out[o + 2] = ATAN2(cy1 - y1, cx1 - x1);
		else
			out[o + 2] = ATAN2(y - (y1 * uu + cy1 * ut * 2 + cy2 * tt), x - (x1 * uu + cx1 * ut * 2 + cx2 * tt));
	}
}

float* spPathConstraint_computeWorldPositions(spPathConstraint* self, spPathAttachment* path, int spacesCount, int/*bool*/ tangents, int/*bool*/percentPosition, int/**/percentSpacing) {
	int i, o, w, curve, segment, /*bool*/closed, verticesLength, curveCount, prevCurve;
	float* out, *curves, *segments;
	float tmpx, tmpy, dddfx, dddfy, ddfx, ddfy, dfx, dfy, pathLength, curveLength, p;
	float x1, y1, cx1, cy1, cx2, cy2, x2, y2;
	spSlot* target = self->target;
	float position = self->position;
	float* spaces = self->spaces, *world = 0;
	if (self->positionsCount != spacesCount * 3 + 2) {
		if (self->positions) FREE(self->positions);
		self->positions = MALLOC(float, spacesCount * 3 + 2);
		self->positionsCount = spacesCount * 3 + 2;
	}
	out = self->positions;
	closed = path->closed;
	verticesLength = path->super.worldVerticesLength, curveCount = verticesLength / 6, prevCurve = PATHCONSTRAINT_NONE;

	if (!path->constantSpeed) {
		float* lengths = path->lengths;
		curveCount -= closed ? 1 : 2;
		pathLength = lengths[curveCount];
		if (percentPosition) position *= pathLength;
		if (percentSpacing) {
			for (i = 1; i < spacesCount; i++)
				spaces[i] *= pathLength;
		}
		if (self->worldCount != 8) {
			if (self->world) FREE(self->world);
			self->world = MALLOC(float, 8);
			self->worldCount = 8;
		}
		world = self->world;
		for (i = 0, o = 0, curve = 0; i < spacesCount; i++, o += 3) {
			float space = spaces[i];
			position += space;
			p = position;

			if (closed) {
				p = FMOD(p, pathLength);
				if (p < 0) p += pathLength;
				curve = 0;
			} else if (p < 0) {
				if (prevCurve != PATHCONSTRAINT_BEFORE) {
					prevCurve = PATHCONSTRAINT_BEFORE;
					spVertexAttachment_computeWorldVertices(SUPER(path), target, 2, 4, world, 0, 2);
				}
				_addBeforePosition(p, world, 0, out, o);
				continue;
			} else if (p > pathLength) {
				if (prevCurve != PATHCONSTRAINT_AFTER) {
					prevCurve = PATHCONSTRAINT_AFTER;
					spVertexAttachment_computeWorldVertices(SUPER(path), target, verticesLength - 6, 4, world, 0, 2);
				}
				_addAfterPosition(p - pathLength, world, 0, out, o);
				continue;
			}

			/* Determine curve containing position. */
			for (;; curve++) {
				float length = lengths[curve];
				if (p > length) continue;
				if (curve == 0)
					p /= length;
				else {
					float prev = lengths[curve - 1];
					p = (p - prev) / (length - prev);
				}
				break;
			}
			if (curve != prevCurve) {
				prevCurve = curve;
				if (closed && curve == curveCount) {
					spVertexAttachment_computeWorldVertices(SUPER(path), target, verticesLength - 4, 4, world, 0, 2);
					spVertexAttachment_computeWorldVertices(SUPER(path), target, 0, 4, world, 4, 2);
				} else
					spVertexAttachment_computeWorldVertices(SUPER(path), target, curve * 6 + 2, 8, world, 0, 2);
			}
			_addCurvePosition(p, world[0], world[1], world[2], world[3], world[4], world[5], world[6], world[7], out, o,
				tangents || (i > 0 && space == 0));
		}
		return out;
	}

	/* World vertices. */
	if (closed) {
		verticesLength += 2;
		if (self->worldCount != verticesLength) {
			if (self->world) FREE(self->world);
			self->world = MALLOC(float, verticesLength);
			self->worldCount = verticesLength;
		}
		world = self->world;
		spVertexAttachment_computeWorldVertices(SUPER(path), target, 2, verticesLength - 4, world, 0, 2);
		spVertexAttachment_computeWorldVertices(SUPER(path), target, 0, 2, world, verticesLength - 4, 2);
		world[verticesLength - 2] = world[0];
		world[verticesLength - 1] = world[1];
	} else {
		curveCount--;
		verticesLength -= 4;
		if (self->worldCount != verticesLength) {
			if (self->world) FREE(self->world);
			self->world = MALLOC(float, verticesLength);
			self->worldCount = verticesLength;
		}
		world = self->world;
		spVertexAttachment_computeWorldVertices(SUPER(path), target, 2, verticesLength, world, 0, 2);
	}

	/* Curve lengths. */
	if (self->curvesCount != curveCount) {
		if (self->curves) FREE(self->curves);
		self->curves = MALLOC(float, curveCount);
		self->curvesCount = curveCount;
	}
	curves = self->curves;
	pathLength = 0;
	x1 = world[0], y1 = world[1], cx1 = 0, cy1 = 0, cx2 = 0, cy2 = 0, x2 = 0, y2 = 0;
	for (i = 0, w = 2; i < curveCount; i++, w += 6) {
		cx1 = world[w];
		cy1 = world[w + 1];
		cx2 = world[w + 2];
		cy2 = world[w + 3];
		x2 = world[w + 4];
		y2 = world[w + 5];
		tmpx = (x1 - cx1 * 2 + cx2) * 0.1875f;
		tmpy = (y1 - cy1 * 2 + cy2) * 0.1875f;
		dddfx = ((cx1 - cx2) * 3 - x1 + x2) * 0.09375f;
		dddfy = ((cy1 - cy2) * 3 - y1 + y2) * 0.09375f;
		ddfx = tmpx * 2 + dddfx;
		ddfy = tmpy * 2 + dddfy;
		dfx = (cx1 - x1) * 0.75f + tmpx + dddfx * 0.16666667f;
		dfy = (cy1 - y1) * 0.75f + tmpy + dddfy * 0.16666667f;
		pathLength += SQRT(dfx * dfx + dfy * dfy);
		dfx += ddfx;
		dfy += ddfy;
		ddfx += dddfx;
		ddfy += dddfy;
		pathLength += SQRT(dfx * dfx + dfy * dfy);
		dfx += ddfx;
		dfy += ddfy;
		pathLength += SQRT(dfx * dfx + dfy * dfy);
		dfx += ddfx + dddfx;
		dfy += ddfy + dddfy;
		pathLength += SQRT(dfx * dfx + dfy * dfy);
		curves[i] = pathLength;
		x1 = x2;
		y1 = y2;
	}
	if (percentPosition)
		position *= pathLength;
	else
		position *= pathLength / path->lengths[curveCount - 1];
	if (percentSpacing) {
		for (i = 1; i < spacesCount; i++)
			spaces[i] *= pathLength;
	}

	segments = self->segments;
	curveLength = 0;
	for (i = 0, o = 0, curve = 0, segment = 0; i < spacesCount; i++, o += 3) {
		float space = spaces[i];
		position += space;
		p = position;

		if (closed) {
			p = FMOD(p, pathLength);
			if (p < 0) p += pathLength;
			curve = 0;
		} else if (p < 0) {
			_addBeforePosition(p, world, 0, out, o);
			continue;
		} else if (p > pathLength) {
			_addAfterPosition(p - pathLength, world, verticesLength - 4, out, o);
			continue;
		}

		/* Determine curve containing position. */
		for (;; curve++) {
			float length = curves[curve];
			if (p > length) continue;
			if (curve == 0)
				p /= length;
			else {
				float prev = curves[curve - 1];
				p = (p - prev) / (length - prev);
			}
			break;
		}

		/* Curve segment lengths. */
		if (curve != prevCurve) {
			int ii;
			prevCurve = curve;
			ii = curve * 6;
			x1 = world[ii];
			y1 = world[ii + 1];
			cx1 = world[ii + 2];
			cy1 = world[ii + 3];
			cx2 = world[ii + 4];
			cy2 = world[ii + 5];
			x2 = world[ii + 6];
			y2 = world[ii + 7];
			tmpx = (x1 - cx1 * 2 + cx2) * 0.03f;
			tmpy = (y1 - cy1 * 2 + cy2) * 0.03f;
			dddfx = ((cx1 - cx2) * 3 - x1 + x2) * 0.006f;
			dddfy = ((cy1 - cy2) * 3 - y1 + y2) * 0.006f;
			ddfx = tmpx * 2 + dddfx;
			ddfy = tmpy * 2 + dddfy;
			dfx = (cx1 - x1) * 0.3f + tmpx + dddfx * 0.16666667f;
			dfy = (cy1 - y1) * 0.3f + tmpy + dddfy * 0.16666667f;
			curveLength = SQRT(dfx * dfx + dfy * dfy);
			segments[0] = curveLength;
			for (ii = 1; ii < 8; ii++) {
				dfx += ddfx;
				dfy += ddfy;
				ddfx += dddfx;
				ddfy += dddfy;
				curveLength += SQRT(dfx * dfx + dfy * dfy);
				segments[ii] = curveLength;
			}
			dfx += ddfx;
			dfy += ddfy;
			curveLength += SQRT(dfx * dfx + dfy * dfy);
			segments[8] = curveLength;
			dfx += ddfx + dddfx;
			dfy += ddfy + dddfy;
			curveLength += SQRT(dfx * dfx + dfy * dfy);
			segments[9] = curveLength;
			segment = 0;
		}

		/* Weight by segment length. */
		p *= curveLength;
		for (;; segment++) {
			float length = segments[segment];
			if (p > length) continue;
			if (segment == 0)
				p /= length;
			else {
				float prev = segments[segment - 1];
				p = segment + (p - prev) / (length - prev);
			}
			break;
		}
		_addCurvePosition(p * 0.1f, x1, y1, cx1, cy1, cx2, cy2, x2, y2, out, o, tangents || (i > 0 && space == 0));
	}
	return out;
}
