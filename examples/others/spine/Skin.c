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

#include <spine/Skin.h>
#include <spine/extension.h>
#include <stdio.h>

_SP_ARRAY_IMPLEMENT_TYPE(spBoneDataArray, spBoneData*)
_SP_ARRAY_IMPLEMENT_TYPE(spIkConstraintDataArray, spIkConstraintData*)
_SP_ARRAY_IMPLEMENT_TYPE(spTransformConstraintDataArray, spTransformConstraintData*)
_SP_ARRAY_IMPLEMENT_TYPE(spPathConstraintDataArray, spPathConstraintData*)

_Entry* _Entry_create (int slotIndex, const char* name, spAttachment* attachment) {
	_Entry* self = NEW(_Entry);
	self->slotIndex = slotIndex;
	MALLOC_STR(self->name, name);
	self->attachment = attachment;
	return self;
}

void _Entry_dispose (_Entry* self) {
	spAttachment_dispose(self->attachment);
	FREE(self->name);
	FREE(self);
}

static _SkinHashTableEntry* _SkinHashTableEntry_create (_Entry* entry) {
	_SkinHashTableEntry* self = NEW(_SkinHashTableEntry);
	self->entry = entry;
	return self;
}

static void _SkinHashTableEntry_dispose (_SkinHashTableEntry* self) {
	FREE(self);
}

/**/

spSkin* spSkin_create (const char* name) {
	spSkin* self = SUPER(NEW(_spSkin));
	MALLOC_STR(self->name, name);
	self->bones = spBoneDataArray_create(4);
	self->ikConstraints = spIkConstraintDataArray_create(4);
	self->transformConstraints = spTransformConstraintDataArray_create(4);
	self->pathConstraints = spPathConstraintDataArray_create(4);
	return self;
}

void spSkin_dispose (spSkin* self) {
	_Entry* entry = SUB_CAST(_spSkin, self)->entries;

	while (entry) {
		_Entry* nextEntry = entry->next;
		_Entry_dispose(entry);
		entry = nextEntry;
	}

	{
		_SkinHashTableEntry** currentHashtableEntry = SUB_CAST(_spSkin, self)->entriesHashTable;
		int i;

		for (i = 0; i < SKIN_ENTRIES_HASH_TABLE_SIZE; ++i, ++currentHashtableEntry) {
			_SkinHashTableEntry* hashtableEntry = *currentHashtableEntry;

			while (hashtableEntry) {
				_SkinHashTableEntry* nextEntry = hashtableEntry->next;
				_SkinHashTableEntry_dispose(hashtableEntry);
				hashtableEntry = nextEntry;
			}
		}
	}

	spBoneDataArray_dispose(self->bones);
	spIkConstraintDataArray_dispose(self->ikConstraints);
	spTransformConstraintDataArray_dispose(self->transformConstraints);
	spPathConstraintDataArray_dispose(self->pathConstraints);
	FREE(self->name);
	FREE(self);
}

void spSkin_setAttachment (spSkin* self, int slotIndex, const char* name, spAttachment* attachment) {
	_SkinHashTableEntry* existingEntry = 0;
	_SkinHashTableEntry* hashEntry = SUB_CAST(_spSkin, self)->entriesHashTable[(unsigned int)slotIndex % SKIN_ENTRIES_HASH_TABLE_SIZE];
	while (hashEntry) {
		if (hashEntry->entry->slotIndex == slotIndex && strcmp(hashEntry->entry->name, name) == 0) {
			existingEntry = hashEntry;
			break;
		}
		hashEntry = hashEntry->next;
	}

	if (attachment) attachment->refCount++;

	if (existingEntry) {
		if (hashEntry->entry->attachment) spAttachment_dispose(hashEntry->entry->attachment);
		hashEntry->entry->attachment = attachment;
	} else {
		_Entry* newEntry = _Entry_create(slotIndex, name, attachment);
		newEntry->next = SUB_CAST(_spSkin, self)->entries;
		SUB_CAST(_spSkin, self)->entries = newEntry;
		{
			unsigned int hashTableIndex = (unsigned int)slotIndex % SKIN_ENTRIES_HASH_TABLE_SIZE;
			_SkinHashTableEntry** hashTable = SUB_CAST(_spSkin, self)->entriesHashTable;

			_SkinHashTableEntry* newHashEntry = _SkinHashTableEntry_create(newEntry);
			newHashEntry->next = hashTable[hashTableIndex];
			SUB_CAST(_spSkin, self)->entriesHashTable[hashTableIndex] = newHashEntry;
		}
	}
}

spAttachment* spSkin_getAttachment (const spSkin* self, int slotIndex, const char* name) {
	const _SkinHashTableEntry* hashEntry = SUB_CAST(_spSkin, self)->entriesHashTable[(unsigned int)slotIndex % SKIN_ENTRIES_HASH_TABLE_SIZE];
	while (hashEntry) {
		if (hashEntry->entry->slotIndex == slotIndex && strcmp(hashEntry->entry->name, name) == 0) return hashEntry->entry->attachment;
		hashEntry = hashEntry->next;
	}
	return 0;
}

const char* spSkin_getAttachmentName (const spSkin* self, int slotIndex, int attachmentIndex) {
	const _Entry* entry = SUB_CAST(_spSkin, self)->entries;
	int i = 0;
	while (entry) {
		if (entry->slotIndex == slotIndex) {
			if (i == attachmentIndex) return entry->name;
			i++;
		}
		entry = entry->next;
	}
	return 0;
}

void spSkin_attachAll (const spSkin* self, spSkeleton* skeleton, const spSkin* oldSkin) {
	const _Entry *entry = SUB_CAST(_spSkin, oldSkin)->entries;
	while (entry) {
		spSlot *slot = skeleton->slots[entry->slotIndex];
		if (slot->attachment == entry->attachment) {
			spAttachment *attachment = spSkin_getAttachment(self, entry->slotIndex, entry->name);
			if (attachment) spSlot_setAttachment(slot, attachment);
		}
		entry = entry->next;
	}
}

void spSkin_addSkin(spSkin* self, const spSkin* other) {
	int i = 0;
	spSkinEntry* entry;

	for (i = 0; i < other->bones->size; i++) {
		if (!spBoneDataArray_contains(self->bones, other->bones->items[i]))
			spBoneDataArray_add(self->bones, other->bones->items[i]);
	}

	for (i = 0; i < other->ikConstraints->size; i++) {
		if (!spIkConstraintDataArray_contains(self->ikConstraints, other->ikConstraints->items[i]))
			spIkConstraintDataArray_add(self->ikConstraints, other->ikConstraints->items[i]);
	}

	for (i = 0; i < other->transformConstraints->size; i++) {
		if (!spTransformConstraintDataArray_contains(self->transformConstraints, other->transformConstraints->items[i]))
			spTransformConstraintDataArray_add(self->transformConstraints, other->transformConstraints->items[i]);
	}

	for (i = 0; i < other->pathConstraints->size; i++) {
		if (!spPathConstraintDataArray_contains(self->pathConstraints, other->pathConstraints->items[i]))
			spPathConstraintDataArray_add(self->pathConstraints, other->pathConstraints->items[i]);
	}

	entry = spSkin_getAttachments(other);
	while (entry) {
		spSkin_setAttachment(self, entry->slotIndex, entry->name, entry->attachment);
		entry = entry->next;
	}
}

void spSkin_copySkin(spSkin* self, const spSkin* other) {
	int i = 0;
	spSkinEntry* entry;

	for (i = 0; i < other->bones->size; i++) {
		if (!spBoneDataArray_contains(self->bones, other->bones->items[i]))
			spBoneDataArray_add(self->bones, other->bones->items[i]);
	}

	for (i = 0; i < other->ikConstraints->size; i++) {
		if (!spIkConstraintDataArray_contains(self->ikConstraints, other->ikConstraints->items[i]))
			spIkConstraintDataArray_add(self->ikConstraints, other->ikConstraints->items[i]);
	}

	for (i = 0; i < other->transformConstraints->size; i++) {
		if (!spTransformConstraintDataArray_contains(self->transformConstraints, other->transformConstraints->items[i]))
			spTransformConstraintDataArray_add(self->transformConstraints, other->transformConstraints->items[i]);
	}

	for (i = 0; i < other->pathConstraints->size; i++) {
		if (!spPathConstraintDataArray_contains(self->pathConstraints, other->pathConstraints->items[i]))
			spPathConstraintDataArray_add(self->pathConstraints, other->pathConstraints->items[i]);
	}

	entry = spSkin_getAttachments(other);
	while (entry) {
		if (entry->attachment->type == SP_ATTACHMENT_MESH) {
			spMeshAttachment* attachment = spMeshAttachment_newLinkedMesh(SUB_CAST(spMeshAttachment, entry->attachment));
			spSkin_setAttachment(self, entry->slotIndex, entry->name, SUPER(SUPER(attachment)));
		} else {
			spAttachment* attachment = entry->attachment ? spAttachment_copy(entry->attachment) : 0;
			spSkin_setAttachment(self, entry->slotIndex, entry->name, attachment);
		}
		entry = entry->next;
	}
}

spSkinEntry* spSkin_getAttachments(const spSkin* self) {
	return SUB_CAST(_spSkin, self)->entries;
}

void spSkin_clear(spSkin* self) {
	_Entry* entry = SUB_CAST(_spSkin, self)->entries;

	while (entry) {
		_Entry* nextEntry = entry->next;
		_Entry_dispose(entry);
		entry = nextEntry;
	}

	SUB_CAST(_spSkin, self)->entries = 0;

	{
		_SkinHashTableEntry** currentHashtableEntry = SUB_CAST(_spSkin, self)->entriesHashTable;
		int i;

		for (i = 0; i < SKIN_ENTRIES_HASH_TABLE_SIZE; ++i, ++currentHashtableEntry) {
			_SkinHashTableEntry* hashtableEntry = *currentHashtableEntry;

			while (hashtableEntry) {
				_SkinHashTableEntry* nextEntry = hashtableEntry->next;
				_SkinHashTableEntry_dispose(hashtableEntry);
				hashtableEntry = nextEntry;
			}

			SUB_CAST(_spSkin, self)->entriesHashTable[i] = 0;
		}
	}

	spBoneDataArray_clear(self->bones);
	spIkConstraintDataArray_clear(self->ikConstraints);
	spTransformConstraintDataArray_clear(self->transformConstraints);
	spPathConstraintDataArray_clear(self->pathConstraints);
}
