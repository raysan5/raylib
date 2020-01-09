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

#ifndef SPINE_ARRAY_H
#define SPINE_ARRAY_H

#include <spine/dll.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _SP_ARRAY_DECLARE_TYPE(name, itemType) \
	typedef struct name { int size; int capacity; itemType* items; } name; \
	SP_API name* name##_create(int initialCapacity); \
	SP_API void name##_dispose(name* self); \
	SP_API void name##_clear(name* self); \
	SP_API name* name##_setSize(name* self, int newSize); \
	SP_API void name##_ensureCapacity(name* self, int newCapacity); \
	SP_API void name##_add(name* self, itemType value); \
	SP_API void name##_addAll(name* self, name* other); \
	SP_API void name##_addAllValues(name* self, itemType* values, int offset, int count); \
	SP_API void name##_removeAt(name* self, int index); \
	SP_API int name##_contains(name* self, itemType value); \
	SP_API itemType name##_pop(name* self); \
	SP_API itemType name##_peek(name* self);

#define _SP_ARRAY_IMPLEMENT_TYPE(name, itemType) \
	name* name##_create(int initialCapacity) { \
		name* array = CALLOC(name, 1); \
		array->size = 0; \
		array->capacity = initialCapacity; \
		array->items = CALLOC(itemType, initialCapacity); \
		return array; \
	} \
	void name##_dispose(name* self) { \
		FREE(self->items); \
		FREE(self); \
	} \
	void name##_clear(name* self) { \
		self->size = 0; \
	} \
	name* name##_setSize(name* self, int newSize) { \
		self->size = newSize; \
		if (self->capacity < newSize) { \
			self->capacity = MAX(8, (int)(self->size * 1.75f)); \
			self->items = REALLOC(self->items, itemType, self->capacity); \
		} \
		return self; \
	} \
	void name##_ensureCapacity(name* self, int newCapacity) { \
		if (self->capacity >= newCapacity) return; \
		self->capacity = newCapacity; \
		self->items = REALLOC(self->items, itemType, self->capacity); \
	} \
	void name##_add(name* self, itemType value) { \
		if (self->size == self->capacity) { \
			self->capacity = MAX(8, (int)(self->size * 1.75f)); \
			self->items = REALLOC(self->items, itemType, self->capacity); \
		} \
		self->items[self->size++] = value; \
	} \
	void name##_addAll(name* self, name* other) { \
		int i = 0; \
		for (; i < other->size; i++) { \
			name##_add(self, other->items[i]); \
		} \
	} \
	void name##_addAllValues(name* self, itemType* values, int offset, int count) { \
		int i = offset, n = offset + count; \
		for (; i < n; i++) { \
			name##_add(self, values[i]); \
		} \
	} \
	void name##_removeAt(name* self, int index) { \
		self->size--; \
		memmove(self->items + index, self->items + index + 1, sizeof(itemType) * (self->size - index)); \
	} \
	int name##_contains(name* self, itemType value) { \
		itemType* items = self->items; \
		int i, n; \
		for (i = 0, n = self->size; i < n; i++) { \
			if (items[i] == value) return -1; \
		} \
		return 0; \
	} \
	itemType name##_pop(name* self) { \
		itemType item = self->items[--self->size]; \
		return item; \
	} \
	itemType name##_peek(name* self) { \
		return self->items[self->size - 1]; \
	}

_SP_ARRAY_DECLARE_TYPE(spFloatArray, float)
_SP_ARRAY_DECLARE_TYPE(spIntArray, int)
_SP_ARRAY_DECLARE_TYPE(spShortArray, short)
_SP_ARRAY_DECLARE_TYPE(spUnsignedShortArray, unsigned short)
_SP_ARRAY_DECLARE_TYPE(spArrayFloatArray, spFloatArray*)
_SP_ARRAY_DECLARE_TYPE(spArrayShortArray, spShortArray*)

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ARRAY_H */
