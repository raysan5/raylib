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

#include <spine/extension.h>
#include <stdio.h>
#include <raylib.h>
#include <rlgl.h>

#define MAX_TEXTURES 10
static Texture2D tm_textures[MAX_TEXTURES] = {0};
static int texture_index = 0;

typedef struct Vertex {
    // Position in x/y plane
    float x, y;

    // UV coordinates
    float u, v;

    // Color, each channel in the range from 0-1
    // (Should really be a 32-bit RGBA packed color)
    float r, g, b, a;
} Vertex;

#define MAX_VERTICES_PER_ATTACHMENT 2048
float worldVerticesPositions[MAX_VERTICES_PER_ATTACHMENT];

Vertex vertices[MAX_VERTICES_PER_ATTACHMENT];

void addVertex(float x, float y, float u, float v, float r, float g, float b, float a, int* index) {
    Vertex* vertex = &vertices[*index];
    vertex->x = x;
    vertex->y = y;
    vertex->u = u;
    vertex->v = v;
    vertex->r = r;
    vertex->g = g;
    vertex->b = b;
    vertex->a = a;
    *index += 1;
}

void draw_vertex(Vertex vertex, Vector3 position){
    rlTexCoord2f(vertex.u, vertex.v);
    rlColor4f(vertex.r, vertex.g, vertex.b, vertex.a);
    rlVertex3f( position.x + vertex.x, position.y + vertex.y, position.z);
}

void engine_drawMesh(Vertex* vertices, int numVertices, Texture* texture, Vector3 position){
    Vertex vertex;

    rlEnableTexture(texture->id);

    rlPushMatrix();
    {
        rlBegin(RL_QUADS);
        {
            rlNormal3f(0.0f, 0.0f, 1.0f);
            for (int i = 0; i < numVertices; i++){
                if(i < 3 || i == 4){
                    vertex = vertices[i];
                    draw_vertex(vertex, position);
                }
            }
        }rlEnd();
    }rlPopMatrix();
    rlDisableTexture();
}

Texture2D* texture_2d_create(char *path) {
    tm_textures[texture_index] = LoadTexture(path);
    Texture2D *t = &tm_textures[texture_index];
    texture_index++;
    return t;
}

void texture_2d_destroy() {
    while(texture_index--) UnloadTexture(tm_textures[texture_index]);
}

float _spInternalRandom () {
	return rand() / (float)RAND_MAX;
}

static void* (*mallocFunc) (size_t size) = malloc;
static void* (*reallocFunc) (void* ptr, size_t size) = realloc;
static void* (*debugMallocFunc) (size_t size, const char* file, int line) = NULL;
static void (*freeFunc) (void* ptr) = free;
static float (*randomFunc) () = _spInternalRandom;

void* _spMalloc (size_t size, const char* file, int line) {
	if(debugMallocFunc)
		return debugMallocFunc(size, file, line);

	return mallocFunc(size);
}
void* _spCalloc (size_t num, size_t size, const char* file, int line) {
	void* ptr = _spMalloc(num * size, file, line);
	if (ptr) memset(ptr, 0, num * size);
	return ptr;
}
void* _spRealloc(void* ptr, size_t size) {
	return reallocFunc(ptr, size);
}
void _spFree (void* ptr) {
	freeFunc(ptr);
}

float _spRandom () {
	return randomFunc();
}

void _spSetDebugMalloc(void* (*malloc) (size_t size, const char* file, int line)) {
	debugMallocFunc = malloc;
}

void _spSetMalloc (void* (*malloc) (size_t size)) {
	mallocFunc = malloc;
}

void _spSetRealloc (void* (*realloc) (void* ptr, size_t size)) {
	reallocFunc = realloc;
}

void _spSetFree (void (*free) (void* ptr)) {
	freeFunc = free;
}

void _spSetRandom (float (*random) ()) {
	randomFunc = random;
}

char* _spReadFile (const char* path, int* length) {
	char *data;
	FILE *file = fopen(path, "rb");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	*length = (int)ftell(file);
	fseek(file, 0, SEEK_SET);

	data = MALLOC(char, *length);
	fread(data, 1, *length, file);
	fclose(file);

	return data;
}

char* _spUtil_readFile(const char* path, int* length) {
    return _spReadFile(path, length);
}
void _spAtlasPage_createTexture (spAtlasPage* self, const char* path) {

    Texture2D* t = texture_2d_create((char *)path);

    self->rendererObject = t;
    self->width = t->width;
    self->height = t->height;
}

#define MAX_VERTICES_PER_ATTACHMENT 2048
float worldVerticesPositions[MAX_VERTICES_PER_ATTACHMENT];
Vertex vertices[MAX_VERTICES_PER_ATTACHMENT];

void drawSkeleton(spSkeleton* skeleton, Vector3 position) {
    // For each slot in the draw order array of the skeleton
    for (int i = 0; i < skeleton->slotsCount; ++i) {
        spSlot* slot = skeleton->drawOrder[i];

        // Fetch the currently active attachment, continue
        // with the next slot in the draw order if no
        // attachment is active on the slot
        spAttachment* attachment = slot->attachment;
        if (!attachment) continue;

        // Calculate the tinting color based on the skeleton's color
        // and the slot's color. Each color channel is given in the
        // range [0-1], you may have to multiply by 255 and cast to
        // and int if your engine uses integer ranges for color channels.
        float tintR = skeleton->color.r * slot->color.r;
        float tintG = skeleton->color.g * slot->color.g;
        float tintB = skeleton->color.b * slot->color.b;
        float tintA = skeleton->color.a * slot->color.a;

        // Fill the vertices array depending on the type of attachment
        Texture* texture = 0;
        int vertexIndex = 0;
        if (attachment->type == SP_ATTACHMENT_REGION) {
            // Cast to an spRegionAttachment so we can get the rendererObject
            // and compute the world vertices
            spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;

            // Our engine specific Texture is stored in the spAtlasRegion which was
            // assigned to the attachment on load. It represents the texture atlas
            // page that contains the image the region attachment is mapped to
            texture = (Texture*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;

            // Computed the world vertices positions for the 4 vertices that make up
            // the rectangular region attachment. This assumes the world transform of the
            // bone to which the slot (and hence attachment) is attached has been calculated
            // before rendering via spSkeleton_updateWorldTransform
            spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, worldVerticesPositions, 0, 2);

            // Create 2 triangles, with 3 vertices each from the region's
            // world vertex positions and its UV coordinates (in the range [0-1]).
            addVertex(worldVerticesPositions[0], worldVerticesPositions[1],
                      regionAttachment->uvs[0], regionAttachment->uvs[1],
                      tintR, tintG, tintB, tintA, &vertexIndex);

            addVertex(worldVerticesPositions[2], worldVerticesPositions[3],
                      regionAttachment->uvs[2], regionAttachment->uvs[3],
                      tintR, tintG, tintB, tintA, &vertexIndex);

            addVertex(worldVerticesPositions[4], worldVerticesPositions[5],
                      regionAttachment->uvs[4], regionAttachment->uvs[5],
                      tintR, tintG, tintB, tintA, &vertexIndex);

            addVertex(worldVerticesPositions[4], worldVerticesPositions[5],
                      regionAttachment->uvs[4], regionAttachment->uvs[5],
                      tintR, tintG, tintB, tintA, &vertexIndex);

            addVertex(worldVerticesPositions[6], worldVerticesPositions[7],
                      regionAttachment->uvs[6], regionAttachment->uvs[7],
                      tintR, tintG, tintB, tintA, &vertexIndex);

            addVertex(worldVerticesPositions[0], worldVerticesPositions[1],
                      regionAttachment->uvs[0], regionAttachment->uvs[1],
                      tintR, tintG, tintB, tintA, &vertexIndex);
        } else if (attachment->type == SP_ATTACHMENT_MESH) {
            // Cast to an spMeshAttachment so we can get the rendererObject
            // and compute the world vertices
            spMeshAttachment* mesh = (spMeshAttachment*)attachment;

            // Check the number of vertices in the mesh attachment. If it is bigger
            // than our scratch buffer, we don't render the mesh. We do this here
            // for simplicity, in production you want to reallocate the scratch buffer
            // to fit the mesh.
            if (mesh->super.worldVerticesLength > MAX_VERTICES_PER_ATTACHMENT) continue;

            // Our engine specific Texture is stored in the spAtlasRegion which was
            // assigned to the attachment on load. It represents the texture atlas
            // page that contains the image the mesh attachment is mapped to
            texture = (Texture*)((spAtlasRegion*)mesh->rendererObject)->page->rendererObject;

            // Computed the world vertices positions for the vertices that make up
            // the mesh attachment. This assumes the world transform of the
            // bone to which the slot (and hence attachment) is attached has been calculated
            // before rendering via spSkeleton_updateWorldTransform
            spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0, mesh->super.worldVerticesLength, worldVerticesPositions, 0, 2);

            // Mesh attachments use an array of vertices, and an array of indices to define which
            // 3 vertices make up each triangle. We loop through all triangle indices
            // and simply emit a vertex for each triangle's vertex.
            for (int i = 0; i < mesh->trianglesCount; ++i) {
                int index = mesh->triangles[i] << 1;
                addVertex(worldVerticesPositions[index], worldVerticesPositions[index + 1],
                          mesh->uvs[index], mesh->uvs[index + 1],
                          tintR, tintG, tintB, tintA, &vertexIndex);
            }

        }
        // Draw the mesh we created for the attachment
        engine_drawMesh(vertices, vertexIndex, texture, position);
    }
}

void _spAtlasPage_disposeTexture (spAtlasPage* self) {
    if (self->rendererObject == NULL) return;
    Texture2D *t2d = self->rendererObject;
    UnloadTexture(*t2d);
}

float _spMath_random(float min, float max) {
	return min + (max - min) * _spRandom();
}

float _spMath_randomTriangular(float min, float max) {
	return _spMath_randomTriangularWith(min, max, (min + max) * 0.5f);
}

float _spMath_randomTriangularWith(float min, float max, float mode) {
	float u = _spRandom();
	float d = max - min;
	if (u <= (mode - min) / d) return min + SQRT(u * d * (mode - min));
	return max - SQRT((1 - u) * d * (max - mode));
}

float _spMath_interpolate(float (*apply) (float a), float start, float end, float a) {
	return start + (end - start) * apply(a);
}

float _spMath_pow2_apply(float a) {
	if (a <= 0.5) return POW(a * 2, 2) / 2;
	return POW((a - 1) * 2, 2) / -2 + 1;
}

float _spMath_pow2out_apply(float a) {
	return POW(a - 1, 2) * -1 + 1;
}
