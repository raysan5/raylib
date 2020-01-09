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

#include <spine/Triangulator.h>
#include <spine/extension.h>
#include <stdio.h>

spTriangulator* spTriangulator_create() {
	spTriangulator* triangulator = CALLOC(spTriangulator, 1);

	triangulator->convexPolygons = spArrayFloatArray_create(16);
	triangulator->convexPolygonsIndices = spArrayShortArray_create(16);
	triangulator->indicesArray = spShortArray_create(128);
	triangulator->isConcaveArray = spIntArray_create(128);
	triangulator->triangles = spShortArray_create(128);
	triangulator->polygonPool = spArrayFloatArray_create(16);
	triangulator->polygonIndicesPool = spArrayShortArray_create(128);

	return triangulator;
}

void spTriangulator_dispose(spTriangulator* self) {
	int i;

	for (i = 0; i < self->convexPolygons->size; i++) {
		spFloatArray_dispose(self->convexPolygons->items[i]);
	}
	spArrayFloatArray_dispose(self->convexPolygons);

	for (i = 0; i < self->convexPolygonsIndices->size; i++) {
		spShortArray_dispose(self->convexPolygonsIndices->items[i]);
	}
	spArrayShortArray_dispose(self->convexPolygonsIndices);

	spShortArray_dispose(self->indicesArray);
	spIntArray_dispose(self->isConcaveArray);
	spShortArray_dispose(self->triangles);

	for (i = 0; i < self->polygonPool->size; i++) {
		spFloatArray_dispose(self->polygonPool->items[i]);
	}
	spArrayFloatArray_dispose(self->polygonPool);

	for (i = 0; i < self->polygonIndicesPool->size; i++) {
		spShortArray_dispose(self->polygonIndicesPool->items[i]);
	}
	spArrayShortArray_dispose(self->polygonIndicesPool);

	FREE(self);
}

static spFloatArray* _obtainPolygon(spTriangulator* self) {
	if (self->polygonPool->size == 0) return spFloatArray_create(16);
	else return spArrayFloatArray_pop(self->polygonPool);
}

static void _freePolygon(spTriangulator* self, spFloatArray* polygon) {
	spArrayFloatArray_add(self->polygonPool, polygon);
}

static void _freeAllPolygons(spTriangulator* self, spArrayFloatArray* polygons) {
	int i;
	for (i = 0; i < polygons->size; i++) {
		_freePolygon(self, polygons->items[i]);
	}
}

static spShortArray* _obtainPolygonIndices(spTriangulator* self) {
	if (self->polygonIndicesPool->size == 0) return spShortArray_create(16);
	else return spArrayShortArray_pop(self->polygonIndicesPool);
}

static void _freePolygonIndices(spTriangulator* self, spShortArray* indices) {
	spArrayShortArray_add(self->polygonIndicesPool, indices);
}

static void _freeAllPolygonIndices(spTriangulator* self, spArrayShortArray* polygonIndices) {
	int i;
	for (i = 0; i < polygonIndices->size; i++) {
		_freePolygonIndices(self, polygonIndices->items[i]);
	}
}

static int _positiveArea(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y) {
	return p1x * (p3y - p2y) + p2x * (p1y - p3y) + p3x * (p2y - p1y) >= 0;
}

static int _isConcave(int index, int vertexCount, float* vertices, short* indices) {
	int previous = indices[(vertexCount + index - 1) % vertexCount] << 1;
	int current = indices[index] << 1;
	int next = indices[(index + 1) % vertexCount] << 1;
	return !_positiveArea(vertices[previous], vertices[previous + 1],
		vertices[current], vertices[current + 1],
		vertices[next], vertices[next + 1]);
}

static int _winding (float p1x, float p1y, float p2x, float p2y, float p3x, float p3y) {
	float px = p2x - p1x, py = p2y - p1y;
	return p3x * py - p3y * px + px * p1y - p1x * py >= 0 ? 1 : -1;
}

spShortArray* spTriangulator_triangulate(spTriangulator* self, spFloatArray* verticesArray) {
	float* vertices = verticesArray->items;
	int vertexCount = verticesArray->size >> 1;
	int i, n, ii;

	spShortArray* indicesArray = self->indicesArray;
	short* indices;
	spIntArray* isConcaveArray;
	int* isConcave;
	spShortArray* triangles;

	spShortArray_clear(indicesArray);
	indices = spShortArray_setSize(indicesArray, vertexCount)->items;
	for (i = 0; i < vertexCount; i++)
		indices[i] = (short)i;

	isConcaveArray = self->isConcaveArray;
	isConcave = spIntArray_setSize(isConcaveArray, vertexCount)->items;
	for (i = 0, n = vertexCount; i < n; ++i)
		isConcave[i] = _isConcave(i, vertexCount, vertices, indices);

	triangles = self->triangles;
	spShortArray_clear(triangles);
	spShortArray_ensureCapacity(triangles, MAX(0, vertexCount - 2) << 2);

	while (vertexCount > 3) {
		int previous = vertexCount - 1, next = 1;
		int previousIndex, nextIndex;
		i = 0;
		while (1) {
			if (!isConcave[i]) {
				int p1 = indices[previous] << 1, p2 = indices[i] << 1, p3 = indices[next] << 1;
				float p1x = vertices[p1], p1y = vertices[p1 + 1];
				float p2x = vertices[p2], p2y = vertices[p2 + 1];
				float p3x = vertices[p3], p3y = vertices[p3 + 1];
				for (ii = (next + 1) % vertexCount; ii != previous; ii = (ii + 1) % vertexCount) {
					int v;
					float vx, vy;
					if (!isConcave[ii]) continue;
					v = indices[ii] << 1;
					vx = vertices[v]; vy = vertices[v + 1];
					if (_positiveArea(p3x, p3y, p1x, p1y, vx, vy)) {
						if (_positiveArea(p1x, p1y, p2x, p2y, vx, vy)) {
							if (_positiveArea(p2x, p2y, p3x, p3y, vx, vy)) goto break_outer;
						}
					}
				}
				break;
			}
			break_outer:

			if (next == 0) {
				do {
					if (!isConcave[i]) break;
					i--;
				} while (i > 0);
				break;
			}

			previous = i;
			i = next;
			next = (next + 1) % vertexCount;
		}

		spShortArray_add(triangles, indices[(vertexCount + i - 1) % vertexCount]);
		spShortArray_add(triangles, indices[i]);
		spShortArray_add(triangles, indices[(i + 1) % vertexCount]);
		spShortArray_removeAt(indicesArray, i);
		spIntArray_removeAt(isConcaveArray, i);
		vertexCount--;

		previousIndex = (vertexCount + i - 1) % vertexCount;
		nextIndex = i == vertexCount ? 0 : i;
		isConcave[previousIndex] = _isConcave(previousIndex, vertexCount, vertices, indices);
		isConcave[nextIndex] = _isConcave(nextIndex, vertexCount, vertices, indices);
	}

	if (vertexCount == 3) {
		spShortArray_add(triangles, indices[2]);
		spShortArray_add(triangles, indices[0]);
		spShortArray_add(triangles, indices[1]);
	}

	return triangles;
}

spArrayFloatArray* spTriangulator_decompose(spTriangulator* self, spFloatArray* verticesArray, spShortArray* triangles) {
	float* vertices = verticesArray->items;

	spArrayFloatArray* convexPolygons = self->convexPolygons;
	spArrayShortArray* convexPolygonsIndices;
	spShortArray* polygonIndices;
	spFloatArray* polygon;

	int fanBaseIndex, lastWinding;
	short* trianglesItems;
	int i, n;

	_freeAllPolygons(self, convexPolygons);
	spArrayFloatArray_clear(convexPolygons);

	convexPolygonsIndices = self->convexPolygonsIndices;
	_freeAllPolygonIndices(self, convexPolygonsIndices);
	spArrayShortArray_clear(convexPolygonsIndices);

	polygonIndices = _obtainPolygonIndices(self);
	spShortArray_clear(polygonIndices);

	polygon = _obtainPolygon(self);
	spFloatArray_clear(polygon);

	fanBaseIndex = -1; lastWinding = 0;
	trianglesItems = triangles->items;
	for (i = 0, n = triangles->size; i < n; i += 3) {
		int t1 = trianglesItems[i] << 1, t2 = trianglesItems[i + 1] << 1, t3 = trianglesItems[i + 2] << 1;
		float x1 = vertices[t1], y1 = vertices[t1 + 1];
		float x2 = vertices[t2], y2 = vertices[t2 + 1];
		float x3 = vertices[t3], y3 = vertices[t3 + 1];

		int merged = 0;
		if (fanBaseIndex == t1) {
			int o = polygon->size - 4;
			float* p = polygon->items;
			int winding1 = _winding(p[o], p[o + 1], p[o + 2], p[o + 3], x3, y3);
			int winding2 = _winding(x3, y3, p[0], p[1], p[2], p[3]);
			if (winding1 == lastWinding && winding2 == lastWinding) {
				spFloatArray_add(polygon, x3);
				spFloatArray_add(polygon, y3);
				spShortArray_add(polygonIndices, t3);
				merged = 1;
			}
		}

		if (!merged) {
			if (polygon->size > 0) {
				spArrayFloatArray_add(convexPolygons, polygon);
				spArrayShortArray_add(convexPolygonsIndices, polygonIndices);
			} else {
				_freePolygon(self, polygon);
				_freePolygonIndices(self, polygonIndices);
			}
			polygon = _obtainPolygon(self);
			spFloatArray_clear(polygon);
			spFloatArray_add(polygon, x1);
			spFloatArray_add(polygon, y1);
			spFloatArray_add(polygon, x2);
			spFloatArray_add(polygon, y2);
			spFloatArray_add(polygon, x3);
			spFloatArray_add(polygon, y3);
			polygonIndices = _obtainPolygonIndices(self);
			spShortArray_clear(polygonIndices);
			spShortArray_add(polygonIndices, t1);
			spShortArray_add(polygonIndices, t2);
			spShortArray_add(polygonIndices, t3);
			lastWinding = _winding(x1, y1, x2, y2, x3, y3);
			fanBaseIndex = t1;
		}
	}

	if (polygon->size > 0) {
		spArrayFloatArray_add(convexPolygons, polygon);
		spArrayShortArray_add(convexPolygonsIndices, polygonIndices);
	}

	for (i = 0, n = convexPolygons->size; i < n; i++) {
		int firstIndex, lastIndex;
		int o;
		float* p;
		float prevPrevX, prevPrevY, prevX, prevY, firstX, firstY, secondX, secondY;
		int winding;
		int ii;

		polygonIndices = convexPolygonsIndices->items[i];
		if (polygonIndices->size == 0) continue;
		firstIndex = polygonIndices->items[0];
		lastIndex = polygonIndices->items[polygonIndices->size - 1];

		polygon = convexPolygons->items[i];
		o = polygon->size - 4;
		p = polygon->items;
		prevPrevX = p[o]; prevPrevY = p[o + 1];
		prevX = p[o + 2]; prevY = p[o + 3];
		firstX = p[0]; firstY = p[1];
		secondX = p[2]; secondY = p[3];
		winding = _winding(prevPrevX, prevPrevY, prevX, prevY, firstX, firstY);

		for (ii = 0; ii < n; ii++) {
			spShortArray* otherIndices;
			int otherFirstIndex, otherSecondIndex, otherLastIndex;
			spFloatArray* otherPoly;
			float x3, y3;
			int winding1, winding2;

			if (ii == i) continue;
			otherIndices = convexPolygonsIndices->items[ii];
			if (otherIndices->size != 3) continue;
			otherFirstIndex = otherIndices->items[0];
			otherSecondIndex = otherIndices->items[1];
			otherLastIndex = otherIndices->items[2];

			otherPoly = convexPolygons->items[ii];
			x3 = otherPoly->items[otherPoly->size - 2]; y3 = otherPoly->items[otherPoly->size - 1];

			if (otherFirstIndex != firstIndex || otherSecondIndex != lastIndex) continue;
			winding1 = _winding(prevPrevX, prevPrevY, prevX, prevY, x3, y3);
			winding2 = _winding(x3, y3, firstX, firstY, secondX, secondY);
			if (winding1 == winding && winding2 == winding) {
				spFloatArray_clear(otherPoly);
				spShortArray_clear(otherIndices);
				spFloatArray_add(polygon, x3);
				spFloatArray_add(polygon, y3);
				spShortArray_add(polygonIndices, otherLastIndex);
				prevPrevX = prevX;
				prevPrevY = prevY;
				prevX = x3;
				prevY = y3;
				ii = 0;
			}
		}
	}

	for (i = convexPolygons->size - 1; i >= 0; i--) {
		polygon = convexPolygons->items[i];
		if (polygon->size == 0) {
			spArrayFloatArray_removeAt(convexPolygons, i);
			_freePolygon(self, polygon);
			polygonIndices = convexPolygonsIndices->items[i];
			spArrayShortArray_removeAt(convexPolygonsIndices, i);
			_freePolygonIndices(self, polygonIndices);
		}
	}

	return convexPolygons;
}
