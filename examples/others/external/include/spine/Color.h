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

#ifndef SPINE_COLOR_H_
#define SPINE_COLOR_H_

#include <spine/dll.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spColor {
	float r, g, b, a;

#ifdef __cplusplus
	spColor() :
		r(0), g(0), b(0), a(0) {
	}

	bool operator==(const spColor& rhs) {
		return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
	}
#endif
} spColor;

/* @param attachmentName May be 0 for no setup pose attachment. */
SP_API spColor* spColor_create();
SP_API void spColor_dispose(spColor* self);
SP_API void spColor_setFromFloats(spColor* color, float r, float g, float b, float a);
SP_API void spColor_setFromColor(spColor* color, spColor* otherColor);
SP_API void spColor_addFloats(spColor* color, float r, float g, float b, float a);
SP_API void spColor_addColor(spColor* color, spColor* otherColor);
SP_API void spColor_clamp(spColor* color);

#ifdef SPINE_SHORT_NAMES
typedef spColor color;
#define Color_create() spColor_create()
#define Color_dispose(...) spColor_dispose(__VA_ARGS__)
#define Color_setFromFloats(...) spColor_setFromFloats(__VA_ARGS__)
#define Color_setFromColor(...) spColor_setFromColor(__VA_ARGS__)
#define Color_addColor(...) spColor_addColor(__VA_ARGS__)
#define Color_addFloats(...) spColor_addFloats(__VA_ARGS__)
#define Color_clamp(...) spColor_clamp(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SPINE_COLOR_H_ */
