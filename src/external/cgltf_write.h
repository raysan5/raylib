/**
 * cgltf_write - a single-file glTF 2.0 writer written in C99.
 *
 * Version: 1.15
 *
 * Website: https://github.com/jkuhlmann/cgltf
 *
 * Distributed under the MIT License, see notice at the end of this file.
 *
 * Building:
 * Include this file where you need the struct and function
 * declarations. Have exactly one source file where you define
 * `CGLTF_WRITE_IMPLEMENTATION` before including this file to get the
 * function definitions.
 *
 * Reference:
 * `cgltf_result cgltf_write_file(const cgltf_options* options, const char*
 * path, const cgltf_data* data)` writes a glTF data to the given file path.
 * If `options->type` is `cgltf_file_type_glb`, both JSON content and binary
 * buffer of the given glTF data will be written in a GLB format.
 * Otherwise, only the JSON part will be written.
 * External buffers and images are not written out. `data` is not deallocated.
 *
 * `cgltf_size cgltf_write(const cgltf_options* options, char* buffer,
 * cgltf_size size, const cgltf_data* data)` writes JSON into the given memory
 * buffer. Returns the number of bytes written to `buffer`, including a null
 * terminator. If buffer is null, returns the number of bytes that would have
 * been written. `data` is not deallocated.
 */
#ifndef CGLTF_WRITE_H_INCLUDED__
#define CGLTF_WRITE_H_INCLUDED__

#include "cgltf.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

cgltf_result cgltf_write_file(const cgltf_options* options, const char* path, const cgltf_data* data);
cgltf_size cgltf_write(const cgltf_options* options, char* buffer, cgltf_size size, const cgltf_data* data);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CGLTF_WRITE_H_INCLUDED__ */

/*
 *
 * Stop now, if you are only interested in the API.
 * Below, you find the implementation.
 *
 */

#if defined(__INTELLISENSE__) || defined(__JETBRAINS_IDE__)
/* This makes MSVC/CLion intellisense work. */
#define CGLTF_WRITE_IMPLEMENTATION
#endif

#ifdef CGLTF_WRITE_IMPLEMENTATION

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define CGLTF_EXTENSION_FLAG_TEXTURE_TRANSFORM      (1 << 0)
#define CGLTF_EXTENSION_FLAG_MATERIALS_UNLIT        (1 << 1)
#define CGLTF_EXTENSION_FLAG_SPECULAR_GLOSSINESS    (1 << 2)
#define CGLTF_EXTENSION_FLAG_LIGHTS_PUNCTUAL        (1 << 3)
#define CGLTF_EXTENSION_FLAG_DRACO_MESH_COMPRESSION (1 << 4)
#define CGLTF_EXTENSION_FLAG_MATERIALS_CLEARCOAT    (1 << 5)
#define CGLTF_EXTENSION_FLAG_MATERIALS_IOR          (1 << 6)
#define CGLTF_EXTENSION_FLAG_MATERIALS_SPECULAR     (1 << 7)
#define CGLTF_EXTENSION_FLAG_MATERIALS_TRANSMISSION (1 << 8)
#define CGLTF_EXTENSION_FLAG_MATERIALS_SHEEN        (1 << 9)
#define CGLTF_EXTENSION_FLAG_MATERIALS_VARIANTS     (1 << 10)
#define CGLTF_EXTENSION_FLAG_MATERIALS_VOLUME       (1 << 11)
#define CGLTF_EXTENSION_FLAG_TEXTURE_BASISU        (1 << 12)
#define CGLTF_EXTENSION_FLAG_MATERIALS_EMISSIVE_STRENGTH (1 << 13)
#define CGLTF_EXTENSION_FLAG_MESH_GPU_INSTANCING (1 << 14)
#define CGLTF_EXTENSION_FLAG_MATERIALS_IRIDESCENCE (1 << 15)
#define CGLTF_EXTENSION_FLAG_MATERIALS_ANISOTROPY (1 << 16)
#define CGLTF_EXTENSION_FLAG_MATERIALS_DISPERSION (1 << 17)
#define CGLTF_EXTENSION_FLAG_TEXTURE_WEBP          (1 << 18)
#define CGLTF_EXTENSION_FLAG_MATERIALS_DIFFUSE_TRANSMISSION (1 << 19)

typedef struct {
	char* buffer;
	cgltf_size buffer_size;
	cgltf_size remaining;
	char* cursor;
	cgltf_size tmp;
	cgltf_size chars_written;
	const cgltf_data* data;
	int depth;
	const char* indent;
	int needs_comma;
	uint32_t extension_flags;
	uint32_t required_extension_flags;
} cgltf_write_context;

#define CGLTF_MIN(a, b) (a < b ? a : b)

#ifdef FLT_DECIMAL_DIG
	// FLT_DECIMAL_DIG is C11
	#define CGLTF_DECIMAL_DIG (FLT_DECIMAL_DIG)
#else
	#define CGLTF_DECIMAL_DIG 9
#endif

#define CGLTF_SPRINTF(...) { \
		assert(context->cursor || (!context->cursor && context->remaining == 0)); \
		context->tmp = snprintf ( context->cursor, context->remaining, __VA_ARGS__ ); \
		context->chars_written += context->tmp; \
		if (context->cursor) { \
			context->cursor += context->tmp; \
			context->remaining -= context->tmp; \
		} }

#define CGLTF_SNPRINTF(length, ...) { \
		assert(context->cursor || (!context->cursor && context->remaining == 0)); \
		context->tmp = snprintf ( context->cursor, CGLTF_MIN(length + 1, context->remaining), __VA_ARGS__ ); \
		context->chars_written += length; \
		if (context->cursor) { \
			context->cursor += length; \
			context->remaining -= length; \
		} }

#define CGLTF_WRITE_IDXPROP(label, val, start) if (val) { \
		cgltf_write_indent(context); \
		CGLTF_SPRINTF("\"%s\": %d", label, (int) (val - start)); \
		context->needs_comma = 1; }

#define CGLTF_WRITE_IDXARRPROP(label, dim, vals, start) if (vals) { \
		cgltf_write_indent(context); \
		CGLTF_SPRINTF("\"%s\": [", label); \
		for (int i = 0; i < (int)(dim); ++i) { \
			int idx = (int) (vals[i] - start); \
			if (i != 0) CGLTF_SPRINTF(","); \
			CGLTF_SPRINTF(" %d", idx); \
		} \
		CGLTF_SPRINTF(" ]"); \
		context->needs_comma = 1; }

#define CGLTF_WRITE_TEXTURE_INFO(label, info) if (info.texture) { \
		cgltf_write_line(context, "\"" label "\": {"); \
		CGLTF_WRITE_IDXPROP("index", info.texture, context->data->textures); \
		cgltf_write_intprop(context, "texCoord", info.texcoord, 0); \
		if (info.has_transform) { \
			context->extension_flags |= CGLTF_EXTENSION_FLAG_TEXTURE_TRANSFORM; \
			cgltf_write_texture_transform(context, &info.transform); \
		} \
		cgltf_write_line(context, "}"); }

#define CGLTF_WRITE_NORMAL_TEXTURE_INFO(label, info) if (info.texture) { \
		cgltf_write_line(context, "\"" label "\": {"); \
		CGLTF_WRITE_IDXPROP("index", info.texture, context->data->textures); \
		cgltf_write_intprop(context, "texCoord", info.texcoord, 0); \
		cgltf_write_floatprop(context, "scale", info.scale, 1.0f); \
		if (info.has_transform) { \
			context->extension_flags |= CGLTF_EXTENSION_FLAG_TEXTURE_TRANSFORM; \
			cgltf_write_texture_transform(context, &info.transform); \
		} \
		cgltf_write_line(context, "}"); }

#define CGLTF_WRITE_OCCLUSION_TEXTURE_INFO(label, info) if (info.texture) { \
		cgltf_write_line(context, "\"" label "\": {"); \
		CGLTF_WRITE_IDXPROP("index", info.texture, context->data->textures); \
		cgltf_write_intprop(context, "texCoord", info.texcoord, 0); \
		cgltf_write_floatprop(context, "strength", info.scale, 1.0f); \
		if (info.has_transform) { \
			context->extension_flags |= CGLTF_EXTENSION_FLAG_TEXTURE_TRANSFORM; \
			cgltf_write_texture_transform(context, &info.transform); \
		} \
		cgltf_write_line(context, "}"); }

#ifndef CGLTF_CONSTS
#define GlbHeaderSize 12
#define GlbChunkHeaderSize 8
static const uint32_t GlbVersion = 2;
static const uint32_t GlbMagic = 0x46546C67;
static const uint32_t GlbMagicJsonChunk = 0x4E4F534A;
static const uint32_t GlbMagicBinChunk = 0x004E4942;
#define CGLTF_CONSTS
#endif

static void cgltf_write_indent(cgltf_write_context* context)
{
	if (context->needs_comma)
	{
		CGLTF_SPRINTF(",\n");
		context->needs_comma = 0;
	}
	else
	{
		CGLTF_SPRINTF("\n");
	}
	for (int i = 0; i < context->depth; ++i)
	{
		CGLTF_SPRINTF("%s", context->indent);
	}
}

static void cgltf_write_line(cgltf_write_context* context, const char* line)
{
	if (line[0] == ']' || line[0] == '}')
	{
		--context->depth;
		context->needs_comma = 0;
	}
	cgltf_write_indent(context);
	CGLTF_SPRINTF("%s", line);
	cgltf_size last = (cgltf_size)(strlen(line) - 1);
	if (line[0] == ']' || line[0] == '}')
	{
		context->needs_comma = 1;
	}
	if (line[last] == '[' || line[last] == '{')
	{
		++context->depth;
		context->needs_comma = 0;
	}
}

static void cgltf_write_strprop(cgltf_write_context* context, const char* label, const char* val)
{
	if (val)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": \"%s\"", label, val);
		context->needs_comma = 1;
	}
}

static void cgltf_write_extras(cgltf_write_context* context, const cgltf_extras* extras)
{
	if (extras->data)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"extras\": %s", extras->data);
		context->needs_comma = 1;
	}
	else
	{
		cgltf_size length = extras->end_offset - extras->start_offset;
		if (length > 0 && context->data->json)
		{
			char* json_string = ((char*) context->data->json) + extras->start_offset;
			cgltf_write_indent(context);
			CGLTF_SPRINTF("%s", "\"extras\": ");
			CGLTF_SNPRINTF(length, "%.*s", (int)(extras->end_offset - extras->start_offset), json_string);
			context->needs_comma = 1;
		}
	}
}

static void cgltf_write_stritem(cgltf_write_context* context, const char* item)
{
	cgltf_write_indent(context);
	CGLTF_SPRINTF("\"%s\"", item);
	context->needs_comma = 1;
}

static void cgltf_write_intprop(cgltf_write_context* context, const char* label, int val, int def)
{
	if (val != def)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": %d", label, val);
		context->needs_comma = 1;
	}
}

static void cgltf_write_sizeprop(cgltf_write_context* context, const char* label, cgltf_size val, cgltf_size def)
{
	if (val != def)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": %zu", label, val);
		context->needs_comma = 1;
	}
}

static void cgltf_write_floatprop(cgltf_write_context* context, const char* label, float val, float def)
{
	if (val != def)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": ", label);
		CGLTF_SPRINTF("%.*g", CGLTF_DECIMAL_DIG, val);
		context->needs_comma = 1;

		if (context->cursor)
		{
			char *decimal_comma = strchr(context->cursor - context->tmp, ',');
			if (decimal_comma)
			{
				*decimal_comma = '.';
			}
		}
	}
}

static void cgltf_write_boolprop_optional(cgltf_write_context* context, const char* label, bool val, bool def)
{
	if (val != def)
	{
		cgltf_write_indent(context);
		CGLTF_SPRINTF("\"%s\": %s", label, val ? "true" : "false");
		context->needs_comma = 1;
	}
}

static void cgltf_write_floatarrayprop(cgltf_write_context* context, const char* label, const cgltf_float* vals, cgltf_size dim)
{
	cgltf_write_indent(context);
	CGLTF_SPRINTF("\"%s\": [", label);
	for (cgltf_size i = 0; i < dim; ++i)
	{
		if (i != 0)
		{
			CGLTF_SPRINTF(", %.*g", CGLTF_DECIMAL_DIG, vals[i]);
		}
		else
		{
			CGLTF_SPRINTF("%.*g", CGLTF_DECIMAL_DIG, vals[i]);
		}
	}
	CGLTF_SPRINTF("]");
	context->needs_comma = 1;
}

static bool cgltf_check_floatarray(const float* vals, int dim, float val) {
	while (dim--)
	{
		if (vals[dim] != val)
		{
			return true;
		}
	}
	return false;
}

static int cgltf_int_from_component_type(cgltf_component_type ctype)
{
	switch (ctype)
	{
		case cgltf_component_type_r_8: return 5120;
		case cgltf_component_type_r_8u: return 5121;
		case cgltf_component_type_r_16: return 5122;
		case cgltf_component_type_r_16u: return 5123;
		case cgltf_component_type_r_32u: return 5125;
		case cgltf_component_type_r_32f: return 5126;
		default: return 0;
	}
}

static int cgltf_int_from_primitive_type(cgltf_primitive_type ctype)
{
	switch (ctype)
	{
		case cgltf_primitive_type_points: return 0;
		case cgltf_primitive_type_lines: return 1;
		case cgltf_primitive_type_line_loop: return 2;
		case cgltf_primitive_type_line_strip: return 3;
		case cgltf_primitive_type_triangles: return 4;
		case cgltf_primitive_type_triangle_strip: return 5;
		case cgltf_primitive_type_triangle_fan: return 6;
		default: return -1;
	}
}

static const char* cgltf_str_from_alpha_mode(cgltf_alpha_mode alpha_mode)
{
	switch (alpha_mode)
	{
		case cgltf_alpha_mode_mask: return "MASK";
		case cgltf_alpha_mode_blend: return "BLEND";
		default: return NULL;
	}
}

static const char* cgltf_str_from_type(cgltf_type type)
{
	switch (type)
	{
		case cgltf_type_scalar: return "SCALAR";
		case cgltf_type_vec2: return "VEC2";
		case cgltf_type_vec3: return "VEC3";
		case cgltf_type_vec4: return "VEC4";
		case cgltf_type_mat2: return "MAT2";
		case cgltf_type_mat3: return "MAT3";
		case cgltf_type_mat4: return "MAT4";
		default: return NULL;
	}
}

static cgltf_size cgltf_dim_from_type(cgltf_type type)
{
	switch (type)
	{
		case cgltf_type_scalar: return 1;
		case cgltf_type_vec2: return 2;
		case cgltf_type_vec3: return 3;
		case cgltf_type_vec4: return 4;
		case cgltf_type_mat2: return 4;
		case cgltf_type_mat3: return 9;
		case cgltf_type_mat4: return 16;
		default: return 0;
	}
}

static const char* cgltf_str_from_camera_type(cgltf_camera_type camera_type)
{
	switch (camera_type)
	{
		case cgltf_camera_type_perspective: return "perspective";
		case cgltf_camera_type_orthographic: return "orthographic";
		default: return NULL;
	}
}

static const char* cgltf_str_from_light_type(cgltf_light_type light_type)
{
	switch (light_type)
	{
		case cgltf_light_type_directional: return "directional";
		case cgltf_light_type_point: return "point";
		case cgltf_light_type_spot: return "spot";
		default: return NULL;
	}
}

static void cgltf_write_texture_transform(cgltf_write_context* context, const cgltf_texture_transform* transform)
{
	cgltf_write_line(context, "\"extensions\": {");
	cgltf_write_line(context, "\"KHR_texture_transform\": {");
	if (cgltf_check_floatarray(transform->offset, 2, 0.0f))
	{
		cgltf_write_floatarrayprop(context, "offset", transform->offset, 2);
	}
	cgltf_write_floatprop(context, "rotation", transform->rotation, 0.0f);
	if (cgltf_check_floatarray(transform->scale, 2, 1.0f))
	{
		cgltf_write_floatarrayprop(context, "scale", transform->scale, 2);
	}
	if (transform->has_texcoord)
	{
		cgltf_write_intprop(context, "texCoord", transform->texcoord, -1);
	}
	cgltf_write_line(context, "}");
	cgltf_write_line(context, "}");
}

static void cgltf_write_asset(cgltf_write_context* context, const cgltf_asset* asset)
{
	cgltf_write_line(context, "\"asset\": {");
	cgltf_write_strprop(context, "copyright", asset->copyright);
	cgltf_write_strprop(context, "generator", asset->generator);
	cgltf_write_strprop(context, "version", asset->version);
	cgltf_write_strprop(context, "min_version", asset->min_version);
	cgltf_write_extras(context, &asset->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_primitive(cgltf_write_context* context, const cgltf_primitive* prim)
{
	cgltf_write_intprop(context, "mode", cgltf_int_from_primitive_type(prim->type), 4);
	CGLTF_WRITE_IDXPROP("indices", prim->indices, context->data->accessors);
	CGLTF_WRITE_IDXPROP("material", prim->material, context->data->materials);
	cgltf_write_line(context, "\"attributes\": {");
	for (cgltf_size i = 0; i < prim->attributes_count; ++i)
	{
		const cgltf_attribute* attr = prim->attributes + i;
		CGLTF_WRITE_IDXPROP(attr->name, attr->data, context->data->accessors);
	}
	cgltf_write_line(context, "}");

	if (prim->targets_count)
	{
		cgltf_write_line(context, "\"targets\": [");
		for (cgltf_size i = 0; i < prim->targets_count; ++i)
		{
			cgltf_write_line(context, "{");
			for (cgltf_size j = 0; j < prim->targets[i].attributes_count; ++j)
			{
				const cgltf_attribute* attr = prim->targets[i].attributes + j;
				CGLTF_WRITE_IDXPROP(attr->name, attr->data, context->data->accessors);
			}
			cgltf_write_line(context, "}");
		}
		cgltf_write_line(context, "]");
	}
	cgltf_write_extras(context, &prim->extras);

	if (prim->has_draco_mesh_compression || prim->mappings_count > 0)
	{
		cgltf_write_line(context, "\"extensions\": {");

		if (prim->has_draco_mesh_compression)
		{
			context->extension_flags |= CGLTF_EXTENSION_FLAG_DRACO_MESH_COMPRESSION;
			if (prim->attributes_count == 0 || prim->indices == 0)
			{
				context->required_extension_flags |= CGLTF_EXTENSION_FLAG_DRACO_MESH_COMPRESSION;
			}

			cgltf_write_line(context, "\"KHR_draco_mesh_compression\": {");
			CGLTF_WRITE_IDXPROP("bufferView", prim->draco_mesh_compression.buffer_view, context->data->buffer_views);
			cgltf_write_line(context, "\"attributes\": {");
			for (cgltf_size i = 0; i < prim->draco_mesh_compression.attributes_count; ++i)
			{
				const cgltf_attribute* attr = prim->draco_mesh_compression.attributes + i;
				CGLTF_WRITE_IDXPROP(attr->name, attr->data, context->data->accessors);
			}
			cgltf_write_line(context, "}");
			cgltf_write_line(context, "}");
		}

		if (prim->mappings_count > 0)
		{
			context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_VARIANTS;
			cgltf_write_line(context, "\"KHR_materials_variants\": {");
			cgltf_write_line(context, "\"mappings\": [");
			for (cgltf_size i = 0; i < prim->mappings_count; ++i)
			{
				const cgltf_material_mapping* map = prim->mappings + i;
				cgltf_write_line(context, "{");
				CGLTF_WRITE_IDXPROP("material", map->material, context->data->materials);

				cgltf_write_indent(context);
				CGLTF_SPRINTF("\"variants\": [%d]", (int)map->variant);
				context->needs_comma = 1;

				cgltf_write_extras(context, &map->extras);
				cgltf_write_line(context, "}");
			}
			cgltf_write_line(context, "]");
			cgltf_write_line(context, "}");
		}

		cgltf_write_line(context, "}");
	}
}

static void cgltf_write_mesh(cgltf_write_context* context, const cgltf_mesh* mesh)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", mesh->name);

	cgltf_write_line(context, "\"primitives\": [");
	for (cgltf_size i = 0; i < mesh->primitives_count; ++i)
	{
		cgltf_write_line(context, "{");
		cgltf_write_primitive(context, mesh->primitives + i);
		cgltf_write_line(context, "}");
	}
	cgltf_write_line(context, "]");

	if (mesh->weights_count > 0)
	{
		cgltf_write_floatarrayprop(context, "weights", mesh->weights, mesh->weights_count);
	}

	cgltf_write_extras(context, &mesh->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_buffer_view(cgltf_write_context* context, const cgltf_buffer_view* view)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", view->name);
	CGLTF_WRITE_IDXPROP("buffer", view->buffer, context->data->buffers);
	cgltf_write_sizeprop(context, "byteLength", view->size, (cgltf_size)-1);
	cgltf_write_sizeprop(context, "byteOffset", view->offset, 0);
	cgltf_write_sizeprop(context, "byteStride", view->stride, 0);
	// NOTE: We skip writing "target" because the spec says its usage can be inferred.
	cgltf_write_extras(context, &view->extras);
	cgltf_write_line(context, "}");
}


static void cgltf_write_buffer(cgltf_write_context* context, const cgltf_buffer* buffer)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", buffer->name);
	cgltf_write_strprop(context, "uri", buffer->uri);
	cgltf_write_sizeprop(context, "byteLength", buffer->size, (cgltf_size)-1);
	cgltf_write_extras(context, &buffer->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_material(cgltf_write_context* context, const cgltf_material* material)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", material->name);
	if (material->alpha_mode == cgltf_alpha_mode_mask)
	{
		cgltf_write_floatprop(context, "alphaCutoff", material->alpha_cutoff, 0.5f);
	}
	cgltf_write_boolprop_optional(context, "doubleSided", (bool)material->double_sided, false);
	// cgltf_write_boolprop_optional(context, "unlit", material->unlit, false);

	if (material->unlit)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_UNLIT;
	}

	if (material->has_pbr_specular_glossiness)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_SPECULAR_GLOSSINESS;
	}

	if (material->has_clearcoat)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_CLEARCOAT;
	}

	if (material->has_transmission)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_TRANSMISSION;
	}

	if (material->has_volume)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_VOLUME;
	}

	if (material->has_ior)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_IOR;
	}

	if (material->has_specular)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_SPECULAR;
	}

	if (material->has_sheen)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_SHEEN;
	}

	if (material->has_emissive_strength)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_EMISSIVE_STRENGTH;
	}

	if (material->has_iridescence)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_IRIDESCENCE;
	}

	if (material->has_diffuse_transmission)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_DIFFUSE_TRANSMISSION;
	}

	if (material->has_anisotropy)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_ANISOTROPY;
	}

	if (material->has_dispersion)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_DISPERSION;
	}

	if (material->has_pbr_metallic_roughness)
	{
		const cgltf_pbr_metallic_roughness* params = &material->pbr_metallic_roughness;
		cgltf_write_line(context, "\"pbrMetallicRoughness\": {");
		CGLTF_WRITE_TEXTURE_INFO("baseColorTexture", params->base_color_texture);
		CGLTF_WRITE_TEXTURE_INFO("metallicRoughnessTexture", params->metallic_roughness_texture);
		cgltf_write_floatprop(context, "metallicFactor", params->metallic_factor, 1.0f);
		cgltf_write_floatprop(context, "roughnessFactor", params->roughness_factor, 1.0f);
		if (cgltf_check_floatarray(params->base_color_factor, 4, 1.0f))
		{
			cgltf_write_floatarrayprop(context, "baseColorFactor", params->base_color_factor, 4);
		}
		cgltf_write_line(context, "}");
	}

	if (material->unlit || material->has_pbr_specular_glossiness || material->has_clearcoat || material->has_ior || material->has_specular || material->has_transmission || material->has_sheen || material->has_volume || material->has_emissive_strength || material->has_iridescence || material->has_anisotropy || material->has_dispersion || material->has_diffuse_transmission)
	{
		cgltf_write_line(context, "\"extensions\": {");
		if (material->has_clearcoat)
		{
			const cgltf_clearcoat* params = &material->clearcoat;
			cgltf_write_line(context, "\"KHR_materials_clearcoat\": {");
			CGLTF_WRITE_TEXTURE_INFO("clearcoatTexture", params->clearcoat_texture);
			CGLTF_WRITE_TEXTURE_INFO("clearcoatRoughnessTexture", params->clearcoat_roughness_texture);
			CGLTF_WRITE_NORMAL_TEXTURE_INFO("clearcoatNormalTexture", params->clearcoat_normal_texture);
			cgltf_write_floatprop(context, "clearcoatFactor", params->clearcoat_factor, 0.0f);
			cgltf_write_floatprop(context, "clearcoatRoughnessFactor", params->clearcoat_roughness_factor, 0.0f);
			cgltf_write_line(context, "}");
		}
		if (material->has_ior)
		{
			const cgltf_ior* params = &material->ior;
			cgltf_write_line(context, "\"KHR_materials_ior\": {");
			cgltf_write_floatprop(context, "ior", params->ior, 1.5f);
			cgltf_write_line(context, "}");
		}
		if (material->has_specular)
		{
			const cgltf_specular* params = &material->specular;
			cgltf_write_line(context, "\"KHR_materials_specular\": {");
			CGLTF_WRITE_TEXTURE_INFO("specularTexture", params->specular_texture);
			CGLTF_WRITE_TEXTURE_INFO("specularColorTexture", params->specular_color_texture);
			cgltf_write_floatprop(context, "specularFactor", params->specular_factor, 1.0f);
			if (cgltf_check_floatarray(params->specular_color_factor, 3, 1.0f))
			{
				cgltf_write_floatarrayprop(context, "specularColorFactor", params->specular_color_factor, 3);
			}
			cgltf_write_line(context, "}");
		}
		if (material->has_transmission)
		{
			const cgltf_transmission* params = &material->transmission;
			cgltf_write_line(context, "\"KHR_materials_transmission\": {");
			CGLTF_WRITE_TEXTURE_INFO("transmissionTexture", params->transmission_texture);
			cgltf_write_floatprop(context, "transmissionFactor", params->transmission_factor, 0.0f);
			cgltf_write_line(context, "}");
		}
		if (material->has_volume)
		{
			const cgltf_volume* params = &material->volume;
			cgltf_write_line(context, "\"KHR_materials_volume\": {");
			CGLTF_WRITE_TEXTURE_INFO("thicknessTexture", params->thickness_texture);
			cgltf_write_floatprop(context, "thicknessFactor", params->thickness_factor, 0.0f);
			if (cgltf_check_floatarray(params->attenuation_color, 3, 1.0f))
			{
				cgltf_write_floatarrayprop(context, "attenuationColor", params->attenuation_color, 3);
			}
			if (params->attenuation_distance < FLT_MAX)
			{
				cgltf_write_floatprop(context, "attenuationDistance", params->attenuation_distance, FLT_MAX);
			}
			cgltf_write_line(context, "}");
		}
		if (material->has_sheen)
		{
			const cgltf_sheen* params = &material->sheen;
			cgltf_write_line(context, "\"KHR_materials_sheen\": {");
			CGLTF_WRITE_TEXTURE_INFO("sheenColorTexture", params->sheen_color_texture);
			CGLTF_WRITE_TEXTURE_INFO("sheenRoughnessTexture", params->sheen_roughness_texture);
			if (cgltf_check_floatarray(params->sheen_color_factor, 3, 0.0f))
			{
				cgltf_write_floatarrayprop(context, "sheenColorFactor", params->sheen_color_factor, 3);
			}
			cgltf_write_floatprop(context, "sheenRoughnessFactor", params->sheen_roughness_factor, 0.0f);
			cgltf_write_line(context, "}");
		}
		if (material->has_pbr_specular_glossiness)
		{
			const cgltf_pbr_specular_glossiness* params = &material->pbr_specular_glossiness;
			cgltf_write_line(context, "\"KHR_materials_pbrSpecularGlossiness\": {");
			CGLTF_WRITE_TEXTURE_INFO("diffuseTexture", params->diffuse_texture);
			CGLTF_WRITE_TEXTURE_INFO("specularGlossinessTexture", params->specular_glossiness_texture);
			if (cgltf_check_floatarray(params->diffuse_factor, 4, 1.0f))
			{
				cgltf_write_floatarrayprop(context, "diffuseFactor", params->diffuse_factor, 4);
			}
			if (cgltf_check_floatarray(params->specular_factor, 3, 1.0f))
			{
				cgltf_write_floatarrayprop(context, "specularFactor", params->specular_factor, 3);
			}
			cgltf_write_floatprop(context, "glossinessFactor", params->glossiness_factor, 1.0f);
			cgltf_write_line(context, "}");
		}
		if (material->unlit)
		{
			cgltf_write_line(context, "\"KHR_materials_unlit\": {}");
		}
		if (material->has_emissive_strength)
		{
			cgltf_write_line(context, "\"KHR_materials_emissive_strength\": {");
			const cgltf_emissive_strength* params = &material->emissive_strength;
			cgltf_write_floatprop(context, "emissiveStrength", params->emissive_strength, 1.f);
			cgltf_write_line(context, "}");
		}
		if (material->has_iridescence)
		{
			cgltf_write_line(context, "\"KHR_materials_iridescence\": {");
			const cgltf_iridescence* params = &material->iridescence;
			cgltf_write_floatprop(context, "iridescenceFactor", params->iridescence_factor, 0.f);
			CGLTF_WRITE_TEXTURE_INFO("iridescenceTexture", params->iridescence_texture);
			cgltf_write_floatprop(context, "iridescenceIor", params->iridescence_ior, 1.3f);
			cgltf_write_floatprop(context, "iridescenceThicknessMinimum", params->iridescence_thickness_min, 100.f);
			cgltf_write_floatprop(context, "iridescenceThicknessMaximum", params->iridescence_thickness_max, 400.f);
			CGLTF_WRITE_TEXTURE_INFO("iridescenceThicknessTexture", params->iridescence_thickness_texture);
			cgltf_write_line(context, "}");
		}
		if (material->has_diffuse_transmission)
		{
			const cgltf_diffuse_transmission* params = &material->diffuse_transmission;
			cgltf_write_line(context, "\"KHR_materials_diffuse_transmission\": {");
			CGLTF_WRITE_TEXTURE_INFO("diffuseTransmissionTexture", params->diffuse_transmission_texture);
			cgltf_write_floatprop(context, "diffuseTransmissionFactor", params->diffuse_transmission_factor, 0.f);
			if (cgltf_check_floatarray(params->diffuse_transmission_color_factor, 3, 1.f))
			{
				cgltf_write_floatarrayprop(context, "diffuseTransmissionColorFactor", params->diffuse_transmission_color_factor, 3);
			}
			CGLTF_WRITE_TEXTURE_INFO("diffuseTransmissionColorTexture", params->diffuse_transmission_color_texture);
			cgltf_write_line(context, "}");
		}
		if (material->has_anisotropy)
		{
			cgltf_write_line(context, "\"KHR_materials_anisotropy\": {");
			const cgltf_anisotropy* params = &material->anisotropy;
			cgltf_write_floatprop(context, "anisotropyStrength", params->anisotropy_strength, 0.f);
			cgltf_write_floatprop(context, "anisotropyRotation", params->anisotropy_rotation, 0.f);
			CGLTF_WRITE_TEXTURE_INFO("anisotropyTexture", params->anisotropy_texture);
			cgltf_write_line(context, "}");
		}
		if (material->has_dispersion)
		{
			cgltf_write_line(context, "\"KHR_materials_dispersion\": {");
			const cgltf_dispersion* params = &material->dispersion;
			cgltf_write_floatprop(context, "dispersion", params->dispersion, 0.f);
			cgltf_write_line(context, "}");
		}
		cgltf_write_line(context, "}");
	}

	CGLTF_WRITE_NORMAL_TEXTURE_INFO("normalTexture", material->normal_texture);
	CGLTF_WRITE_OCCLUSION_TEXTURE_INFO("occlusionTexture", material->occlusion_texture);
	CGLTF_WRITE_TEXTURE_INFO("emissiveTexture", material->emissive_texture);
	if (cgltf_check_floatarray(material->emissive_factor, 3, 0.0f))
	{
		cgltf_write_floatarrayprop(context, "emissiveFactor", material->emissive_factor, 3);
	}
	cgltf_write_strprop(context, "alphaMode", cgltf_str_from_alpha_mode(material->alpha_mode));
	cgltf_write_extras(context, &material->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_image(cgltf_write_context* context, const cgltf_image* image)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", image->name);
	cgltf_write_strprop(context, "uri", image->uri);
	CGLTF_WRITE_IDXPROP("bufferView", image->buffer_view, context->data->buffer_views);
	cgltf_write_strprop(context, "mimeType", image->mime_type);
	cgltf_write_extras(context, &image->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_texture(cgltf_write_context* context, const cgltf_texture* texture)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", texture->name);
	CGLTF_WRITE_IDXPROP("source", texture->image, context->data->images);
	CGLTF_WRITE_IDXPROP("sampler", texture->sampler, context->data->samplers);

	if (texture->has_basisu || texture->has_webp)
	{
		cgltf_write_line(context, "\"extensions\": {");
		if (texture->has_basisu)
		{
			context->extension_flags |= CGLTF_EXTENSION_FLAG_TEXTURE_BASISU;
			cgltf_write_line(context, "\"KHR_texture_basisu\": {");
			CGLTF_WRITE_IDXPROP("source", texture->basisu_image, context->data->images);
			cgltf_write_line(context, "}");
		}
		if (texture->has_webp)
		{
			context->extension_flags |= CGLTF_EXTENSION_FLAG_TEXTURE_WEBP;
			cgltf_write_line(context, "\"EXT_texture_webp\": {");
			CGLTF_WRITE_IDXPROP("source", texture->webp_image, context->data->images);
			cgltf_write_line(context, "}");
		}
		cgltf_write_line(context, "}");
	}
	cgltf_write_extras(context, &texture->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_skin(cgltf_write_context* context, const cgltf_skin* skin)
{
	cgltf_write_line(context, "{");
	CGLTF_WRITE_IDXPROP("skeleton", skin->skeleton, context->data->nodes);
	CGLTF_WRITE_IDXPROP("inverseBindMatrices", skin->inverse_bind_matrices, context->data->accessors);
	CGLTF_WRITE_IDXARRPROP("joints", skin->joints_count, skin->joints, context->data->nodes);
	cgltf_write_strprop(context, "name", skin->name);
	cgltf_write_extras(context, &skin->extras);
	cgltf_write_line(context, "}");
}

static const char* cgltf_write_str_path_type(cgltf_animation_path_type path_type)
{
	switch (path_type)
	{
	case cgltf_animation_path_type_translation:
		return "translation";
	case cgltf_animation_path_type_rotation:
		return "rotation";
	case cgltf_animation_path_type_scale:
		return "scale";
	case cgltf_animation_path_type_weights:
		return "weights";
	default:
		break;
	}
	return "invalid";
}

static const char* cgltf_write_str_interpolation_type(cgltf_interpolation_type interpolation_type)
{
	switch (interpolation_type)
	{
	case cgltf_interpolation_type_linear:
		return "LINEAR";
	case cgltf_interpolation_type_step:
		return "STEP";
	case cgltf_interpolation_type_cubic_spline:
		return "CUBICSPLINE";
	default:
		break;
	}
	return "invalid";
}

static void cgltf_write_path_type(cgltf_write_context* context, const char *label, cgltf_animation_path_type path_type)
{
	cgltf_write_strprop(context, label, cgltf_write_str_path_type(path_type));
}

static void cgltf_write_interpolation_type(cgltf_write_context* context, const char *label, cgltf_interpolation_type interpolation_type)
{
	cgltf_write_strprop(context, label, cgltf_write_str_interpolation_type(interpolation_type));
}

static void cgltf_write_animation_sampler(cgltf_write_context* context, const cgltf_animation_sampler* animation_sampler)
{
	cgltf_write_line(context, "{");
	cgltf_write_interpolation_type(context, "interpolation", animation_sampler->interpolation);
	CGLTF_WRITE_IDXPROP("input", animation_sampler->input, context->data->accessors);
	CGLTF_WRITE_IDXPROP("output", animation_sampler->output, context->data->accessors);
	cgltf_write_extras(context, &animation_sampler->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_animation_channel(cgltf_write_context* context, const cgltf_animation* animation, const cgltf_animation_channel* animation_channel)
{
	cgltf_write_line(context, "{");
	CGLTF_WRITE_IDXPROP("sampler", animation_channel->sampler, animation->samplers);
	cgltf_write_line(context, "\"target\": {");
	CGLTF_WRITE_IDXPROP("node", animation_channel->target_node, context->data->nodes);
	cgltf_write_path_type(context, "path", animation_channel->target_path);
	cgltf_write_line(context, "}");
	cgltf_write_extras(context, &animation_channel->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_animation(cgltf_write_context* context, const cgltf_animation* animation)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", animation->name);

	if (animation->samplers_count > 0)
	{
		cgltf_write_line(context, "\"samplers\": [");
		for (cgltf_size i = 0; i < animation->samplers_count; ++i)
		{
			cgltf_write_animation_sampler(context, animation->samplers + i);
		}
		cgltf_write_line(context, "]");
	}
	if (animation->channels_count > 0)
	{
		cgltf_write_line(context, "\"channels\": [");
		for (cgltf_size i = 0; i < animation->channels_count; ++i)
		{
			cgltf_write_animation_channel(context, animation, animation->channels + i);
		}
		cgltf_write_line(context, "]");
	}
	cgltf_write_extras(context, &animation->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_sampler(cgltf_write_context* context, const cgltf_sampler* sampler)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", sampler->name);
	cgltf_write_intprop(context, "magFilter", sampler->mag_filter, 0);
	cgltf_write_intprop(context, "minFilter", sampler->min_filter, 0);
	cgltf_write_intprop(context, "wrapS", sampler->wrap_s, 10497);
	cgltf_write_intprop(context, "wrapT", sampler->wrap_t, 10497);
	cgltf_write_extras(context, &sampler->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_node(cgltf_write_context* context, const cgltf_node* node)
{
	cgltf_write_line(context, "{");
	CGLTF_WRITE_IDXARRPROP("children", node->children_count, node->children, context->data->nodes);
	CGLTF_WRITE_IDXPROP("mesh", node->mesh, context->data->meshes);
	cgltf_write_strprop(context, "name", node->name);
	if (node->has_matrix)
	{
		cgltf_write_floatarrayprop(context, "matrix", node->matrix, 16);
	}
	if (node->has_translation)
	{
		cgltf_write_floatarrayprop(context, "translation", node->translation, 3);
	}
	if (node->has_rotation)
	{
		cgltf_write_floatarrayprop(context, "rotation", node->rotation, 4);
	}
	if (node->has_scale)
	{
		cgltf_write_floatarrayprop(context, "scale", node->scale, 3);
	}
	if (node->skin)
	{
		CGLTF_WRITE_IDXPROP("skin", node->skin, context->data->skins);
	}

	bool has_extension = node->light || (node->has_mesh_gpu_instancing && node->mesh_gpu_instancing.attributes_count > 0);
	if(has_extension)
		cgltf_write_line(context, "\"extensions\": {");

	if (node->light)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_LIGHTS_PUNCTUAL;
		cgltf_write_line(context, "\"KHR_lights_punctual\": {");
		CGLTF_WRITE_IDXPROP("light", node->light, context->data->lights);
		cgltf_write_line(context, "}");
	}

	if (node->has_mesh_gpu_instancing && node->mesh_gpu_instancing.attributes_count > 0)
	{
		context->extension_flags |= CGLTF_EXTENSION_FLAG_MESH_GPU_INSTANCING;
		context->required_extension_flags |= CGLTF_EXTENSION_FLAG_MESH_GPU_INSTANCING;

		cgltf_write_line(context, "\"EXT_mesh_gpu_instancing\": {");
		{
			cgltf_write_line(context, "\"attributes\": {");
			{
				for (cgltf_size i = 0; i < node->mesh_gpu_instancing.attributes_count; ++i)
				{
					const cgltf_attribute* attr = node->mesh_gpu_instancing.attributes + i;
					CGLTF_WRITE_IDXPROP(attr->name, attr->data, context->data->accessors);
				}
			}
			cgltf_write_line(context, "}");
		}
		cgltf_write_line(context, "}");
	}

	if (has_extension)
		cgltf_write_line(context, "}");

	if (node->weights_count > 0)
	{
		cgltf_write_floatarrayprop(context, "weights", node->weights, node->weights_count);
	}

	if (node->camera)
	{
		CGLTF_WRITE_IDXPROP("camera", node->camera, context->data->cameras);
	}

	cgltf_write_extras(context, &node->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_scene(cgltf_write_context* context, const cgltf_scene* scene)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", scene->name);
	CGLTF_WRITE_IDXARRPROP("nodes", scene->nodes_count, scene->nodes, context->data->nodes);
	cgltf_write_extras(context, &scene->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_accessor(cgltf_write_context* context, const cgltf_accessor* accessor)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", accessor->name);
	CGLTF_WRITE_IDXPROP("bufferView", accessor->buffer_view, context->data->buffer_views);
	cgltf_write_intprop(context, "componentType", cgltf_int_from_component_type(accessor->component_type), 0);
	cgltf_write_strprop(context, "type", cgltf_str_from_type(accessor->type));
	cgltf_size dim = cgltf_dim_from_type(accessor->type);
	cgltf_write_boolprop_optional(context, "normalized", (bool)accessor->normalized, false);
	cgltf_write_sizeprop(context, "byteOffset", (int)accessor->offset, 0);
	cgltf_write_intprop(context, "count", (int)accessor->count, -1);
	if (accessor->has_min)
	{
		cgltf_write_floatarrayprop(context, "min", accessor->min, dim);
	}
	if (accessor->has_max)
	{
		cgltf_write_floatarrayprop(context, "max", accessor->max, dim);
	}
	if (accessor->is_sparse)
	{
		cgltf_write_line(context, "\"sparse\": {");
		cgltf_write_intprop(context, "count", (int)accessor->sparse.count, 0);
		cgltf_write_line(context, "\"indices\": {");
		cgltf_write_sizeprop(context, "byteOffset", (int)accessor->sparse.indices_byte_offset, 0);
		CGLTF_WRITE_IDXPROP("bufferView", accessor->sparse.indices_buffer_view, context->data->buffer_views);
		cgltf_write_intprop(context, "componentType", cgltf_int_from_component_type(accessor->sparse.indices_component_type), 0);
		cgltf_write_line(context, "}");
		cgltf_write_line(context, "\"values\": {");
		cgltf_write_sizeprop(context, "byteOffset", (int)accessor->sparse.values_byte_offset, 0);
		CGLTF_WRITE_IDXPROP("bufferView", accessor->sparse.values_buffer_view, context->data->buffer_views);
		cgltf_write_line(context, "}");
		cgltf_write_line(context, "}");
	}
	cgltf_write_extras(context, &accessor->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_camera(cgltf_write_context* context, const cgltf_camera* camera)
{
	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "type", cgltf_str_from_camera_type(camera->type));
	if (camera->name)
	{
		cgltf_write_strprop(context, "name", camera->name);
	}

	if (camera->type == cgltf_camera_type_orthographic)
	{
		cgltf_write_line(context, "\"orthographic\": {");
		cgltf_write_floatprop(context, "xmag", camera->data.orthographic.xmag, -1.0f);
		cgltf_write_floatprop(context, "ymag", camera->data.orthographic.ymag, -1.0f);
		cgltf_write_floatprop(context, "zfar", camera->data.orthographic.zfar, -1.0f);
		cgltf_write_floatprop(context, "znear", camera->data.orthographic.znear, -1.0f);
		cgltf_write_extras(context, &camera->data.orthographic.extras);
		cgltf_write_line(context, "}");
	}
	else if (camera->type == cgltf_camera_type_perspective)
	{
		cgltf_write_line(context, "\"perspective\": {");

		if (camera->data.perspective.has_aspect_ratio) {
			cgltf_write_floatprop(context, "aspectRatio", camera->data.perspective.aspect_ratio, -1.0f);
		}

		cgltf_write_floatprop(context, "yfov", camera->data.perspective.yfov, -1.0f);

		if (camera->data.perspective.has_zfar) {
			cgltf_write_floatprop(context, "zfar", camera->data.perspective.zfar, -1.0f);
		}

		cgltf_write_floatprop(context, "znear", camera->data.perspective.znear, -1.0f);
		cgltf_write_extras(context, &camera->data.perspective.extras);
		cgltf_write_line(context, "}");
	}
	cgltf_write_extras(context, &camera->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_light(cgltf_write_context* context, const cgltf_light* light)
{
	context->extension_flags |= CGLTF_EXTENSION_FLAG_LIGHTS_PUNCTUAL;

	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "type", cgltf_str_from_light_type(light->type));
	if (light->name)
	{
		cgltf_write_strprop(context, "name", light->name);
	}
	if (cgltf_check_floatarray(light->color, 3, 1.0f))
	{
		cgltf_write_floatarrayprop(context, "color", light->color, 3);
	}
	cgltf_write_floatprop(context, "intensity", light->intensity, 1.0f);
	cgltf_write_floatprop(context, "range", light->range, 0.0f);

	if (light->type == cgltf_light_type_spot)
	{
		cgltf_write_line(context, "\"spot\": {");
		cgltf_write_floatprop(context, "innerConeAngle", light->spot_inner_cone_angle, 0.0f);
		cgltf_write_floatprop(context, "outerConeAngle", light->spot_outer_cone_angle, 3.14159265358979323846f/4.0f);
		cgltf_write_line(context, "}");
	}
	cgltf_write_extras( context, &light->extras );
	cgltf_write_line(context, "}");
}

static void cgltf_write_variant(cgltf_write_context* context, const cgltf_material_variant* variant)
{
	context->extension_flags |= CGLTF_EXTENSION_FLAG_MATERIALS_VARIANTS;

	cgltf_write_line(context, "{");
	cgltf_write_strprop(context, "name", variant->name);
	cgltf_write_extras(context, &variant->extras);
	cgltf_write_line(context, "}");
}

static void cgltf_write_glb(FILE* file, const void* json_buf, const cgltf_size json_size, const void* bin_buf, const cgltf_size bin_size)
{
	char header[GlbHeaderSize];
	char chunk_header[GlbChunkHeaderSize];
	char json_pad[3] = { 0x20, 0x20, 0x20 };
	char bin_pad[3] = { 0, 0, 0 };

	cgltf_size json_padsize = (json_size % 4 != 0) ? 4 - json_size % 4 : 0;
	cgltf_size bin_padsize = (bin_size % 4 != 0) ? 4 - bin_size % 4 : 0;
	cgltf_size total_size = GlbHeaderSize + GlbChunkHeaderSize + json_size + json_padsize;
	if (bin_buf != NULL && bin_size > 0) {
		total_size += GlbChunkHeaderSize + bin_size + bin_padsize;
	}

	// Write a GLB header
	memcpy(header, &GlbMagic, 4);
	memcpy(header + 4, &GlbVersion, 4);
	memcpy(header + 8, &total_size, 4);
	fwrite(header, 1, GlbHeaderSize, file);

	// Write a JSON chunk (header & data)
	uint32_t json_chunk_size = (uint32_t)(json_size + json_padsize);
	memcpy(chunk_header, &json_chunk_size, 4);
	memcpy(chunk_header + 4, &GlbMagicJsonChunk, 4);
	fwrite(chunk_header, 1, GlbChunkHeaderSize, file);

	fwrite(json_buf, 1, json_size, file);
	fwrite(json_pad, 1, json_padsize, file);

	if (bin_buf != NULL && bin_size > 0) {
		// Write a binary chunk (header & data)
		uint32_t bin_chunk_size = (uint32_t)(bin_size + bin_padsize);
		memcpy(chunk_header, &bin_chunk_size, 4);
		memcpy(chunk_header + 4, &GlbMagicBinChunk, 4);
		fwrite(chunk_header, 1, GlbChunkHeaderSize, file);

		fwrite(bin_buf, 1, bin_size, file);
		fwrite(bin_pad, 1, bin_padsize, file);
	}
}

cgltf_result cgltf_write_file(const cgltf_options* options, const char* path, const cgltf_data* data)
{
	cgltf_size expected = cgltf_write(options, NULL, 0, data);
	char* buffer = (char*) malloc(expected);
	cgltf_size actual = cgltf_write(options, buffer, expected, data);
	if (expected != actual) {
		fprintf(stderr, "Error: expected %zu bytes but wrote %zu bytes.\n", expected, actual);
	}
	FILE* file = fopen(path, "wb");
	if (!file)
	{
		return cgltf_result_file_not_found;
	}
	// Note that cgltf_write() includes a null terminator, which we omit from the file content.
	if (options->type == cgltf_file_type_glb) {
		cgltf_write_glb(file, buffer, actual - 1, data->bin, data->bin_size);
	} else {
		// Write a plain JSON file.
		fwrite(buffer, actual - 1, 1, file);
	}
	fclose(file);
	free(buffer);
	return cgltf_result_success;
}

static void cgltf_write_extensions(cgltf_write_context* context, uint32_t extension_flags)
{
	if (extension_flags & CGLTF_EXTENSION_FLAG_TEXTURE_TRANSFORM) {
		cgltf_write_stritem(context, "KHR_texture_transform");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_UNLIT) {
		cgltf_write_stritem(context, "KHR_materials_unlit");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_SPECULAR_GLOSSINESS) {
		cgltf_write_stritem(context, "KHR_materials_pbrSpecularGlossiness");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_LIGHTS_PUNCTUAL) {
		cgltf_write_stritem(context, "KHR_lights_punctual");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_DRACO_MESH_COMPRESSION) {
		cgltf_write_stritem(context, "KHR_draco_mesh_compression");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_CLEARCOAT) {
		cgltf_write_stritem(context, "KHR_materials_clearcoat");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_IOR) {
		cgltf_write_stritem(context, "KHR_materials_ior");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_SPECULAR) {
		cgltf_write_stritem(context, "KHR_materials_specular");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_TRANSMISSION) {
		cgltf_write_stritem(context, "KHR_materials_transmission");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_SHEEN) {
		cgltf_write_stritem(context, "KHR_materials_sheen");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_VARIANTS) {
		cgltf_write_stritem(context, "KHR_materials_variants");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_VOLUME) {
		cgltf_write_stritem(context, "KHR_materials_volume");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_TEXTURE_BASISU) {
		cgltf_write_stritem(context, "KHR_texture_basisu");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_TEXTURE_WEBP) {
		cgltf_write_stritem(context, "EXT_texture_webp");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_EMISSIVE_STRENGTH) {
		cgltf_write_stritem(context, "KHR_materials_emissive_strength");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_IRIDESCENCE) {
		cgltf_write_stritem(context, "KHR_materials_iridescence");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_DIFFUSE_TRANSMISSION) {
		cgltf_write_stritem(context, "KHR_materials_diffuse_transmission");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_ANISOTROPY) {
		cgltf_write_stritem(context, "KHR_materials_anisotropy");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MESH_GPU_INSTANCING) {
		cgltf_write_stritem(context, "EXT_mesh_gpu_instancing");
	}
	if (extension_flags & CGLTF_EXTENSION_FLAG_MATERIALS_DISPERSION) {
		cgltf_write_stritem(context, "KHR_materials_dispersion");
	}
}

cgltf_size cgltf_write(const cgltf_options* options, char* buffer, cgltf_size size, const cgltf_data* data)
{
	(void)options;
	cgltf_write_context ctx;
	ctx.buffer = buffer;
	ctx.buffer_size = size;
	ctx.remaining = size;
	ctx.cursor = buffer;
	ctx.chars_written = 0;
	ctx.data = data;
	ctx.depth = 1;
	ctx.indent = "  ";
	ctx.needs_comma = 0;
	ctx.extension_flags = 0;
	ctx.required_extension_flags = 0;

	cgltf_write_context* context = &ctx;

	CGLTF_SPRINTF("{");

	if (data->accessors_count > 0)
	{
		cgltf_write_line(context, "\"accessors\": [");
		for (cgltf_size i = 0; i < data->accessors_count; ++i)
		{
			cgltf_write_accessor(context, data->accessors + i);
		}
		cgltf_write_line(context, "]");
	}

	cgltf_write_asset(context, &data->asset);

	if (data->buffer_views_count > 0)
	{
		cgltf_write_line(context, "\"bufferViews\": [");
		for (cgltf_size i = 0; i < data->buffer_views_count; ++i)
		{
			cgltf_write_buffer_view(context, data->buffer_views + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->buffers_count > 0)
	{
		cgltf_write_line(context, "\"buffers\": [");
		for (cgltf_size i = 0; i < data->buffers_count; ++i)
		{
			cgltf_write_buffer(context, data->buffers + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->images_count > 0)
	{
		cgltf_write_line(context, "\"images\": [");
		for (cgltf_size i = 0; i < data->images_count; ++i)
		{
			cgltf_write_image(context, data->images + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->meshes_count > 0)
	{
		cgltf_write_line(context, "\"meshes\": [");
		for (cgltf_size i = 0; i < data->meshes_count; ++i)
		{
			cgltf_write_mesh(context, data->meshes + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->materials_count > 0)
	{
		cgltf_write_line(context, "\"materials\": [");
		for (cgltf_size i = 0; i < data->materials_count; ++i)
		{
			cgltf_write_material(context, data->materials + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->nodes_count > 0)
	{
		cgltf_write_line(context, "\"nodes\": [");
		for (cgltf_size i = 0; i < data->nodes_count; ++i)
		{
			cgltf_write_node(context, data->nodes + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->samplers_count > 0)
	{
		cgltf_write_line(context, "\"samplers\": [");
		for (cgltf_size i = 0; i < data->samplers_count; ++i)
		{
			cgltf_write_sampler(context, data->samplers + i);
		}
		cgltf_write_line(context, "]");
	}

	CGLTF_WRITE_IDXPROP("scene", data->scene, data->scenes);

	if (data->scenes_count > 0)
	{
		cgltf_write_line(context, "\"scenes\": [");
		for (cgltf_size i = 0; i < data->scenes_count; ++i)
		{
			cgltf_write_scene(context, data->scenes + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->textures_count > 0)
	{
		cgltf_write_line(context, "\"textures\": [");
		for (cgltf_size i = 0; i < data->textures_count; ++i)
		{
			cgltf_write_texture(context, data->textures + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->skins_count > 0)
	{
		cgltf_write_line(context, "\"skins\": [");
		for (cgltf_size i = 0; i < data->skins_count; ++i)
		{
			cgltf_write_skin(context, data->skins + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->animations_count > 0)
	{
		cgltf_write_line(context, "\"animations\": [");
		for (cgltf_size i = 0; i < data->animations_count; ++i)
		{
			cgltf_write_animation(context, data->animations + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->cameras_count > 0)
	{
		cgltf_write_line(context, "\"cameras\": [");
		for (cgltf_size i = 0; i < data->cameras_count; ++i)
		{
			cgltf_write_camera(context, data->cameras + i);
		}
		cgltf_write_line(context, "]");
	}

	if (data->lights_count > 0 || data->variants_count > 0)
	{
		cgltf_write_line(context, "\"extensions\": {");

		if (data->lights_count > 0)
		{
			cgltf_write_line(context, "\"KHR_lights_punctual\": {");
			cgltf_write_line(context, "\"lights\": [");
			for (cgltf_size i = 0; i < data->lights_count; ++i)
			{
				cgltf_write_light(context, data->lights + i);
			}
			cgltf_write_line(context, "]");
			cgltf_write_line(context, "}");
		}

		if (data->variants_count)
		{
			cgltf_write_line(context, "\"KHR_materials_variants\": {");
			cgltf_write_line(context, "\"variants\": [");
			for (cgltf_size i = 0; i < data->variants_count; ++i)
			{
				cgltf_write_variant(context, data->variants + i);
			}
			cgltf_write_line(context, "]");
			cgltf_write_line(context, "}");
		}

		cgltf_write_line(context, "}");
	}

	if (context->extension_flags != 0)
	{
		cgltf_write_line(context, "\"extensionsUsed\": [");
		cgltf_write_extensions(context, context->extension_flags);
		cgltf_write_line(context, "]");
	}

	if (context->required_extension_flags != 0)
	{
		cgltf_write_line(context, "\"extensionsRequired\": [");
		cgltf_write_extensions(context, context->required_extension_flags);
		cgltf_write_line(context, "]");
	}

	cgltf_write_extras(context, &data->extras);

	CGLTF_SPRINTF("\n}\n");

	// snprintf does not include the null terminator in its return value, so be sure to include it
	// in the returned byte count.
	return 1 + ctx.chars_written;
}

#endif /* #ifdef CGLTF_WRITE_IMPLEMENTATION */

/* cgltf is distributed under MIT license:
 *
 * Copyright (c) 2019-2021 Philip Rideout

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
