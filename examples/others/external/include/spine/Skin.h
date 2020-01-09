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

#ifndef SPINE_SKIN_H_
#define SPINE_SKIN_H_

#include <spine/dll.h>
#include <spine/Attachment.h>
#include <spine/IkConstraintData.h>
#include <spine/TransformConstraintData.h>
#include <spine/PathConstraintData.h>
#include <spine/Array.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Size of hashtable used in skin structure for fast attachment lookup. */
#define SKIN_ENTRIES_HASH_TABLE_SIZE 100

struct spSkeleton;

_SP_ARRAY_DECLARE_TYPE(spBoneDataArray, spBoneData*)
_SP_ARRAY_DECLARE_TYPE(spIkConstraintDataArray, spIkConstraintData*)
_SP_ARRAY_DECLARE_TYPE(spTransformConstraintDataArray, spTransformConstraintData*)
_SP_ARRAY_DECLARE_TYPE(spPathConstraintDataArray, spPathConstraintData*)

typedef struct spSkin {
	const char* const name;

	spBoneDataArray* bones;
	spIkConstraintDataArray* ikConstraints;
	spTransformConstraintDataArray* transformConstraints;
	spPathConstraintDataArray* pathConstraints;

#ifdef __cplusplus
	spSkin() :
		name(0),
		bones(0),
		ikConstraints(0),
		transformConstraints(0),
		pathConstraints(0) {
	}
#endif
} spSkin;

/* Private structs, needed by Skeleton */
typedef struct _Entry _Entry;
typedef struct _Entry spSkinEntry;
struct _Entry {
	int slotIndex;
	const char* name;
	spAttachment* attachment;
	_Entry* next;
};

typedef struct _SkinHashTableEntry _SkinHashTableEntry;
struct _SkinHashTableEntry {
	_Entry* entry;
	_SkinHashTableEntry* next;
};

typedef struct {
	spSkin super;
	_Entry* entries; /* entries list stored for getting attachment name by attachment index */
	_SkinHashTableEntry* entriesHashTable[SKIN_ENTRIES_HASH_TABLE_SIZE]; /* hashtable for fast attachment lookup */
} _spSkin;

SP_API spSkin* spSkin_create (const char* name);
SP_API void spSkin_dispose (spSkin* self);

/* The Skin owns the attachment. */
SP_API void spSkin_setAttachment (spSkin* self, int slotIndex, const char* name, spAttachment* attachment);
/* Returns 0 if the attachment was not found. */
SP_API spAttachment* spSkin_getAttachment (const spSkin* self, int slotIndex, const char* name);

/* Returns 0 if the slot or attachment was not found. */
SP_API const char* spSkin_getAttachmentName (const spSkin* self, int slotIndex, int attachmentIndex);

/** Attach each attachment in this skin if the corresponding attachment in oldSkin is currently attached. */
SP_API void spSkin_attachAll (const spSkin* self, struct spSkeleton* skeleton, const spSkin* oldspSkin);

/** Adds all attachments, bones, and constraints from the specified skin to this skin. */
SP_API void spSkin_addSkin(spSkin* self, const spSkin* other);

/** Adds all attachments, bones, and constraints from the specified skin to this skin. Attachments are deep copied. */
SP_API void spSkin_copySkin(spSkin* self, const spSkin* other);

/** Returns all attachments in this skin. */
SP_API spSkinEntry* spSkin_getAttachments(const spSkin* self);

/** Clears all attachments, bones, and constraints. */
SP_API void spSkin_clear(spSkin* self);

#ifdef SPINE_SHORT_NAMES
typedef spSkin Skin;
#define Skin_create(...) spSkin_create(__VA_ARGS__)
#define Skin_dispose(...) spSkin_dispose(__VA_ARGS__)
#define Skin_setAttachment(...) spSkin_addAttachment(__VA_ARGS__)
#define Skin_getAttachment(...) spSkin_getAttachment(__VA_ARGS__)
#define Skin_getAttachmentName(...) spSkin_getAttachmentName(__VA_ARGS__)
#define Skin_attachAll(...) spSkin_attachAll(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SKIN_H_ */
