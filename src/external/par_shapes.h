// SHAPES :: https://github.com/prideout/par
// Simple C library for creation and manipulation of triangle meshes.
//
// The API is divided into three sections:
//
//   - Generators.  Create parametric surfaces, platonic solids, etc.
//   - Queries.     Ask a mesh for its axis-aligned bounding box, etc.
//   - Transforms.  Rotate a mesh, merge it with another, add normals, etc.
//
// In addition to the comment block above each function declaration, the API
// has informal documentation here:
//
//     https://prideout.net/shapes
//
// For our purposes, a "mesh" is a list of points and a list of triangles; the
// former is a flattened list of three-tuples (32-bit floats) and the latter is
// also a flattened list of three-tuples (16-bit uints).  Triangles are always
// oriented such that their front face winds counter-clockwise.
//
// Optionally, meshes can contain 3D normals (one per vertex), and 2D texture
// coordinates (one per vertex).  That's it!  If you need something fancier,
// look elsewhere.
//
// The MIT License
// Copyright (c) 2015 Philip Rideout

#ifndef PAR_SHAPES_H
#define PAR_SHAPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Ray: commented to avoid conflict with raylib bool
/*
#if !defined(_MSC_VER)
# include <stdbool.h>
#else // MSVC
# if _MSC_VER >= 1800
#  include <stdbool.h>
# else // stdbool.h missing prior to MSVC++ 12.0 (VS2013)
//#  define bool int      
//#  define true 1
//#  define false 0
# endif
#endif
*/

#ifndef PAR_SHAPES_T
#define PAR_SHAPES_T uint16_t
#endif

typedef struct par_shapes_mesh_s {
    float* points;           // Flat list of 3-tuples (X Y Z X Y Z...)
    int npoints;             // Number of points
    PAR_SHAPES_T* triangles; // Flat list of 3-tuples (I J K I J K...)
    int ntriangles;          // Number of triangles
    float* normals;          // Optional list of 3-tuples (X Y Z X Y Z...)
    float* tcoords;          // Optional list of 2-tuples (U V U V U V...)
} par_shapes_mesh;

void par_shapes_free_mesh(par_shapes_mesh*);

// Generators ------------------------------------------------------------------

// Instance a cylinder that sits on the Z=0 plane using the given tessellation
// levels across the UV domain.  Think of "slices" like a number of pizza
// slices, and "stacks" like a number of stacked rings.  Height and radius are
// both 1.0, but they can easily be changed with par_shapes_scale.
par_shapes_mesh* par_shapes_create_cylinder(int slices, int stacks);

// Create a donut that sits on the Z=0 plane with the specified inner radius.
// The outer radius can be controlled with par_shapes_scale.
par_shapes_mesh* par_shapes_create_torus(int slices, int stacks, float radius);

// Create a sphere with texture coordinates and small triangles near the poles.
par_shapes_mesh* par_shapes_create_parametric_sphere(int slices, int stacks);

// Approximate a sphere with a subdivided icosahedron, which produces a nice
// distribution of triangles, but no texture coordinates.  Each subdivision
// level scales the number of triangles by four, so use a very low number.
par_shapes_mesh* par_shapes_create_subdivided_sphere(int nsubdivisions);

// More parametric surfaces.
par_shapes_mesh* par_shapes_create_klein_bottle(int slices, int stacks);
par_shapes_mesh* par_shapes_create_trefoil_knot(int slices, int stacks,
    float radius);
par_shapes_mesh* par_shapes_create_hemisphere(int slices, int stacks);
par_shapes_mesh* par_shapes_create_plane(int slices, int stacks);

// Create a parametric surface from a callback function that consumes a 2D
// point in [0,1] and produces a 3D point.
typedef void (*par_shapes_fn)(float const*, float*, void*);
par_shapes_mesh* par_shapes_create_parametric(par_shapes_fn, int slices,
    int stacks, void* userdata);

// Generate points for a 20-sided polyhedron that fits in the unit sphere.
// Texture coordinates and normals are not generated.
par_shapes_mesh* par_shapes_create_icosahedron();

// Generate points for a 12-sided polyhedron that fits in the unit sphere.
// Again, texture coordinates and normals are not generated.
par_shapes_mesh* par_shapes_create_dodecahedron();

// More platonic solids.
par_shapes_mesh* par_shapes_create_octahedron();
par_shapes_mesh* par_shapes_create_tetrahedron();
par_shapes_mesh* par_shapes_create_cube();

// Generate an orientable disk shape in 3-space.  Does not include normals or
// texture coordinates.
par_shapes_mesh* par_shapes_create_disk(float radius, int slices,
    float const* center, float const* normal);

// Create an empty shape.  Useful for building scenes with merge_and_free.
par_shapes_mesh* par_shapes_create_empty();

// Generate a rock shape that sits on the Y=0 plane, and sinks into it a bit.
// This includes smooth normals but no texture coordinates.  Each subdivision
// level scales the number of triangles by four, so use a very low number.
par_shapes_mesh* par_shapes_create_rock(int seed, int nsubdivisions);

// Create trees or vegetation by executing a recursive turtle graphics program.
// The program is a list of command-argument pairs.  See the unit test for
// an example.  Texture coordinates and normals are not generated.
par_shapes_mesh* par_shapes_create_lsystem(char const* program, int slices,
    int maxdepth);

// Queries ---------------------------------------------------------------------

// Dump out a text file conforming to the venerable OBJ format.
void par_shapes_export(par_shapes_mesh const*, char const* objfile);

// Take a pointer to 6 floats and set them to min xyz, max xyz.
void par_shapes_compute_aabb(par_shapes_mesh const* mesh, float* aabb);

// Make a deep copy of a mesh.  To make a brand new copy, pass null to "target".
// To avoid memory churn, pass an existing mesh to "target".
par_shapes_mesh* par_shapes_clone(par_shapes_mesh const* mesh,
    par_shapes_mesh* target);

// Transformations -------------------------------------------------------------

void par_shapes_merge(par_shapes_mesh* dst, par_shapes_mesh const* src);
void par_shapes_translate(par_shapes_mesh*, float x, float y, float z);
void par_shapes_rotate(par_shapes_mesh*, float radians, float const* axis);
void par_shapes_scale(par_shapes_mesh*, float x, float y, float z);
void par_shapes_merge_and_free(par_shapes_mesh* dst, par_shapes_mesh* src);

// Reverse the winding of a run of faces.  Useful when drawing the inside of
// a Cornell Box.  Pass 0 for nfaces to reverse every face in the mesh.
void par_shapes_invert(par_shapes_mesh*, int startface, int nfaces);

// Remove all triangles whose area is less than minarea.
void par_shapes_remove_degenerate(par_shapes_mesh*, float minarea);

// Dereference the entire index buffer and replace the point list.
// This creates an inefficient structure, but is useful for drawing facets.
// If create_indices is true, a trivial "0 1 2 3..." index buffer is generated.
void par_shapes_unweld(par_shapes_mesh* mesh, bool create_indices);

// Merge colocated verts, build a new index buffer, and return the
// optimized mesh.  Epsilon is the maximum distance to consider when
// welding vertices. The mapping argument can be null, or a pointer to
// npoints integers, which gets filled with the mapping from old vertex
// indices to new indices.
par_shapes_mesh* par_shapes_weld(par_shapes_mesh const*, float epsilon,
    PAR_SHAPES_T* mapping);

// Compute smooth normals by averaging adjacent facet normals.
void par_shapes_compute_normals(par_shapes_mesh* m);

#ifndef PAR_PI
#define PAR_PI (3.14159265359)
#define PAR_MIN(a, b) (a > b ? b : a)
#define PAR_MAX(a, b) (a > b ? a : b)
#define PAR_CLAMP(v, lo, hi) PAR_MAX(lo, PAR_MIN(hi, v))
#define PAR_SWAP(T, A, B) { T tmp = B; B = A; A = tmp; }
#define PAR_SQR(a) ((a) * (a))
#endif

#ifndef PAR_MALLOC
#define PAR_MALLOC(T, N) ((T*) malloc(N * sizeof(T)))
#define PAR_CALLOC(T, N) ((T*) calloc(N * sizeof(T), 1))
#define PAR_REALLOC(T, BUF, N) ((T*) realloc(BUF, sizeof(T) * (N)))
#define PAR_FREE(BUF) free(BUF)
#endif

#ifdef __cplusplus
}
#endif

// -----------------------------------------------------------------------------
// END PUBLIC API
// -----------------------------------------------------------------------------

#ifdef PAR_SHAPES_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <errno.h>

static void par_shapes__sphere(float const* uv, float* xyz, void*);
static void par_shapes__hemisphere(float const* uv, float* xyz, void*);
static void par_shapes__plane(float const* uv, float* xyz, void*);
static void par_shapes__klein(float const* uv, float* xyz, void*);
static void par_shapes__cylinder(float const* uv, float* xyz, void*);
static void par_shapes__torus(float const* uv, float* xyz, void*);
static void par_shapes__trefoil(float const* uv, float* xyz, void*);

struct osn_context;
static int par__simplex_noise(int64_t seed, struct osn_context** ctx);
static void par__simplex_noise_free(struct osn_context* ctx);
static double par__simplex_noise2(struct osn_context* ctx, double x, double y);

static void par_shapes__copy3(float* result, float const* a)
{
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
}

static float par_shapes__dot3(float const* a, float const* b)
{
    return b[0] * a[0] + b[1] * a[1] + b[2] * a[2];
}

static void par_shapes__transform3(float* p, float const* x, float const* y,
    float const* z)
{
    float px = par_shapes__dot3(p, x);
    float py = par_shapes__dot3(p, y);
    float pz = par_shapes__dot3(p, z);
    p[0] = px;
    p[1] = py;
    p[2] = pz;
}

static void par_shapes__cross3(float* result, float const* a, float const* b)
{
    float x = (a[1] * b[2]) - (a[2] * b[1]);
    float y = (a[2] * b[0]) - (a[0] * b[2]);
    float z = (a[0] * b[1]) - (a[1] * b[0]);
    result[0] = x;
    result[1] = y;
    result[2] = z;
}

static void par_shapes__mix3(float* d, float const* a, float const* b, float t)
{
    float x = b[0] * t + a[0] * (1 - t);
    float y = b[1] * t + a[1] * (1 - t);
    float z = b[2] * t + a[2] * (1 - t);
    d[0] = x;
    d[1] = y;
    d[2] = z;
}

static void par_shapes__scale3(float* result, float a)
{
    result[0] *= a;
    result[1] *= a;
    result[2] *= a;
}

static void par_shapes__normalize3(float* v)
{
    float lsqr = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (lsqr > 0) {
        par_shapes__scale3(v, 1.0f / lsqr);
    }
}

static void par_shapes__subtract3(float* result, float const* a)
{
    result[0] -= a[0];
    result[1] -= a[1];
    result[2] -= a[2];
}

static void par_shapes__add3(float* result, float const* a)
{
    result[0] += a[0];
    result[1] += a[1];
    result[2] += a[2];
}

static float par_shapes__sqrdist3(float const* a, float const* b)
{
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    return dx * dx + dy * dy + dz * dz;
}

static void par_shapes__compute_welded_normals(par_shapes_mesh* m)
{
    m->normals = PAR_MALLOC(float, m->npoints * 3);
    PAR_SHAPES_T* weldmap = PAR_MALLOC(PAR_SHAPES_T, m->npoints);
    par_shapes_mesh* welded = par_shapes_weld(m, 0.01, weldmap);
    par_shapes_compute_normals(welded);
    float* pdst = m->normals;
    for (int i = 0; i < m->npoints; i++, pdst += 3) {
        int d = weldmap[i];
        float const* pnormal = welded->normals + d * 3;
        pdst[0] = pnormal[0];
        pdst[1] = pnormal[1];
        pdst[2] = pnormal[2];
    }
    PAR_FREE(weldmap);
    par_shapes_free_mesh(welded);
}

par_shapes_mesh* par_shapes_create_cylinder(int slices, int stacks)
{
    if (slices < 3 || stacks < 1) {
        return 0;
    }
    return par_shapes_create_parametric(par_shapes__cylinder, slices,
        stacks, 0);
}

par_shapes_mesh* par_shapes_create_parametric_sphere(int slices, int stacks)
{
    if (slices < 3 || stacks < 3) {
        return 0;
    }
    par_shapes_mesh* m = par_shapes_create_parametric(par_shapes__sphere,
        slices, stacks, 0);
    par_shapes_remove_degenerate(m, 0.0001);
    return m;
}

par_shapes_mesh* par_shapes_create_hemisphere(int slices, int stacks)
{
    if (slices < 3 || stacks < 3) {
        return 0;
    }
    par_shapes_mesh* m = par_shapes_create_parametric(par_shapes__hemisphere,
        slices, stacks, 0);
    par_shapes_remove_degenerate(m, 0.0001);
    return m;
}

par_shapes_mesh* par_shapes_create_torus(int slices, int stacks, float radius)
{
    if (slices < 3 || stacks < 3) {
        return 0;
    }
    assert(radius <= 1.0 && "Use smaller radius to avoid self-intersection.");
    assert(radius >= 0.1 && "Use larger radius to avoid self-intersection.");
    void* userdata = (void*) &radius;
    return par_shapes_create_parametric(par_shapes__torus, slices,
        stacks, userdata);
}

par_shapes_mesh* par_shapes_create_klein_bottle(int slices, int stacks)
{
    if (slices < 3 || stacks < 3) {
        return 0;
    }
    par_shapes_mesh* mesh = par_shapes_create_parametric(
        par_shapes__klein, slices, stacks, 0);
    int face = 0;
    for (int stack = 0; stack < stacks; stack++) {
        for (int slice = 0; slice < slices; slice++, face += 2) {
            if (stack < 27 * stacks / 32) {
                par_shapes_invert(mesh, face, 2);
            }
        }
    }
    par_shapes__compute_welded_normals(mesh);
    return mesh;
}

par_shapes_mesh* par_shapes_create_trefoil_knot(int slices, int stacks,
    float radius)
{
    if (slices < 3 || stacks < 3) {
        return 0;
    }
    assert(radius <= 3.0 && "Use smaller radius to avoid self-intersection.");
    assert(radius >= 0.5 && "Use larger radius to avoid self-intersection.");
    void* userdata = (void*) &radius;
    return par_shapes_create_parametric(par_shapes__trefoil, slices,
        stacks, userdata);
}

par_shapes_mesh* par_shapes_create_plane(int slices, int stacks)
{
    if (slices < 1 || stacks < 1) {
        return 0;
    }
    return par_shapes_create_parametric(par_shapes__plane, slices,
        stacks, 0);
}

par_shapes_mesh* par_shapes_create_parametric(par_shapes_fn fn,
    int slices, int stacks, void* userdata)
{
    par_shapes_mesh* mesh = PAR_CALLOC(par_shapes_mesh, 1);

    // Generate verts.
    mesh->npoints = (slices + 1) * (stacks + 1);
    mesh->points = PAR_CALLOC(float, 3 * mesh->npoints);
    float uv[2];
    float xyz[3];
    float* points = mesh->points;
    for (int stack = 0; stack < stacks + 1; stack++) {
        uv[0] = (float) stack / stacks;
        for (int slice = 0; slice < slices + 1; slice++) {
            uv[1] = (float) slice / slices;
            fn(uv, xyz, userdata);
            *points++ = xyz[0];
            *points++ = xyz[1];
            *points++ = xyz[2];
        }
    }

    // Generate texture coordinates.
    mesh->tcoords = PAR_CALLOC(float, 2 * mesh->npoints);
    float* uvs = mesh->tcoords;
    for (int stack = 0; stack < stacks + 1; stack++) {
        uv[0] = (float) stack / stacks;
        for (int slice = 0; slice < slices + 1; slice++) {
            uv[1] = (float) slice / slices;
            *uvs++ = uv[0];
            *uvs++ = uv[1];
        }
    }

    // Generate faces.
    mesh->ntriangles = 2 * slices * stacks;
    mesh->triangles = PAR_CALLOC(PAR_SHAPES_T, 3 * mesh->ntriangles);
    int v = 0;
    PAR_SHAPES_T* face = mesh->triangles;
    for (int stack = 0; stack < stacks; stack++) {
        for (int slice = 0; slice < slices; slice++) {
            int next = slice + 1;
            *face++ = v + slice + slices + 1;
            *face++ = v + next;
            *face++ = v + slice;
            *face++ = v + slice + slices + 1;
            *face++ = v + next + slices + 1;
            *face++ = v + next;
        }
        v += slices + 1;
    }

    par_shapes__compute_welded_normals(mesh);
    return mesh;
}

void par_shapes_free_mesh(par_shapes_mesh* mesh)
{
    PAR_FREE(mesh->points);
    PAR_FREE(mesh->triangles);
    PAR_FREE(mesh->normals);
    PAR_FREE(mesh->tcoords);
    PAR_FREE(mesh);
}

void par_shapes_export(par_shapes_mesh const* mesh, char const* filename)
{
    FILE* objfile = fopen(filename, "wt");
    float const* points = mesh->points;
    float const* tcoords = mesh->tcoords;
    float const* norms = mesh->normals;
    PAR_SHAPES_T const* indices = mesh->triangles;
    if (tcoords && norms) {
        for (int nvert = 0; nvert < mesh->npoints; nvert++) {
            fprintf(objfile, "v %f %f %f\n", points[0], points[1], points[2]);
            fprintf(objfile, "vt %f %f\n", tcoords[0], tcoords[1]);
            fprintf(objfile, "vn %f %f %f\n", norms[0], norms[1], norms[2]);
            points += 3;
            norms += 3;
            tcoords += 2;
        }
        for (int nface = 0; nface < mesh->ntriangles; nface++) {
            int a = 1 + *indices++;
            int b = 1 + *indices++;
            int c = 1 + *indices++;
            fprintf(objfile, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                a, a, a, b, b, b, c, c, c);
        }
    } else if (norms) {
        for (int nvert = 0; nvert < mesh->npoints; nvert++) {
            fprintf(objfile, "v %f %f %f\n", points[0], points[1], points[2]);
            fprintf(objfile, "vn %f %f %f\n", norms[0], norms[1], norms[2]);
            points += 3;
            norms += 3;
        }
        for (int nface = 0; nface < mesh->ntriangles; nface++) {
            int a = 1 + *indices++;
            int b = 1 + *indices++;
            int c = 1 + *indices++;
            fprintf(objfile, "f %d//%d %d//%d %d//%d\n", a, a, b, b, c, c);
        }
    } else if (tcoords) {
        for (int nvert = 0; nvert < mesh->npoints; nvert++) {
            fprintf(objfile, "v %f %f %f\n", points[0], points[1], points[2]);
            fprintf(objfile, "vt %f %f\n", tcoords[0], tcoords[1]);
            points += 3;
            tcoords += 2;
        }
        for (int nface = 0; nface < mesh->ntriangles; nface++) {
            int a = 1 + *indices++;
            int b = 1 + *indices++;
            int c = 1 + *indices++;
            fprintf(objfile, "f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c);
        }
    } else {
        for (int nvert = 0; nvert < mesh->npoints; nvert++) {
            fprintf(objfile, "v %f %f %f\n", points[0], points[1], points[2]);
            points += 3;
        }
        for (int nface = 0; nface < mesh->ntriangles; nface++) {
            int a = 1 + *indices++;
            int b = 1 + *indices++;
            int c = 1 + *indices++;
            fprintf(objfile, "f %d %d %d\n", a, b, c);
        }
    }
    fclose(objfile);
}

static void par_shapes__sphere(float const* uv, float* xyz, void* userdata)
{
    float phi = uv[0] * PAR_PI;
    float theta = uv[1] * 2 * PAR_PI;
    xyz[0] = cosf(theta) * sinf(phi);
    xyz[1] = sinf(theta) * sinf(phi);
    xyz[2] = cosf(phi);
}

static void par_shapes__hemisphere(float const* uv, float* xyz, void* userdata)
{
    float phi = uv[0] * PAR_PI;
    float theta = uv[1] * PAR_PI;
    xyz[0] = cosf(theta) * sinf(phi);
    xyz[1] = sinf(theta) * sinf(phi);
    xyz[2] = cosf(phi);
}

static void par_shapes__plane(float const* uv, float* xyz, void* userdata)
{
    xyz[0] = uv[0];
    xyz[1] = uv[1];
    xyz[2] = 0;
}

static void par_shapes__klein(float const* uv, float* xyz, void* userdata)
{
    float u = uv[0] * PAR_PI;
    float v = uv[1] * 2 * PAR_PI;
    u = u * 2;
    if (u < PAR_PI) {
        xyz[0] = 3 * cosf(u) * (1 + sinf(u)) + (2 * (1 - cosf(u) / 2)) *
            cosf(u) * cosf(v);
        xyz[2] = -8 * sinf(u) - 2 * (1 - cosf(u) / 2) * sinf(u) * cosf(v);
    } else {
        xyz[0] = 3 * cosf(u) * (1 + sinf(u)) + (2 * (1 - cosf(u) / 2)) *
            cosf(v + PAR_PI);
        xyz[2] = -8 * sinf(u);
    }
    xyz[1] = -2 * (1 - cosf(u) / 2) * sinf(v);
}

static void par_shapes__cylinder(float const* uv, float* xyz, void* userdata)
{
    float theta = uv[1] * 2 * PAR_PI;
    xyz[0] = sinf(theta);
    xyz[1] = cosf(theta);
    xyz[2] = uv[0];
}

static void par_shapes__torus(float const* uv, float* xyz, void* userdata)
{
    float major = 1;
    float minor = *((float*) userdata);
    float theta = uv[0] * 2 * PAR_PI;
    float phi = uv[1] * 2 * PAR_PI;
    float beta = major + minor * cosf(phi);
    xyz[0] = cosf(theta) * beta;
    xyz[1] = sinf(theta) * beta;
    xyz[2] = sinf(phi) * minor;
}

static void par_shapes__trefoil(float const* uv, float* xyz, void* userdata)
{
    float minor = *((float*) userdata);
    const float a = 0.5f;
    const float b = 0.3f;
    const float c = 0.5f;
    const float d = minor * 0.1f;
    const float u = (1 - uv[0]) * 4 * PAR_PI;
    const float v = uv[1] * 2 * PAR_PI;
    const float r = a + b * cos(1.5f * u);
    const float x = r * cos(u);
    const float y = r * sin(u);
    const float z = c * sin(1.5f * u);
    float q[3];
    q[0] =
        -1.5f * b * sin(1.5f * u) * cos(u) - (a + b * cos(1.5f * u)) * sin(u);
    q[1] =
        -1.5f * b * sin(1.5f * u) * sin(u) + (a + b * cos(1.5f * u)) * cos(u);
    q[2] = 1.5f * c * cos(1.5f * u);
    par_shapes__normalize3(q);
    float qvn[3] = {q[1], -q[0], 0};
    par_shapes__normalize3(qvn);
    float ww[3];
    par_shapes__cross3(ww, q, qvn);
    xyz[0] = x + d * (qvn[0] * cos(v) + ww[0] * sin(v));
    xyz[1] = y + d * (qvn[1] * cos(v) + ww[1] * sin(v));
    xyz[2] = z + d * ww[2] * sin(v);
}

void par_shapes_merge(par_shapes_mesh* dst, par_shapes_mesh const* src)
{
    PAR_SHAPES_T offset = dst->npoints;
    int npoints = dst->npoints + src->npoints;
    int vecsize = sizeof(float) * 3;
    dst->points = PAR_REALLOC(float, dst->points, 3 * npoints);
    memcpy(dst->points + 3 * dst->npoints, src->points, vecsize * src->npoints);
    dst->npoints = npoints;
    if (src->normals || dst->normals) {
        dst->normals = PAR_REALLOC(float, dst->normals, 3 * npoints);
        if (src->normals) {
            memcpy(dst->normals + 3 * offset, src->normals,
                vecsize * src->npoints);
        }
    }
    if (src->tcoords || dst->tcoords) {
        int uvsize = sizeof(float) * 2;
        dst->tcoords = PAR_REALLOC(float, dst->tcoords, 2 * npoints);
        if (src->tcoords) {
            memcpy(dst->tcoords + 2 * offset, src->tcoords,
                uvsize * src->npoints);
        }
    }
    int ntriangles = dst->ntriangles + src->ntriangles;
    dst->triangles = PAR_REALLOC(PAR_SHAPES_T, dst->triangles, 3 * ntriangles);
    PAR_SHAPES_T* ptriangles = dst->triangles + 3 * dst->ntriangles;
    PAR_SHAPES_T const* striangles = src->triangles;
    for (int i = 0; i < src->ntriangles; i++) {
        *ptriangles++ = offset + *striangles++;
        *ptriangles++ = offset + *striangles++;
        *ptriangles++ = offset + *striangles++;
    }
    dst->ntriangles = ntriangles;
}

par_shapes_mesh* par_shapes_create_disk(float radius, int slices,
    float const* center, float const* normal)
{
    par_shapes_mesh* mesh = PAR_CALLOC(par_shapes_mesh, 1);
    mesh->npoints = slices + 1;
    mesh->points = PAR_MALLOC(float, 3 * mesh->npoints);
    float* points = mesh->points;
    *points++ = 0;
    *points++ = 0;
    *points++ = 0;
    for (int i = 0; i < slices; i++) {
        float theta = i * PAR_PI * 2 / slices;
        *points++ = radius * cos(theta);
        *points++ = radius * sin(theta);
        *points++ = 0;
    }
    float nnormal[3] = {normal[0], normal[1], normal[2]};
    par_shapes__normalize3(nnormal);
    mesh->normals = PAR_MALLOC(float, 3 * mesh->npoints);
    float* norms = mesh->normals;
    for (int i = 0; i < mesh->npoints; i++) {
        *norms++ = nnormal[0];
        *norms++ = nnormal[1];
        *norms++ = nnormal[2];
    }
    mesh->ntriangles = slices;
    mesh->triangles = PAR_MALLOC(PAR_SHAPES_T, 3 * mesh->ntriangles);
    PAR_SHAPES_T* triangles = mesh->triangles;
    for (int i = 0; i < slices; i++) {
        *triangles++ = 0;
        *triangles++ = 1 + i;
        *triangles++ = 1 + (i + 1) % slices;
    }
    float k[3] = {0, 0, -1};
    float axis[3];
    par_shapes__cross3(axis, nnormal, k);
    par_shapes__normalize3(axis);
    par_shapes_rotate(mesh, acos(nnormal[2]), axis);
    par_shapes_translate(mesh, center[0], center[1], center[2]);
    return mesh;
}

par_shapes_mesh* par_shapes_create_empty()
{
    return PAR_CALLOC(par_shapes_mesh, 1);
}

void par_shapes_translate(par_shapes_mesh* m, float x, float y, float z)
{
    float* points = m->points;
    for (int i = 0; i < m->npoints; i++) {
        *points++ += x;
        *points++ += y;
        *points++ += z;
    }
}

void par_shapes_rotate(par_shapes_mesh* mesh, float radians, float const* axis)
{
    float s = sinf(radians);
    float c = cosf(radians);
    float x = axis[0];
    float y = axis[1];
    float z = axis[2];
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    float oneMinusC = 1.0f - c;
    float col0[3] = {
        (((x * x) * oneMinusC) + c),
        ((xy * oneMinusC) + (z * s)), ((zx * oneMinusC) - (y * s))
    };
    float col1[3] = {
        ((xy * oneMinusC) - (z * s)),
        (((y * y) * oneMinusC) + c), ((yz * oneMinusC) + (x * s))
    };
    float col2[3] = {
        ((zx * oneMinusC) + (y * s)),
        ((yz * oneMinusC) - (x * s)), (((z * z) * oneMinusC) + c)
    };
    float* p = mesh->points;
    for (int i = 0; i < mesh->npoints; i++, p += 3) {
        float x = col0[0] * p[0] + col1[0] * p[1] + col2[0] * p[2];
        float y = col0[1] * p[0] + col1[1] * p[1] + col2[1] * p[2];
        float z = col0[2] * p[0] + col1[2] * p[1] + col2[2] * p[2];
        p[0] = x;
        p[1] = y;
        p[2] = z;
    }
    p = mesh->normals;
    if (p) {
        for (int i = 0; i < mesh->npoints; i++, p += 3) {
            float x = col0[0] * p[0] + col1[0] * p[1] + col2[0] * p[2];
            float y = col0[1] * p[0] + col1[1] * p[1] + col2[1] * p[2];
            float z = col0[2] * p[0] + col1[2] * p[1] + col2[2] * p[2];
            p[0] = x;
            p[1] = y;
            p[2] = z;
        }
    }
}

void par_shapes_scale(par_shapes_mesh* m, float x, float y, float z)
{
    float* points = m->points;
    for (int i = 0; i < m->npoints; i++) {
        *points++ *= x;
        *points++ *= y;
        *points++ *= z;
    }
}

void par_shapes_merge_and_free(par_shapes_mesh* dst, par_shapes_mesh* src)
{
    par_shapes_merge(dst, src);
    par_shapes_free_mesh(src);
}

void par_shapes_compute_aabb(par_shapes_mesh const* m, float* aabb)
{
    float* points = m->points;
    aabb[0] = aabb[3] = points[0];
    aabb[1] = aabb[4] = points[1];
    aabb[2] = aabb[5] = points[2];
    points += 3;
    for (int i = 1; i < m->npoints; i++, points += 3) {
        aabb[0] = PAR_MIN(points[0], aabb[0]);
        aabb[1] = PAR_MIN(points[1], aabb[1]);
        aabb[2] = PAR_MIN(points[2], aabb[2]);
        aabb[3] = PAR_MAX(points[0], aabb[3]);
        aabb[4] = PAR_MAX(points[1], aabb[4]);
        aabb[5] = PAR_MAX(points[2], aabb[5]);
    }
}

void par_shapes_invert(par_shapes_mesh* m, int face, int nfaces)
{
    nfaces = nfaces ? nfaces : m->ntriangles;
    PAR_SHAPES_T* tri = m->triangles + face * 3;
    for (int i = 0; i < nfaces; i++) {
        PAR_SWAP(PAR_SHAPES_T, tri[0], tri[2]);
        tri += 3;
    }
}

par_shapes_mesh* par_shapes_create_icosahedron()
{
    static float verts[] = {
        0.000,  0.000,  1.000,
        0.894,  0.000,  0.447,
        0.276,  0.851,  0.447,
        -0.724,  0.526,  0.447,
        -0.724, -0.526,  0.447,
        0.276, -0.851,  0.447,
        0.724,  0.526, -0.447,
        -0.276,  0.851, -0.447,
        -0.894,  0.000, -0.447,
        -0.276, -0.851, -0.447,
        0.724, -0.526, -0.447,
        0.000,  0.000, -1.000
    };
    static PAR_SHAPES_T faces[] = {
        0,1,2,
        0,2,3,
        0,3,4,
        0,4,5,
        0,5,1,
        7,6,11,
        8,7,11,
        9,8,11,
        10,9,11,
        6,10,11,
        6,2,1,
        7,3,2,
        8,4,3,
        9,5,4,
        10,1,5,
        6,7,2,
        7,8,3,
        8,9,4,
        9,10,5,
        10,6,1
    };
    par_shapes_mesh* mesh = PAR_CALLOC(par_shapes_mesh, 1);
    mesh->npoints = sizeof(verts) / sizeof(verts[0]) / 3;
    mesh->points = PAR_MALLOC(float, sizeof(verts) / 4);
    memcpy(mesh->points, verts, sizeof(verts));
    mesh->ntriangles = sizeof(faces) / sizeof(faces[0]) / 3;
    mesh->triangles = PAR_MALLOC(PAR_SHAPES_T, sizeof(faces) / 2);
    memcpy(mesh->triangles, faces, sizeof(faces));
    return mesh;
}

par_shapes_mesh* par_shapes_create_dodecahedron()
{
    static float verts[20 * 3] = {
        0.607, 0.000, 0.795,
        0.188, 0.577, 0.795,
        -0.491, 0.357, 0.795,
        -0.491, -0.357, 0.795,
        0.188, -0.577, 0.795,
        0.982, 0.000, 0.188,
        0.304, 0.934, 0.188,
        -0.795, 0.577, 0.188,
        -0.795, -0.577, 0.188,
        0.304, -0.934, 0.188,
        0.795, 0.577, -0.188,
        -0.304, 0.934, -0.188,
        -0.982, 0.000, -0.188,
        -0.304, -0.934, -0.188,
        0.795, -0.577, -0.188,
        0.491, 0.357, -0.795,
        -0.188, 0.577, -0.795,
        -0.607, 0.000, -0.795,
        -0.188, -0.577, -0.795,
        0.491, -0.357, -0.795,
    };
    static PAR_SHAPES_T pentagons[12 * 5] = {
        0,1,2,3,4,
        5,10,6,1,0,
        6,11,7,2,1,
        7,12,8,3,2,
        8,13,9,4,3,
        9,14,5,0,4,
        15,16,11,6,10,
        16,17,12,7,11,
        17,18,13,8,12,
        18,19,14,9,13,
        19,15,10,5,14,
        19,18,17,16,15
    };
    int npentagons = sizeof(pentagons) / sizeof(pentagons[0]) / 5;
    par_shapes_mesh* mesh = PAR_CALLOC(par_shapes_mesh, 1);
    int ncorners = sizeof(verts) / sizeof(verts[0]) / 3;
    mesh->npoints = ncorners;
    mesh->points = PAR_MALLOC(float, mesh->npoints * 3);
    memcpy(mesh->points, verts, sizeof(verts));
    PAR_SHAPES_T const* pentagon = pentagons;
    mesh->ntriangles = npentagons * 3;
    mesh->triangles = PAR_MALLOC(PAR_SHAPES_T, mesh->ntriangles * 3);
    PAR_SHAPES_T* tris = mesh->triangles;
    for (int p = 0; p < npentagons; p++, pentagon += 5) {
        *tris++ = pentagon[0];
        *tris++ = pentagon[1];
        *tris++ = pentagon[2];
        *tris++ = pentagon[0];
        *tris++ = pentagon[2];
        *tris++ = pentagon[3];
        *tris++ = pentagon[0];
        *tris++ = pentagon[3];
        *tris++ = pentagon[4];
    }
    return mesh;
}

par_shapes_mesh* par_shapes_create_octahedron()
{
    static float verts[6 * 3] = {
        0.000, 0.000, 1.000,
        1.000, 0.000, 0.000,
        0.000, 1.000, 0.000,
        -1.000, 0.000, 0.000,
        0.000, -1.000, 0.000,
        0.000, 0.000, -1.000
    };
    static PAR_SHAPES_T triangles[8 * 3] = {
        0,1,2,
        0,2,3,
        0,3,4,
        0,4,1,
        2,1,5,
        3,2,5,
        4,3,5,
        1,4,5,
    };
    int ntris = sizeof(triangles) / sizeof(triangles[0]) / 3;
    par_shapes_mesh* mesh = PAR_CALLOC(par_shapes_mesh, 1);
    int ncorners = sizeof(verts) / sizeof(verts[0]) / 3;
    mesh->npoints = ncorners;
    mesh->points = PAR_MALLOC(float, mesh->npoints * 3);
    memcpy(mesh->points, verts, sizeof(verts));
    PAR_SHAPES_T const* triangle = triangles;
    mesh->ntriangles = ntris;
    mesh->triangles = PAR_MALLOC(PAR_SHAPES_T, mesh->ntriangles * 3);
    PAR_SHAPES_T* tris = mesh->triangles;
    for (int p = 0; p < ntris; p++) {
        *tris++ = *triangle++;
        *tris++ = *triangle++;
        *tris++ = *triangle++;
    }
    return mesh;
}

par_shapes_mesh* par_shapes_create_tetrahedron()
{
    static float verts[4 * 3] = {
        0.000, 1.333, 0,
        0.943, 0, 0,
        -0.471, 0, 0.816,
        -0.471, 0, -0.816,
    };
    static PAR_SHAPES_T triangles[4 * 3] = {
        2,1,0,
        3,2,0,
        1,3,0,
        1,2,3,
    };
    int ntris = sizeof(triangles) / sizeof(triangles[0]) / 3;
    par_shapes_mesh* mesh = PAR_CALLOC(par_shapes_mesh, 1);
    int ncorners = sizeof(verts) / sizeof(verts[0]) / 3;
    mesh->npoints = ncorners;
    mesh->points = PAR_MALLOC(float, mesh->npoints * 3);
    memcpy(mesh->points, verts, sizeof(verts));
    PAR_SHAPES_T const* triangle = triangles;
    mesh->ntriangles = ntris;
    mesh->triangles = PAR_MALLOC(PAR_SHAPES_T, mesh->ntriangles * 3);
    PAR_SHAPES_T* tris = mesh->triangles;
    for (int p = 0; p < ntris; p++) {
        *tris++ = *triangle++;
        *tris++ = *triangle++;
        *tris++ = *triangle++;
    }
    return mesh;
}

par_shapes_mesh* par_shapes_create_cube()
{
    static float verts[8 * 3] = {
        0, 0, 0, // 0
        0, 1, 0, // 1
        1, 1, 0, // 2
        1, 0, 0, // 3
        0, 0, 1, // 4
        0, 1, 1, // 5
        1, 1, 1, // 6
        1, 0, 1, // 7
    };
    static PAR_SHAPES_T quads[6 * 4] = {
        7,6,5,4, // front
        0,1,2,3, // back
        6,7,3,2, // right
        5,6,2,1, // top
        4,5,1,0, // left
        7,4,0,3, // bottom
    };
    int nquads = sizeof(quads) / sizeof(quads[0]) / 4;
    par_shapes_mesh* mesh = PAR_CALLOC(par_shapes_mesh, 1);
    int ncorners = sizeof(verts) / sizeof(verts[0]) / 3;
    mesh->npoints = ncorners;
    mesh->points = PAR_MALLOC(float, mesh->npoints * 3);
    memcpy(mesh->points, verts, sizeof(verts));
    PAR_SHAPES_T const* quad = quads;
    mesh->ntriangles = nquads * 2;
    mesh->triangles = PAR_MALLOC(PAR_SHAPES_T, mesh->ntriangles * 3);
    PAR_SHAPES_T* tris = mesh->triangles;
    for (int p = 0; p < nquads; p++, quad += 4) {
        *tris++ = quad[0];
        *tris++ = quad[1];
        *tris++ = quad[2];
        *tris++ = quad[2];
        *tris++ = quad[3];
        *tris++ = quad[0];
    }
    return mesh;
}

typedef struct {
    char* cmd;
    char* arg;
} par_shapes__command;

typedef struct {
    char const* name;
    int weight;
    int ncommands;
    par_shapes__command* commands;
} par_shapes__rule;

typedef struct {
    int pc;
    float position[3];
    float scale[3];
    par_shapes_mesh* orientation;
    par_shapes__rule* rule;
} par_shapes__stackframe;

static par_shapes__rule* par_shapes__pick_rule(const char* name,
    par_shapes__rule* rules, int nrules)
{
    par_shapes__rule* rule = 0;
    int total = 0;
    for (int i = 0; i < nrules; i++) {
        rule = rules + i;
        if (!strcmp(rule->name, name)) {
            total += rule->weight;
        }
    }
    float r = (float) rand() / RAND_MAX;
    float t = 0;
    for (int i = 0; i < nrules; i++) {
        rule = rules + i;
        if (!strcmp(rule->name, name)) {
            t += (float) rule->weight / total;
            if (t >= r) {
                return rule;
            }
        }
    }
    return rule;
}

static par_shapes_mesh* par_shapes__create_turtle()
{
    const float xaxis[] = {1, 0, 0};
    const float yaxis[] = {0, 1, 0};
    const float zaxis[] = {0, 0, 1};
    par_shapes_mesh* turtle = PAR_CALLOC(par_shapes_mesh, 1);
    turtle->npoints = 3;
    turtle->points = PAR_CALLOC(float, turtle->npoints * 3);
    par_shapes__copy3(turtle->points + 0, xaxis);
    par_shapes__copy3(turtle->points + 3, yaxis);
    par_shapes__copy3(turtle->points + 6, zaxis);
    return turtle;
}

static par_shapes_mesh* par_shapes__apply_turtle(par_shapes_mesh* mesh,
    par_shapes_mesh* turtle, float const* pos, float const* scale)
{
    par_shapes_mesh* m = par_shapes_clone(mesh, 0);
    for (int p = 0; p < m->npoints; p++) {
        float* pt = m->points + p * 3;
        pt[0] *= scale[0];
        pt[1] *= scale[1];
        pt[2] *= scale[2];
        par_shapes__transform3(pt,
            turtle->points + 0, turtle->points + 3, turtle->points + 6);
        pt[0] += pos[0];
        pt[1] += pos[1];
        pt[2] += pos[2];
    }
    return m;
}

static void par_shapes__connect(par_shapes_mesh* scene,
    par_shapes_mesh* cylinder, int slices)
{
    int stacks = 1;
    int npoints = (slices + 1) * (stacks + 1);
    assert(scene->npoints >= npoints && "Cannot connect to empty scene.");

    // Create the new point list.
    npoints = scene->npoints + (slices + 1);
    float* points = PAR_MALLOC(float, npoints * 3);
    memcpy(points, scene->points, sizeof(float) * scene->npoints * 3);
    float* newpts = points + scene->npoints * 3;
    memcpy(newpts, cylinder->points + (slices + 1) * 3,
        sizeof(float) * (slices + 1) * 3);
    PAR_FREE(scene->points);
    scene->points = points;

    // Create the new triangle list.
    int ntriangles = scene->ntriangles + 2 * slices * stacks;
    PAR_SHAPES_T* triangles = PAR_MALLOC(PAR_SHAPES_T, ntriangles * 3);
    memcpy(triangles, scene->triangles, 2 * scene->ntriangles * 3);
    int v = scene->npoints - (slices + 1);
    PAR_SHAPES_T* face = triangles + scene->ntriangles * 3;
    for (int stack = 0; stack < stacks; stack++) {
        for (int slice = 0; slice < slices; slice++) {
            int next = slice + 1;
            *face++ = v + slice + slices + 1;
            *face++ = v + next;
            *face++ = v + slice;
            *face++ = v + slice + slices + 1;
            *face++ = v + next + slices + 1;
            *face++ = v + next;
        }
        v += slices + 1;
    }
    PAR_FREE(scene->triangles);
    scene->triangles = triangles;

    scene->npoints = npoints;
    scene->ntriangles = ntriangles;
}

par_shapes_mesh* par_shapes_create_lsystem(char const* text, int slices,
    int maxdepth)
{
    char* program;
    program = PAR_MALLOC(char, strlen(text) + 1);

    // The first pass counts the number of rules and commands.
    strcpy(program, text);
    char *cmd = strtok(program, " ");
    int nrules = 1;
    int ncommands = 0;
    while (cmd) {
        char *arg = strtok(0, " ");
        if (!arg) {
            //puts("lsystem error: unexpected end of program.");
            break;
        }
        if (!strcmp(cmd, "rule")) {
            nrules++;
        } else {
            ncommands++;
        }
        cmd = strtok(0, " ");
    }

    // Allocate space.
    par_shapes__rule* rules = PAR_MALLOC(par_shapes__rule, nrules);
    par_shapes__command* commands = PAR_MALLOC(par_shapes__command, ncommands);

    // Initialize the entry rule.
    par_shapes__rule* current_rule = &rules[0];
    par_shapes__command* current_command = &commands[0];
    current_rule->name = "entry";
    current_rule->weight = 1;
    current_rule->ncommands = 0;
    current_rule->commands = current_command;

    // The second pass fills in the structures.
    strcpy(program, text);
    cmd = strtok(program, " ");
    while (cmd) {
        char *arg = strtok(0, " ");
        if (!strcmp(cmd, "rule")) {
            current_rule++;

            // Split the argument into a rule name and weight.
            char* dot = strchr(arg, '.');
            if (dot) {
                current_rule->weight = atoi(dot + 1);
                *dot = 0;
            } else {
                current_rule->weight = 1;
            }

            current_rule->name = arg;
            current_rule->ncommands = 0;
            current_rule->commands = current_command;
        } else {
            current_rule->ncommands++;
            current_command->cmd = cmd;
            current_command->arg = arg;
            current_command++;
        }
        cmd = strtok(0, " ");
    }

    // For testing purposes, dump out the parsed program.
    #ifdef TEST_PARSE
    /*
    for (int i = 0; i < nrules; i++) {
        par_shapes__rule rule = rules[i];
        printf("rule %s.%d\n", rule.name, rule.weight);
        for (int c = 0; c < rule.ncommands; c++) {
            par_shapes__command cmd = rule.commands[c];
            printf("\t%s %s\n", cmd.cmd, cmd.arg);
        }
    }
    */
    #endif

    // Instantiate the aggregated shape and the template shapes.
    par_shapes_mesh* scene = PAR_CALLOC(par_shapes_mesh, 1);
    par_shapes_mesh* tube = par_shapes_create_cylinder(slices, 1);
    par_shapes_mesh* turtle = par_shapes__create_turtle();

    // We're not attempting to support texture coordinates and normals
    // with L-systems, so remove them from the template shape.
    PAR_FREE(tube->normals);
    PAR_FREE(tube->tcoords);
    tube->normals = 0;
    tube->tcoords = 0;

    const float xaxis[] = {1, 0, 0};
    const float yaxis[] = {0, 1, 0};
    const float zaxis[] = {0, 0, 1};
    const float units[] = {1, 1, 1};

    // Execute the L-system program until the stack size is 0.
    par_shapes__stackframe* stack =
        PAR_CALLOC(par_shapes__stackframe, maxdepth);
    int stackptr = 0;
    stack[0].orientation = turtle;
    stack[0].rule = &rules[0];
    par_shapes__copy3(stack[0].scale, units);
    while (stackptr >= 0) {
        par_shapes__stackframe* frame = &stack[stackptr];
        par_shapes__rule* rule = frame->rule;
        par_shapes_mesh* turtle = frame->orientation;
        float* position = frame->position;
        float* scale = frame->scale;
        if (frame->pc >= rule->ncommands) {
            par_shapes_free_mesh(turtle);
            stackptr--;
            continue;
        }

        par_shapes__command* cmd = rule->commands + (frame->pc++);
        #ifdef DUMP_TRACE
        //printf("%5s %5s %5s:%d  %03d\n", cmd->cmd, cmd->arg, rule->name, frame->pc - 1, stackptr);
        #endif

        float value;
        if (!strcmp(cmd->cmd, "shape")) {
            par_shapes_mesh* m = par_shapes__apply_turtle(tube, turtle,
                position, scale);
            if (!strcmp(cmd->arg, "connect")) {
                par_shapes__connect(scene, m, slices);
            } else {
                par_shapes_merge(scene, m);
            }
            par_shapes_free_mesh(m);
        } else if (!strcmp(cmd->cmd, "call") && stackptr < maxdepth - 1) {
            rule = par_shapes__pick_rule(cmd->arg, rules, nrules);
            frame = &stack[++stackptr];
            frame->rule = rule;
            frame->orientation = par_shapes_clone(turtle, 0);
            frame->pc = 0;
            par_shapes__copy3(frame->scale, scale);
            par_shapes__copy3(frame->position, position);
            continue;
        } else {
            value = atof(cmd->arg);
            if (!strcmp(cmd->cmd, "rx")) {
                par_shapes_rotate(turtle, value * PAR_PI / 180.0, xaxis);
            } else if (!strcmp(cmd->cmd, "ry")) {
                par_shapes_rotate(turtle, value * PAR_PI / 180.0, yaxis);
            } else if (!strcmp(cmd->cmd, "rz")) {
                par_shapes_rotate(turtle, value * PAR_PI / 180.0, zaxis);
            } else if (!strcmp(cmd->cmd, "tx")) {
                float vec[3] = {value, 0, 0};
                float t[3] = {
                    par_shapes__dot3(turtle->points + 0, vec),
                    par_shapes__dot3(turtle->points + 3, vec),
                    par_shapes__dot3(turtle->points + 6, vec)
                };
                par_shapes__add3(position, t);
            } else if (!strcmp(cmd->cmd, "ty")) {
                float vec[3] = {0, value, 0};
                float t[3] = {
                    par_shapes__dot3(turtle->points + 0, vec),
                    par_shapes__dot3(turtle->points + 3, vec),
                    par_shapes__dot3(turtle->points + 6, vec)
                };
                par_shapes__add3(position, t);
            } else if (!strcmp(cmd->cmd, "tz")) {
                float vec[3] = {0, 0, value};
                float t[3] = {
                    par_shapes__dot3(turtle->points + 0, vec),
                    par_shapes__dot3(turtle->points + 3, vec),
                    par_shapes__dot3(turtle->points + 6, vec)
                };
                par_shapes__add3(position, t);
            } else if (!strcmp(cmd->cmd, "sx")) {
                scale[0] *= value;
            } else if (!strcmp(cmd->cmd, "sy")) {
                scale[1] *= value;
            } else if (!strcmp(cmd->cmd, "sz")) {
                scale[2] *= value;
            } else if (!strcmp(cmd->cmd, "sa")) {
                scale[0] *= value;
                scale[1] *= value;
                scale[2] *= value;
            }
        }
    }
    PAR_FREE(stack);
    PAR_FREE(program);
    PAR_FREE(rules);
    PAR_FREE(commands);
    return scene;
}

void par_shapes_unweld(par_shapes_mesh* mesh, bool create_indices)
{
    int npoints = mesh->ntriangles * 3;
    float* points = PAR_MALLOC(float, 3 * npoints);
    float* dst = points;
    PAR_SHAPES_T const* index = mesh->triangles;
    for (int i = 0; i < npoints; i++) {
        float const* src = mesh->points + 3 * (*index++);
        *dst++ = src[0];
        *dst++ = src[1];
        *dst++ = src[2];
    }
    PAR_FREE(mesh->points);
    mesh->points = points;
    mesh->npoints = npoints;
    if (create_indices) {
        PAR_SHAPES_T* tris = PAR_MALLOC(PAR_SHAPES_T, 3 * mesh->ntriangles);
        PAR_SHAPES_T* index = tris;
        for (int i = 0; i < mesh->ntriangles * 3; i++) {
            *index++ = i;
        }
        PAR_FREE(mesh->triangles);
        mesh->triangles = tris;
    }
}

void par_shapes_compute_normals(par_shapes_mesh* m)
{
    PAR_FREE(m->normals);
    m->normals = PAR_CALLOC(float, m->npoints * 3);
    PAR_SHAPES_T const* triangle = m->triangles;
    float next[3], prev[3], cp[3];
    for (int f = 0; f < m->ntriangles; f++, triangle += 3) {
        float const* pa = m->points + 3 * triangle[0];
        float const* pb = m->points + 3 * triangle[1];
        float const* pc = m->points + 3 * triangle[2];
        par_shapes__copy3(next, pb);
        par_shapes__subtract3(next, pa);
        par_shapes__copy3(prev, pc);
        par_shapes__subtract3(prev, pa);
        par_shapes__cross3(cp, next, prev);
        par_shapes__add3(m->normals + 3 * triangle[0], cp);
        par_shapes__copy3(next, pc);
        par_shapes__subtract3(next, pb);
        par_shapes__copy3(prev, pa);
        par_shapes__subtract3(prev, pb);
        par_shapes__cross3(cp, next, prev);
        par_shapes__add3(m->normals + 3 * triangle[1], cp);
        par_shapes__copy3(next, pa);
        par_shapes__subtract3(next, pc);
        par_shapes__copy3(prev, pb);
        par_shapes__subtract3(prev, pc);
        par_shapes__cross3(cp, next, prev);
        par_shapes__add3(m->normals + 3 * triangle[2], cp);
    }
    float* normal = m->normals;
    for (int p = 0; p < m->npoints; p++, normal += 3) {
        par_shapes__normalize3(normal);
    }
}

static void par_shapes__subdivide(par_shapes_mesh* mesh)
{
    assert(mesh->npoints == mesh->ntriangles * 3 && "Must be unwelded.");
    int ntriangles = mesh->ntriangles * 4;
    int npoints = ntriangles * 3;
    float* points = PAR_CALLOC(float, npoints * 3);
    float* dpoint = points;
    float const* spoint = mesh->points;
    for (int t = 0; t < mesh->ntriangles; t++, spoint += 9, dpoint += 3) {
        float const* a = spoint;
        float const* b = spoint + 3;
        float const* c = spoint + 6;
        float const* p0 = dpoint;
        float const* p1 = dpoint + 3;
        float const* p2 = dpoint + 6;
        par_shapes__mix3(dpoint, a, b, 0.5);
        par_shapes__mix3(dpoint += 3, b, c, 0.5);
        par_shapes__mix3(dpoint += 3, a, c, 0.5);
        par_shapes__add3(dpoint += 3, a);
        par_shapes__add3(dpoint += 3, p0);
        par_shapes__add3(dpoint += 3, p2);
        par_shapes__add3(dpoint += 3, p0);
        par_shapes__add3(dpoint += 3, b);
        par_shapes__add3(dpoint += 3, p1);
        par_shapes__add3(dpoint += 3, p2);
        par_shapes__add3(dpoint += 3, p1);
        par_shapes__add3(dpoint += 3, c);
    }
    PAR_FREE(mesh->points);
    mesh->points = points;
    mesh->npoints = npoints;
    mesh->ntriangles = ntriangles;
}

par_shapes_mesh* par_shapes_create_subdivided_sphere(int nsubd)
{
    par_shapes_mesh* mesh = par_shapes_create_icosahedron();
    par_shapes_unweld(mesh, false);
    PAR_FREE(mesh->triangles);
    mesh->triangles = 0;
    while (nsubd--) {
        par_shapes__subdivide(mesh);
    }
    for (int i = 0; i < mesh->npoints; i++) {
        par_shapes__normalize3(mesh->points + i * 3);
    }
    mesh->triangles = PAR_MALLOC(PAR_SHAPES_T, 3 * mesh->ntriangles);
    for (int i = 0; i < mesh->ntriangles * 3; i++) {
        mesh->triangles[i] = i;
    }
    par_shapes_mesh* tmp = mesh;
    mesh = par_shapes_weld(mesh, 0.01, 0);
    par_shapes_free_mesh(tmp);
    par_shapes_compute_normals(mesh);
    return mesh;
}

par_shapes_mesh* par_shapes_create_rock(int seed, int subd)
{
    par_shapes_mesh* mesh = par_shapes_create_subdivided_sphere(subd);
    struct osn_context* ctx;
    par__simplex_noise(seed, &ctx);
    for (int p = 0; p < mesh->npoints; p++) {
        float* pt = mesh->points + p * 3;
        float a = 0.25, f = 1.0;
        double n = a * par__simplex_noise2(ctx, f * pt[0], f * pt[2]);
        a *= 0.5; f *= 2;
        n += a * par__simplex_noise2(ctx, f * pt[0], f * pt[2]);
        pt[0] *= 1 + 2 * n;
        pt[1] *= 1 + n;
        pt[2] *= 1 + 2 * n;
        if (pt[1] < 0) {
            pt[1] = -pow(-pt[1], 0.5) / 2;
        }
    }
    par__simplex_noise_free(ctx);
    par_shapes_compute_normals(mesh);
    return mesh;
}

par_shapes_mesh* par_shapes_clone(par_shapes_mesh const* mesh,
    par_shapes_mesh* clone)
{
    if (!clone) {
        clone = PAR_CALLOC(par_shapes_mesh, 1);
    }
    clone->npoints = mesh->npoints;
    clone->points = PAR_REALLOC(float, clone->points, 3 * clone->npoints);
    memcpy(clone->points, mesh->points, sizeof(float) * 3 * clone->npoints);
    clone->ntriangles = mesh->ntriangles;
    clone->triangles = PAR_REALLOC(PAR_SHAPES_T, clone->triangles, 3 *
        clone->ntriangles);
    memcpy(clone->triangles, mesh->triangles,
        sizeof(PAR_SHAPES_T) * 3 * clone->ntriangles);
    if (mesh->normals) {
        clone->normals = PAR_REALLOC(float, clone->normals, 3 * clone->npoints);
        memcpy(clone->normals, mesh->normals,
            sizeof(float) * 3 * clone->npoints);
    }
    if (mesh->tcoords) {
        clone->tcoords = PAR_REALLOC(float, clone->tcoords, 2 * clone->npoints);
        memcpy(clone->tcoords, mesh->tcoords,
            sizeof(float) * 2 * clone->npoints);
    }
    return clone;
}

static struct {
    float const* points;
    int gridsize;
} par_shapes__sort_context;

static int par_shapes__cmp1(const void *arg0, const void *arg1)
{
    const int g = par_shapes__sort_context.gridsize;

    // Convert arg0 into a flattened grid index.
    PAR_SHAPES_T d0 = *(const PAR_SHAPES_T*) arg0;
    float const* p0 = par_shapes__sort_context.points + d0 * 3;
    int i0 = (int) p0[0];
    int j0 = (int) p0[1];
    int k0 = (int) p0[2];
    int index0 = i0 + g * j0 + g * g * k0;

    // Convert arg1 into a flattened grid index.
    PAR_SHAPES_T d1 = *(const PAR_SHAPES_T*) arg1;
    float const* p1 = par_shapes__sort_context.points + d1 * 3;
    int i1 = (int) p1[0];
    int j1 = (int) p1[1];
    int k1 = (int) p1[2];
    int index1 = i1 + g * j1 + g * g * k1;

    // Return the ordering.
    if (index0 < index1) return -1;
    if (index0 > index1) return 1;
    return 0;
}

static void par_shapes__sort_points(par_shapes_mesh* mesh, int gridsize,
    PAR_SHAPES_T* sortmap)
{
    // Run qsort over a list of consecutive integers that get deferenced
    // within the comparator function; this creates a reorder mapping.
    for (int i = 0; i < mesh->npoints; i++) {
        sortmap[i] = i;
    }
    par_shapes__sort_context.gridsize = gridsize;
    par_shapes__sort_context.points = mesh->points;
    qsort(sortmap, mesh->npoints, sizeof(PAR_SHAPES_T), par_shapes__cmp1);

    // Apply the reorder mapping to the XYZ coordinate data.
    float* newpts = PAR_MALLOC(float, mesh->npoints * 3);
    PAR_SHAPES_T* invmap = PAR_MALLOC(PAR_SHAPES_T, mesh->npoints);
    float* dstpt = newpts;
    for (int i = 0; i < mesh->npoints; i++) {
        invmap[sortmap[i]] = i;
        float const* srcpt = mesh->points + 3 * sortmap[i];
        *dstpt++ = *srcpt++;
        *dstpt++ = *srcpt++;
        *dstpt++ = *srcpt++;
    }
    PAR_FREE(mesh->points);
    mesh->points = newpts;

    // Apply the inverse reorder mapping to the triangle indices.
    PAR_SHAPES_T* newinds = PAR_MALLOC(PAR_SHAPES_T, mesh->ntriangles * 3);
    PAR_SHAPES_T* dstind = newinds;
    PAR_SHAPES_T const* srcind = mesh->triangles;
    for (int i = 0; i < mesh->ntriangles * 3; i++) {
        *dstind++ = invmap[*srcind++];
    }
    PAR_FREE(mesh->triangles);
    mesh->triangles = newinds;

    // Cleanup.
    memcpy(sortmap, invmap, sizeof(PAR_SHAPES_T) * mesh->npoints);
    PAR_FREE(invmap);
}

static void par_shapes__weld_points(par_shapes_mesh* mesh, int gridsize,
    float epsilon, PAR_SHAPES_T* weldmap)
{
    // Each bin contains a "pointer" (really an index) to its first point.
    // We add 1 because 0 is reserved to mean that the bin is empty.
    // Since the points are spatially sorted, there's no need to store
    // a point count in each bin.
    PAR_SHAPES_T* bins = PAR_CALLOC(PAR_SHAPES_T,
        gridsize * gridsize * gridsize);
    int prev_binindex = -1;
    for (int p = 0; p < mesh->npoints; p++) {
        float const* pt = mesh->points + p * 3;
        int i = (int) pt[0];
        int j = (int) pt[1];
        int k = (int) pt[2];
        int this_binindex = i + gridsize * j + gridsize * gridsize * k;
        if (this_binindex != prev_binindex) {
            bins[this_binindex] = 1 + p;
        }
        prev_binindex = this_binindex;
    }

    // Examine all bins that intersect the epsilon-sized cube centered at each
    // point, and check for colocated points within those bins.
    float const* pt = mesh->points;
    int nremoved = 0;
    for (int p = 0; p < mesh->npoints; p++, pt += 3) {

        // Skip if this point has already been welded.
        if (weldmap[p] != p) {
            continue;
        }

        // Build a list of bins that intersect the epsilon-sized cube.
        int nearby[8];
        int nbins = 0;
        int minp[3], maxp[3];
        for (int c = 0; c < 3; c++) {
            minp[c] = (int) (pt[c] - epsilon);
            maxp[c] = (int) (pt[c] + epsilon);
        }
        for (int i = minp[0]; i <= maxp[0]; i++) {
            for (int j = minp[1]; j <= maxp[1]; j++) {
                for (int k = minp[2]; k <= maxp[2]; k++) {
                    int binindex = i + gridsize * j + gridsize * gridsize * k;
                    PAR_SHAPES_T binvalue = *(bins + binindex);
                    if (binvalue > 0) {
                        if (nbins == 8) {
                            //printf("Epsilon value is too large.\n");
                            break;
                        }
                        nearby[nbins++] = binindex;
                    }
                }
            }
        }

        // Check for colocated points in each nearby bin.
        for (int b = 0; b < nbins; b++) {
            int binindex = nearby[b];
            PAR_SHAPES_T binvalue = *(bins + binindex);
            PAR_SHAPES_T nindex = binvalue - 1;
            while (true) {

                // If this isn't "self" and it's colocated, then weld it!
                if (nindex != p && weldmap[nindex] == nindex) {
                    float const* thatpt = mesh->points + nindex * 3;
                    float dist2 = par_shapes__sqrdist3(thatpt, pt);
                    if (dist2 < epsilon) {
                        weldmap[nindex] = p;
                        nremoved++;
                    }
                }

                // Advance to the next point if possible.
                if (++nindex >= mesh->npoints) {
                    break;
                }

                // If the next point is outside the bin, then we're done.
                float const* nextpt = mesh->points + nindex * 3;
                int i = (int) nextpt[0];
                int j = (int) nextpt[1];
                int k = (int) nextpt[2];
                int nextbinindex = i + gridsize * j + gridsize * gridsize * k;
                if (nextbinindex != binindex) {
                    break;
                }
            }
        }
    }
    PAR_FREE(bins);

    // Apply the weldmap to the vertices.
    int npoints = mesh->npoints - nremoved;
    float* newpts = PAR_MALLOC(float, 3 * npoints);
    float* dst = newpts;
    PAR_SHAPES_T* condensed_map = PAR_MALLOC(PAR_SHAPES_T, mesh->npoints);
    PAR_SHAPES_T* cmap = condensed_map;
    float const* src = mesh->points;
    int ci = 0;
    for (int p = 0; p < mesh->npoints; p++, src += 3) {
        if (weldmap[p] == p) {
            *dst++ = src[0];
            *dst++ = src[1];
            *dst++ = src[2];
            *cmap++ = ci++;
        } else {
            *cmap++ = condensed_map[weldmap[p]];
        }
    }
    assert(ci == npoints);
    PAR_FREE(mesh->points);
    memcpy(weldmap, condensed_map, mesh->npoints * sizeof(PAR_SHAPES_T));
    PAR_FREE(condensed_map);
    mesh->points = newpts;
    mesh->npoints = npoints;

    // Apply the weldmap to the triangle indices and skip the degenerates.
    PAR_SHAPES_T const* tsrc = mesh->triangles;
    PAR_SHAPES_T* tdst = mesh->triangles;
    int ntriangles = 0;
    for (int i = 0; i < mesh->ntriangles; i++, tsrc += 3) {
        PAR_SHAPES_T a = weldmap[tsrc[0]];
        PAR_SHAPES_T b = weldmap[tsrc[1]];
        PAR_SHAPES_T c = weldmap[tsrc[2]];
        if (a != b && a != c && b != c) {
            *tdst++ = a;
            *tdst++ = b;
            *tdst++ = c;
            ntriangles++;
        }
    }
    mesh->ntriangles = ntriangles;
}

par_shapes_mesh* par_shapes_weld(par_shapes_mesh const* mesh, float epsilon,
    PAR_SHAPES_T* weldmap)
{
    par_shapes_mesh* clone = par_shapes_clone(mesh, 0);
    float aabb[6];
    int gridsize = 20;
    float maxcell = gridsize - 1;
    par_shapes_compute_aabb(clone, aabb);
    float scale[3] = {
        aabb[3] == aabb[0] ? 1.0f : maxcell / (aabb[3] - aabb[0]),
        aabb[4] == aabb[1] ? 1.0f : maxcell / (aabb[4] - aabb[1]),
        aabb[5] == aabb[2] ? 1.0f : maxcell / (aabb[5] - aabb[2]),
    };
    par_shapes_translate(clone, -aabb[0], -aabb[1], -aabb[2]);
    par_shapes_scale(clone, scale[0], scale[1], scale[2]);
    PAR_SHAPES_T* sortmap = PAR_MALLOC(PAR_SHAPES_T, mesh->npoints);
    par_shapes__sort_points(clone, gridsize, sortmap);
    bool owner = false;
    if (!weldmap) {
        owner = true;
        weldmap = PAR_MALLOC(PAR_SHAPES_T, mesh->npoints);
    }
    for (int i = 0; i < mesh->npoints; i++) {
        weldmap[i] = i;
    }
    par_shapes__weld_points(clone, gridsize, epsilon, weldmap);
    if (owner) {
        PAR_FREE(weldmap);
    } else {
        PAR_SHAPES_T* newmap = PAR_MALLOC(PAR_SHAPES_T, mesh->npoints);
        for (int i = 0; i < mesh->npoints; i++) {
            newmap[i] = weldmap[sortmap[i]];
        }
        memcpy(weldmap, newmap, sizeof(PAR_SHAPES_T) * mesh->npoints);
        PAR_FREE(newmap);
    }
    PAR_FREE(sortmap);
    par_shapes_scale(clone, 1.0 / scale[0], 1.0 / scale[1], 1.0 / scale[2]);
    par_shapes_translate(clone, aabb[0], aabb[1], aabb[2]);
    return clone;
}

// -----------------------------------------------------------------------------
// BEGIN OPEN SIMPLEX NOISE
// -----------------------------------------------------------------------------

#define STRETCH_CONSTANT_2D (-0.211324865405187)  // (1 / sqrt(2 + 1) - 1 ) / 2;
#define SQUISH_CONSTANT_2D (0.366025403784439)  // (sqrt(2 + 1) -1) / 2;
#define STRETCH_CONSTANT_3D (-1.0 / 6.0)  // (1 / sqrt(3 + 1) - 1) / 3;
#define SQUISH_CONSTANT_3D (1.0 / 3.0)  // (sqrt(3+1)-1)/3;
#define STRETCH_CONSTANT_4D (-0.138196601125011)  // (1 / sqrt(4 + 1) - 1) / 4;
#define SQUISH_CONSTANT_4D (0.309016994374947)  // (sqrt(4 + 1) - 1) / 4;

#define NORM_CONSTANT_2D (47.0)
#define NORM_CONSTANT_3D (103.0)
#define NORM_CONSTANT_4D (30.0)

#define DEFAULT_SEED (0LL)

struct osn_context {
    int16_t* perm;
    int16_t* permGradIndex3D;
};

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

/*
 * Gradients for 2D. They approximate the directions to the
 * vertices of an octagon from the center.
 */
static const int8_t gradients2D[] = {
    5, 2, 2, 5, -5, 2, -2, 5, 5, -2, 2, -5, -5, -2, -2, -5,
};

/*
 * Gradients for 3D. They approximate the directions to the
 * vertices of a rhombicuboctahedron from the center, skewed so
 * that the triangular and square facets can be inscribed inside
 * circles of the same radius.
 */
static const signed char gradients3D[] = {
    -11, 4, 4, -4, 11, 4, -4, 4, 11, 11, 4, 4, 4, 11, 4, 4, 4, 11, -11, -4, 4,
    -4, -11, 4, -4, -4, 11, 11, -4, 4, 4, -11, 4, 4, -4, 11, -11, 4, -4, -4, 11,
    -4, -4, 4, -11, 11, 4, -4, 4, 11, -4, 4, 4, -11, -11, -4, -4, -4, -11, -4,
    -4, -4, -11, 11, -4, -4, 4, -11, -4, 4, -4, -11,
};

/*
 * Gradients for 4D. They approximate the directions to the
 * vertices of a disprismatotesseractihexadecachoron from the center,
 * skewed so that the tetrahedral and cubic facets can be inscribed inside
 * spheres of the same radius.
 */
static const signed char gradients4D[] = {
    3, 1, 1, 1, 1, 3, 1, 1, 1, 1, 3, 1, 1, 1, 1, 3, -3, 1, 1, 1, -1, 3, 1, 1,
    -1, 1, 3, 1, -1, 1, 1, 3, 3, -1, 1, 1, 1, -3, 1, 1, 1, -1, 3, 1, 1, -1, 1,
    3, -3, -1, 1, 1, -1, -3, 1, 1, -1, -1, 3, 1, -1, -1, 1, 3, 3, 1, -1, 1, 1,
    3, -1, 1, 1, 1, -3, 1, 1, 1, -1, 3, -3, 1, -1, 1, -1, 3, -1, 1, -1, 1, -3,
    1, -1, 1, -1, 3, 3, -1, -1, 1, 1, -3, -1, 1, 1, -1, -3, 1, 1, -1, -1, 3, -3,
    -1, -1, 1, -1, -3, -1, 1, -1, -1, -3, 1, -1, -1, -1, 3, 3, 1, 1, -1, 1, 3,
    1, -1, 1, 1, 3, -1, 1, 1, 1, -3, -3, 1, 1, -1, -1, 3, 1, -1, -1, 1, 3, -1,
    -1, 1, 1, -3, 3, -1, 1, -1, 1, -3, 1, -1, 1, -1, 3, -1, 1, -1, 1, -3, -3,
    -1, 1, -1, -1, -3, 1, -1, -1, -1, 3, -1, -1, -1, 1, -3, 3, 1, -1, -1, 1, 3,
    -1, -1, 1, 1, -3, -1, 1, 1, -1, -3, -3, 1, -1, -1, -1, 3, -1, -1, -1, 1, -3,
    -1, -1, 1, -1, -3, 3, -1, -1, -1, 1, -3, -1, -1, 1, -1, -3, -1, 1, -1, -1,
    -3, -3, -1, -1, -1, -1, -3, -1, -1, -1, -1, -3, -1, -1, -1, -1, -3,
};

static double extrapolate2(
    struct osn_context* ctx, int xsb, int ysb, double dx, double dy)
{
    int16_t* perm = ctx->perm;
    int index = perm[(perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
    return gradients2D[index] * dx + gradients2D[index + 1] * dy;
}

static inline int fastFloor(double x)
{
    int xi = (int) x;
    return x < xi ? xi - 1 : xi;
}

static int allocate_perm(struct osn_context* ctx, int nperm, int ngrad)
{
    PAR_FREE(ctx->perm);
    PAR_FREE(ctx->permGradIndex3D);
    ctx->perm = PAR_MALLOC(int16_t, nperm);
    if (!ctx->perm) {
        return -ENOMEM;
    }
    ctx->permGradIndex3D = PAR_MALLOC(int16_t, ngrad);
    if (!ctx->permGradIndex3D) {
        PAR_FREE(ctx->perm);
        return -ENOMEM;
    }
    return 0;
}

static int par__simplex_noise(int64_t seed, struct osn_context** ctx)
{
    int rc;
    int16_t source[256];
    int i;
    int16_t* perm;
    int16_t* permGradIndex3D;
    *ctx = PAR_MALLOC(struct osn_context, 1);
    if (!(*ctx)) {
        return -ENOMEM;
    }
    (*ctx)->perm = NULL;
    (*ctx)->permGradIndex3D = NULL;
    rc = allocate_perm(*ctx, 256, 256);
    if (rc) {
        PAR_FREE(*ctx);
        return rc;
    }
    perm = (*ctx)->perm;
    permGradIndex3D = (*ctx)->permGradIndex3D;
    for (i = 0; i < 256; i++) {
        source[i] = (int16_t) i;
    }
    seed = seed * 6364136223846793005LL + 1442695040888963407LL;
    seed = seed * 6364136223846793005LL + 1442695040888963407LL;
    seed = seed * 6364136223846793005LL + 1442695040888963407LL;
    for (i = 255; i >= 0; i--) {
        seed = seed * 6364136223846793005LL + 1442695040888963407LL;
        int r = (int) ((seed + 31) % (i + 1));
        if (r < 0)
            r += (i + 1);
        perm[i] = source[r];
        permGradIndex3D[i] =
            (short) ((perm[i] % (ARRAYSIZE(gradients3D) / 3)) * 3);
        source[r] = source[i];
    }
    return 0;
}

static void par__simplex_noise_free(struct osn_context* ctx)
{
    if (!ctx)
        return;
    if (ctx->perm) {
        PAR_FREE(ctx->perm);
        ctx->perm = NULL;
    }
    if (ctx->permGradIndex3D) {
        PAR_FREE(ctx->permGradIndex3D);
        ctx->permGradIndex3D = NULL;
    }
    PAR_FREE(ctx);
}

static double par__simplex_noise2(struct osn_context* ctx, double x, double y)
{
    // Place input coordinates onto grid.
    double stretchOffset = (x + y) * STRETCH_CONSTANT_2D;
    double xs = x + stretchOffset;
    double ys = y + stretchOffset;

    // Floor to get grid coordinates of rhombus (stretched square) super-cell
    // origin.
    int xsb = fastFloor(xs);
    int ysb = fastFloor(ys);

    // Skew out to get actual coordinates of rhombus origin. We'll need these
    // later.
    double squishOffset = (xsb + ysb) * SQUISH_CONSTANT_2D;
    double xb = xsb + squishOffset;
    double yb = ysb + squishOffset;

    // Compute grid coordinates relative to rhombus origin.
    double xins = xs - xsb;
    double yins = ys - ysb;

    // Sum those together to get a value that determines which region we're in.
    double inSum = xins + yins;

    // Positions relative to origin point.
    double dx0 = x - xb;
    double dy0 = y - yb;

    // We'll be defining these inside the next block and using them afterwards.
    double dx_ext, dy_ext;
    int xsv_ext, ysv_ext;

    double value = 0;

    // Contribution (1,0)
    double dx1 = dx0 - 1 - SQUISH_CONSTANT_2D;
    double dy1 = dy0 - 0 - SQUISH_CONSTANT_2D;
    double attn1 = 2 - dx1 * dx1 - dy1 * dy1;
    if (attn1 > 0) {
        attn1 *= attn1;
        value += attn1 * attn1 * extrapolate2(ctx, xsb + 1, ysb + 0, dx1, dy1);
    }

    // Contribution (0,1)
    double dx2 = dx0 - 0 - SQUISH_CONSTANT_2D;
    double dy2 = dy0 - 1 - SQUISH_CONSTANT_2D;
    double attn2 = 2 - dx2 * dx2 - dy2 * dy2;
    if (attn2 > 0) {
        attn2 *= attn2;
        value += attn2 * attn2 * extrapolate2(ctx, xsb + 0, ysb + 1, dx2, dy2);
    }

    if (inSum <= 1) {  // We're inside the triangle (2-Simplex) at (0,0)
        double zins = 1 - inSum;
        if (zins > xins || zins > yins) {
            if (xins > yins) {
                xsv_ext = xsb + 1;
                ysv_ext = ysb - 1;
                dx_ext = dx0 - 1;
                dy_ext = dy0 + 1;
            } else {
                xsv_ext = xsb - 1;
                ysv_ext = ysb + 1;
                dx_ext = dx0 + 1;
                dy_ext = dy0 - 1;
            }
        } else {  //(1,0) and (0,1) are the closest two vertices.
            xsv_ext = xsb + 1;
            ysv_ext = ysb + 1;
            dx_ext = dx0 - 1 - 2 * SQUISH_CONSTANT_2D;
            dy_ext = dy0 - 1 - 2 * SQUISH_CONSTANT_2D;
        }
    } else {  // We're inside the triangle (2-Simplex) at (1,1)
        double zins = 2 - inSum;
        if (zins < xins || zins < yins) {
            if (xins > yins) {
                xsv_ext = xsb + 2;
                ysv_ext = ysb + 0;
                dx_ext = dx0 - 2 - 2 * SQUISH_CONSTANT_2D;
                dy_ext = dy0 + 0 - 2 * SQUISH_CONSTANT_2D;
            } else {
                xsv_ext = xsb + 0;
                ysv_ext = ysb + 2;
                dx_ext = dx0 + 0 - 2 * SQUISH_CONSTANT_2D;
                dy_ext = dy0 - 2 - 2 * SQUISH_CONSTANT_2D;
            }
        } else {  //(1,0) and (0,1) are the closest two vertices.
            dx_ext = dx0;
            dy_ext = dy0;
            xsv_ext = xsb;
            ysv_ext = ysb;
        }
        xsb += 1;
        ysb += 1;
        dx0 = dx0 - 1 - 2 * SQUISH_CONSTANT_2D;
        dy0 = dy0 - 1 - 2 * SQUISH_CONSTANT_2D;
    }

    // Contribution (0,0) or (1,1)
    double attn0 = 2 - dx0 * dx0 - dy0 * dy0;
    if (attn0 > 0) {
        attn0 *= attn0;
        value += attn0 * attn0 * extrapolate2(ctx, xsb, ysb, dx0, dy0);
    }

    // Extra Vertex
    double attn_ext = 2 - dx_ext * dx_ext - dy_ext * dy_ext;
    if (attn_ext > 0) {
        attn_ext *= attn_ext;
        value += attn_ext * attn_ext *
            extrapolate2(ctx, xsv_ext, ysv_ext, dx_ext, dy_ext);
    }

    return value / NORM_CONSTANT_2D;
}

void par_shapes_remove_degenerate(par_shapes_mesh* mesh, float mintriarea)
{
    int ntriangles = 0;
    PAR_SHAPES_T* triangles = PAR_MALLOC(PAR_SHAPES_T, mesh->ntriangles * 3);
    PAR_SHAPES_T* dst = triangles;
    PAR_SHAPES_T const* src = mesh->triangles;
    float next[3], prev[3], cp[3];
    float mincplen2 = (mintriarea * 2) * (mintriarea * 2);
    for (int f = 0; f < mesh->ntriangles; f++, src += 3) {
        float const* pa = mesh->points + 3 * src[0];
        float const* pb = mesh->points + 3 * src[1];
        float const* pc = mesh->points + 3 * src[2];
        par_shapes__copy3(next, pb);
        par_shapes__subtract3(next, pa);
        par_shapes__copy3(prev, pc);
        par_shapes__subtract3(prev, pa);
        par_shapes__cross3(cp, next, prev);
        float cplen2 = par_shapes__dot3(cp, cp);
        if (cplen2 >= mincplen2) {
            *dst++ = src[0];
            *dst++ = src[1];
            *dst++ = src[2];
            ntriangles++;
        }
    }
    mesh->ntriangles = ntriangles;
    PAR_FREE(mesh->triangles);
    mesh->triangles = triangles;
}

#endif // PAR_SHAPES_IMPLEMENTATION
#endif // PAR_SHAPES_H
