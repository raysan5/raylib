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

#include <spine/Atlas.h>
#include <ctype.h>
#include <spine/extension.h>

spAtlasPage* spAtlasPage_create(spAtlas* atlas, const char* name) {
	spAtlasPage* self = NEW(spAtlasPage);
	CONST_CAST(spAtlas*, self->atlas) = atlas;
	MALLOC_STR(self->name, name);
	return self;
}

void spAtlasPage_dispose(spAtlasPage* self) {
	_spAtlasPage_disposeTexture(self);
	FREE(self->name);
	FREE(self);
}

/**/

spAtlasRegion* spAtlasRegion_create() {
	return NEW(spAtlasRegion);
}

void spAtlasRegion_dispose(spAtlasRegion* self) {
	FREE(self->name);
	FREE(self->splits);
	FREE(self->pads);
	FREE(self);
}

/**/

typedef struct {
	const char* begin;
	const char* end;
} Str;

static void trim(Str* str) {
	while (isspace((unsigned char)*str->begin) && str->begin < str->end)
		(str->begin)++;
	if (str->begin == str->end) return;
	str->end--;
	while (((unsigned char)*str->end == '\r') && str->end >= str->begin)
		str->end--;
	str->end++;
}

/* Tokenize string without modification. Returns 0 on failure. */
static int readLine(const char** begin, const char* end, Str* str) {
	if (*begin == end) return 0;
	str->begin = *begin;

	/* Find next delimiter. */
	while (*begin != end && **begin != '\n')
		(*begin)++;

	str->end = *begin;
	trim(str);

	if (*begin != end) (*begin)++;
	return 1;
}

/* Moves str->begin past the first occurence of c. Returns 0 on failure. */
static int beginPast(Str* str, char c) {
	const char* begin = str->begin;
	while (1) {
		char lastSkippedChar = *begin;
		if (begin == str->end) return 0;
		begin++;
		if (lastSkippedChar == c) break;
	}
	str->begin = begin;
	return 1;
}

/* Returns 0 on failure. */
static int readValue(const char** begin, const char* end, Str* str) {
	readLine(begin, end, str);
	if (!beginPast(str, ':')) return 0;
	trim(str);
	return 1;
}

/* Returns the number of tuple values read (1, 2, 4, or 0 for failure). */
static int readTuple(const char** begin, const char* end, Str tuple[]) {
	int i;
	Str str = { NULL, NULL };
	readLine(begin, end, &str);
	if (!beginPast(&str, ':')) return 0;

	for (i = 0; i < 3; ++i) {
		tuple[i].begin = str.begin;
		if (!beginPast(&str, ',')) break;
		tuple[i].end = str.begin - 2;
		trim(&tuple[i]);
	}
	tuple[i].begin = str.begin;
	tuple[i].end = str.end;
	trim(&tuple[i]);
	return i + 1;
}

static char* mallocString(Str* str) {
	int length = (int)(str->end - str->begin);
	char* string = MALLOC(char, length + 1);
	memcpy(string, str->begin, length);
	string[length] = '\0';
	return string;
}

static int indexOf(const char** array, int count, Str* str) {
	int length = (int)(str->end - str->begin);
	int i;
	for (i = count - 1; i >= 0; i--)
		if (strncmp(array[i], str->begin, length) == 0) return i;
	return 0;
}

static int equals(Str* str, const char* other) {
	return strncmp(other, str->begin, str->end - str->begin) == 0;
}

static int toInt(Str* str) {
	return (int)strtol(str->begin, (char**)&str->end, 10);
}

static spAtlas* abortAtlas(spAtlas* self) {
	spAtlas_dispose(self);
	return 0;
}

static const char* formatNames[] = { "", "Alpha", "Intensity", "LuminanceAlpha", "RGB565", "RGBA4444", "RGB888", "RGBA8888" };
static const char* textureFilterNames[] = { "", "Nearest", "Linear", "MipMap", "MipMapNearestNearest", "MipMapLinearNearest",
"MipMapNearestLinear", "MipMapLinearLinear" };

spAtlas* spAtlas_create(const char* begin, int length, const char* dir, void* rendererObject) {
	spAtlas* self;

	int count;
	const char* end = begin + length;
	int dirLength = (int)strlen(dir);
	int needsSlash = dirLength > 0 && dir[dirLength - 1] != '/' && dir[dirLength - 1] != '\\';

	spAtlasPage *page = 0;
	spAtlasPage *lastPage = 0;
	spAtlasRegion *lastRegion = 0;
	Str str;
	Str tuple[4];

	self = NEW(spAtlas);
	self->rendererObject = rendererObject;

	while (readLine(&begin, end, &str)) {
		if (str.end - str.begin == 0)
			page = 0;
		else if (!page) {
			char* name = mallocString(&str);
			char* path = MALLOC(char, dirLength + needsSlash + strlen(name) + 1);
			memcpy(path, dir, dirLength);
			if (needsSlash) path[dirLength] = '/';
			strcpy(path + dirLength + needsSlash, name);

			page = spAtlasPage_create(self, name);
			FREE(name);
			if (lastPage)
				lastPage->next = page;
			else
				self->pages = page;
			lastPage = page;

			switch (readTuple(&begin, end, tuple)) {
			case 0:
				return abortAtlas(self);
			case 2: /* size is only optional for an atlas packed with an old TexturePacker. */
				page->width = toInt(tuple);
				page->height = toInt(tuple + 1);
				if (!readTuple(&begin, end, tuple)) return abortAtlas(self);
			}
			page->format = (spAtlasFormat)indexOf(formatNames, 8, tuple);

			if (!readTuple(&begin, end, tuple)) return abortAtlas(self);
			page->minFilter = (spAtlasFilter)indexOf(textureFilterNames, 8, tuple);
			page->magFilter = (spAtlasFilter)indexOf(textureFilterNames, 8, tuple + 1);

			if (!readValue(&begin, end, &str)) return abortAtlas(self);

			page->uWrap = SP_ATLAS_CLAMPTOEDGE;
			page->vWrap = SP_ATLAS_CLAMPTOEDGE;
			if (!equals(&str, "none")) {
				if (str.end - str.begin == 1) {
					if (*str.begin == 'x')
						page->uWrap = SP_ATLAS_REPEAT;
					else if (*str.begin == 'y')
						page->vWrap = SP_ATLAS_REPEAT;
				}
				else if (equals(&str, "xy")) {
					page->uWrap = SP_ATLAS_REPEAT;
					page->vWrap = SP_ATLAS_REPEAT;
				}
			}

			_spAtlasPage_createTexture(page, path);
			FREE(path);
		} else {
			spAtlasRegion *region = spAtlasRegion_create();
			if (lastRegion)
				lastRegion->next = region;
			else
				self->regions = region;
			lastRegion = region;

			region->page = page;
			region->name = mallocString(&str);

			if (!readValue(&begin, end, &str)) return abortAtlas(self);
			if (equals(&str, "true"))
				region->degrees = 90;
			else if (equals(&str, "false"))
				region->degrees = 0;
			else
				region->degrees = toInt(&str);
			region->rotate = region->degrees == 90;

			if (readTuple(&begin, end, tuple) != 2) return abortAtlas(self);
			region->x = toInt(tuple);
			region->y = toInt(tuple + 1);

			if (readTuple(&begin, end, tuple) != 2) return abortAtlas(self);
			region->width = toInt(tuple);
			region->height = toInt(tuple + 1);

			region->u = region->x / (float)page->width;
			region->v = region->y / (float)page->height;
			if (region->rotate) {
				region->u2 = (region->x + region->height) / (float)page->width;
				region->v2 = (region->y + region->width) / (float)page->height;
			} else {
				region->u2 = (region->x + region->width) / (float)page->width;
				region->v2 = (region->y + region->height) / (float)page->height;
			}

			count = readTuple(&begin, end, tuple);
			if (!count) return abortAtlas(self);
			if (count == 4) { /* split is optional */
				region->splits = MALLOC(int, 4);
				region->splits[0] = toInt(tuple);
				region->splits[1] = toInt(tuple + 1);
				region->splits[2] = toInt(tuple + 2);
				region->splits[3] = toInt(tuple + 3);

				count = readTuple(&begin, end, tuple);
				if (!count) return abortAtlas(self);
				if (count == 4) { /* pad is optional, but only present with splits */
					region->pads = MALLOC(int, 4);
					region->pads[0] = toInt(tuple);
					region->pads[1] = toInt(tuple + 1);
					region->pads[2] = toInt(tuple + 2);
					region->pads[3] = toInt(tuple + 3);

					if (!readTuple(&begin, end, tuple)) return abortAtlas(self);
				}
			}

			region->originalWidth = toInt(tuple);
			region->originalHeight = toInt(tuple + 1);

			readTuple(&begin, end, tuple);
			region->offsetX = toInt(tuple);
			region->offsetY = toInt(tuple + 1);

			if (!readValue(&begin, end, &str)) return abortAtlas(self);
			region->index = toInt(&str);
		}
	}

	return self;
}

spAtlas* spAtlas_createFromFile(const char* path, void* rendererObject) {
	int dirLength;
	char *dir;
	int length;
	const char* data;

	spAtlas* atlas = 0;

	/* Get directory from atlas path. */
	const char* lastForwardSlash = strrchr(path, '/');
	const char* lastBackwardSlash = strrchr(path, '\\');
	const char* lastSlash = lastForwardSlash > lastBackwardSlash ? lastForwardSlash : lastBackwardSlash;
	if (lastSlash == path) lastSlash++; /* Never drop starting slash. */
	dirLength = (int)(lastSlash ? lastSlash - path : 0);
	dir = MALLOC(char, dirLength + 1);
	memcpy(dir, path, dirLength);
	dir[dirLength] = '\0';

	data = _spUtil_readFile(path, &length);
	if (data) atlas = spAtlas_create(data, length, dir, rendererObject);

	FREE(data);
	FREE(dir);
	return atlas;
}

void spAtlas_dispose(spAtlas* self) {
	spAtlasRegion* region, *nextRegion;
	spAtlasPage* page = self->pages;
	while (page) {
		spAtlasPage* nextPage = page->next;
		spAtlasPage_dispose(page);
		page = nextPage;
	}

	region = self->regions;
	while (region) {
		nextRegion = region->next;
		spAtlasRegion_dispose(region);
		region = nextRegion;
	}

	FREE(self);
}

spAtlasRegion* spAtlas_findRegion(const spAtlas* self, const char* name) {
	spAtlasRegion* region = self->regions;
	while (region) {
		if (strcmp(region->name, name) == 0) return region;
		region = region->next;
	}
	return 0;
}
