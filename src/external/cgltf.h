#ifndef CGLTF_H_INCLUDED__
#define CGLTF_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned long cgltf_size;
typedef float cgltf_float;
typedef int cgltf_bool;

typedef enum cgltf_file_type
{
	cgltf_file_type_invalid,
	cgltf_file_type_gltf,
	cgltf_file_type_glb,
} cgltf_file_type;

typedef struct cgltf_options
{
	cgltf_file_type type;
	cgltf_size json_token_count; /* 0 == auto */
	void* (*memory_alloc)(void* user, cgltf_size size);
	void (*memory_free) (void* user, void* ptr);
	void* memory_user_data;
} cgltf_options;

typedef enum cgltf_result
{
	cgltf_result_success,
	cgltf_result_data_too_short,
	cgltf_result_unknown_format,
	cgltf_result_invalid_json,
	cgltf_result_invalid_options,
} cgltf_result;

typedef enum cgltf_buffer_view_type
{
	cgltf_buffer_view_type_invalid,
	cgltf_buffer_view_type_indices,
	cgltf_buffer_view_type_vertices,
} cgltf_buffer_view_type;

typedef enum cgltf_attribute_type
{
	cgltf_attribute_type_invalid,
	cgltf_attribute_type_position,
	cgltf_attribute_type_normal,
	cgltf_attribute_type_tangent,
	cgltf_attribute_type_texcoord_0,
	cgltf_attribute_type_texcoord_1,
	cgltf_attribute_type_color_0,
	cgltf_attribute_type_joints_0,
	cgltf_attribute_type_weights_0,
} cgltf_attribute_type;

typedef enum cgltf_component_type
{
	cgltf_component_type_invalid,
	cgltf_component_type_rgb_32f,
	cgltf_component_type_rgba_32f,
	cgltf_component_type_rg_32f,
	cgltf_component_type_rg_8,
	cgltf_component_type_rg_16,
	cgltf_component_type_rgba_8,
	cgltf_component_type_rgba_16,
	cgltf_component_type_r_8,
	cgltf_component_type_r_8u,
	cgltf_component_type_r_16,
	cgltf_component_type_r_16u,
	cgltf_component_type_r_32u,
	cgltf_component_type_r_32f,
} cgltf_component_type;

typedef enum cgltf_type
{
	cgltf_type_invalid,
	cgltf_type_scalar,
	cgltf_type_vec2,
	cgltf_type_vec3,
	cgltf_type_vec4,
	cgltf_type_mat2,
	cgltf_type_mat3,
	cgltf_type_mat4,
} cgltf_type;

typedef enum cgltf_primitive_type
{
	cgltf_type_points,
	cgltf_type_lines,
	cgltf_type_line_loop,
	cgltf_type_line_strip,
	cgltf_type_triangles,
	cgltf_type_triangle_strip,
	cgltf_type_triangle_fan,
} cgltf_primitive_type;

typedef struct cgltf_buffer
{
	cgltf_size size;
	char* uri;
} cgltf_buffer;

typedef struct cgltf_buffer_view
{
	cgltf_buffer* buffer;
	cgltf_size offset;
	cgltf_size size;
	cgltf_size stride; /* 0 == automatically determined by accessor */
	cgltf_buffer_view_type type;
} cgltf_buffer_view;

typedef struct cgltf_accessor
{
	cgltf_component_type component_type;
	cgltf_type type;
	cgltf_size offset;
	cgltf_size count;
	cgltf_size stride;
	cgltf_buffer_view* buffer_view;
} cgltf_accessor;

typedef struct cgltf_attribute
{
	cgltf_attribute_type name;
	cgltf_accessor* data;
} cgltf_attribute;


typedef struct cgltf_rgba 
{
	cgltf_float r;
	cgltf_float g;
	cgltf_float b;
	cgltf_float a;
} cgltf_rgba;

typedef struct cgltf_image 
{
	char* uri;
	cgltf_buffer_view* buffer_view;
	char* mime_type;
} cgltf_image;

typedef struct cgltf_sampler
{
	cgltf_float mag_filter;
	cgltf_float min_filter;
	cgltf_float wrap_s;
	cgltf_float wrap_t;
} cgltf_sampler;

typedef struct cgltf_texture
{
	cgltf_image* image;
	cgltf_sampler* sampler;
} cgltf_texture;

typedef struct cgltf_texture_view
{	
	cgltf_texture* texture;
	cgltf_size texcoord;
	cgltf_float scale;
} cgltf_texture_view;

typedef struct cgltf_pbr
{
	cgltf_texture_view base_color_texture;
	cgltf_texture_view metallic_roughness_texture;

	cgltf_rgba base_color;
	cgltf_float metallic_factor;
	cgltf_float roughness_factor;
} cgltf_pbr;

typedef struct cgltf_material
{
	char* name;
	cgltf_pbr pbr;
	cgltf_rgba emissive_color;
	cgltf_texture_view normal_texture;
	cgltf_texture_view emissive_texture;
	cgltf_texture_view occlusion_texture;
	cgltf_bool double_sided;
} cgltf_material;

typedef struct cgltf_primitive {
	cgltf_primitive_type type;
	cgltf_accessor* indices;
	cgltf_material* material;
	cgltf_attribute* attributes;
	cgltf_size attributes_count;
} cgltf_primitive;

typedef struct cgltf_mesh {
	char* name;
	cgltf_primitive* primitives;
	cgltf_size primitives_count;
} cgltf_mesh;

typedef struct cgltf_data
{
	unsigned version;
	cgltf_file_type file_type;

	cgltf_mesh* meshes;
	cgltf_size meshes_count;

	cgltf_material* materials;
	cgltf_size materials_count;

	cgltf_accessor* accessors;
	cgltf_size accessors_count;

	cgltf_buffer_view* buffer_views;
	cgltf_size buffer_views_count;

	cgltf_buffer* buffers;
	cgltf_size buffers_count;

	cgltf_image* images;
	cgltf_size images_count;

	cgltf_texture* textures;
	cgltf_size textures_count;

	cgltf_sampler* samplers;
	cgltf_size samplers_count;

	const void* bin;
	cgltf_size bin_size;

	void (*memory_free) (void* user, void* ptr);
	void* memory_user_data;
} cgltf_data;

cgltf_result cgltf_parse(
		const cgltf_options* options,
		const void* data,
		cgltf_size size,
		cgltf_data* out_data);

void cgltf_free(cgltf_data* data);

#endif /* #ifndef CGLTF_H_INCLUDED__ */

/*
 *
 * Stop now, if you are only interested in the API.
 * Below, you find the implementation.
 *
 */

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define CGLTF_IMPLEMENTATION
#endif

#ifdef CGLTF_IMPLEMENTATION

#include <stdint.h> /* For uint8_t, uint32_t */
#include <string.h> /* For strncpy */
#include <stdlib.h> /* For malloc, free */


/*
 * -- jsmn.h start --
 * Source: https://github.com/zserge/jsmn
 * License: MIT
 */
typedef enum {
	JSMN_UNDEFINED = 0,
	JSMN_OBJECT = 1,
	JSMN_ARRAY = 2,
	JSMN_STRING = 3,
	JSMN_PRIMITIVE = 4
} jsmntype_t;
enum jsmnerr {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3
};
typedef struct {
	jsmntype_t type;
	int start;
	int end;
	int size;
#ifdef JSMN_PARENT_LINKS
	int parent;
#endif
} jsmntok_t;
typedef struct {
	unsigned int pos; /* offset in the JSON string */
	unsigned int toknext; /* next token to allocate */
	int toksuper; /* superior token node, e.g parent object or array */
} jsmn_parser;
void jsmn_init(jsmn_parser *parser);
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len, jsmntok_t *tokens, unsigned int num_tokens);
/*
 * -- jsmn.h end --
 */


static const cgltf_size GltfHeaderSize = 12;
static const cgltf_size GltfChunkHeaderSize = 8;
static const uint32_t GltfMagic = 0x46546C67;
static const uint32_t GltfMagicJsonChunk = 0x4E4F534A;
static const uint32_t GltfMagicBinChunk = 0x004E4942;

static void* cgltf_mem_alloc(void* user, cgltf_size size)
{
	return malloc(size);
}

static void cgltf_mem_free(void* user, void* ptr)
{
	free(ptr);
}

static cgltf_result cgltf_parse_json(cgltf_options* options, const uint8_t* json_chunk, cgltf_size size, cgltf_data* out_data);

cgltf_result cgltf_parse(const cgltf_options* options, const void* data, cgltf_size size, cgltf_data* out_data)
{
	if (size < GltfHeaderSize)
	{
		return cgltf_result_data_too_short;
	}

	if (options == NULL)
	{
		return cgltf_result_invalid_options;
	}

	cgltf_options fixed_options = *options;
	if (fixed_options.memory_alloc == NULL)
	{
		fixed_options.memory_alloc = &cgltf_mem_alloc;
	}
	if (fixed_options.memory_free == NULL)
	{
		fixed_options.memory_free = &cgltf_mem_free;
	}

	uint32_t tmp;
	// Magic
	memcpy(&tmp, data, 4);
	if (tmp != GltfMagic)
	{
		if (fixed_options.type == cgltf_file_type_invalid)
		{
			fixed_options.type = cgltf_file_type_gltf;
		}
		else
		{
			return cgltf_result_unknown_format;
		}
	}

	memset(out_data, 0, sizeof(cgltf_data));
	out_data->memory_free = fixed_options.memory_free;
	out_data->memory_user_data = fixed_options.memory_user_data;

	if (fixed_options.type == cgltf_file_type_gltf)
	{
		out_data->file_type = cgltf_file_type_gltf;
		return cgltf_parse_json(&fixed_options, data, size, out_data);
	}

	const uint8_t* ptr = (const uint8_t*)data;
	// Version
	memcpy(&tmp, ptr + 4, 4);
	out_data->version = tmp;

	// Total length
	memcpy(&tmp, ptr + 8, 4);
	if (tmp > size)
	{
		return cgltf_result_data_too_short;
	}

	const uint8_t* json_chunk = ptr + GltfHeaderSize;

	// JSON chunk: length
	uint32_t json_length;
	memcpy(&json_length, json_chunk, 4);
	if (GltfHeaderSize + GltfChunkHeaderSize + json_length > size)
	{
		return cgltf_result_data_too_short;
	}

	// JSON chunk: magic
	memcpy(&tmp, json_chunk + 4, 4);
	if (tmp != GltfMagicJsonChunk)
	{
		return cgltf_result_unknown_format;
	}

	json_chunk += GltfChunkHeaderSize;
	cgltf_result json_result = cgltf_parse_json(&fixed_options, json_chunk, json_length, out_data);
	if (json_result != cgltf_result_success)
	{
		return json_result;
	}

	out_data->file_type = cgltf_file_type_invalid;
	if (GltfHeaderSize + GltfChunkHeaderSize + json_length + GltfChunkHeaderSize <= size)
	{
		// We can read another chunk
		const uint8_t* bin_chunk = json_chunk + json_length;

		// Bin chunk: length
		uint32_t bin_length;
		memcpy(&bin_length, bin_chunk, 4);
		if (GltfHeaderSize + GltfChunkHeaderSize + json_length + GltfChunkHeaderSize + bin_length > size)
		{
			return cgltf_result_data_too_short;
		}

		// Bin chunk: magic
		memcpy(&tmp, bin_chunk + 4, 4);
		if (tmp != GltfMagicBinChunk)
		{
			return cgltf_result_unknown_format;
		}

		bin_chunk += GltfChunkHeaderSize;

		out_data->file_type = cgltf_file_type_glb;
		out_data->bin = bin_chunk;
		out_data->bin_size = bin_length;
	}

	return cgltf_result_success;
}

void cgltf_free(cgltf_data* data)
{
	data->memory_free(data->memory_user_data, data->accessors);
	data->memory_free(data->memory_user_data, data->buffer_views);


	for (cgltf_size i = 0; i < data->buffers_count; ++i)
	{
		data->memory_free(data->memory_user_data, data->buffers[i].uri);
	}
	data->memory_free(data->memory_user_data, data->buffers);

	for (cgltf_size i = 0; i < data->meshes_count; ++i)
	{
		data->memory_free(data->memory_user_data, data->meshes[i].name);
		for (cgltf_size j = 0; j < data->meshes[i].primitives_count; ++j)
		{
			data->memory_free(data->memory_user_data, data->meshes[i].primitives[j].attributes);
		}
		data->memory_free(data->memory_user_data, data->meshes[i].primitives);
	}
	data->memory_free(data->memory_user_data, data->meshes);

	for (cgltf_size i = 0; i < data->materials_count; ++i)
	{
		data->memory_free(data->memory_user_data, data->materials[i].name);
	}

	data->memory_free(data->memory_user_data, data->materials);

	for (cgltf_size i = 0; i < data->images_count; ++i) 
	{
		data->memory_free(data->memory_user_data, data->images[i].uri);
		data->memory_free(data->memory_user_data, data->images[i].mime_type);
	}

	data->memory_free(data->memory_user_data, data->images);
	data->memory_free(data->memory_user_data, data->textures);
	data->memory_free(data->memory_user_data, data->samplers);
}

#define CGLTF_CHECK_TOKTYPE(tok_, type_) if ((tok_).type != (type_)) { return -128; }

static char cgltf_to_lower(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		c = 'a' + (c - 'A');
	}
	return c;
}

static int cgltf_json_strcmp(jsmntok_t const* tok, const uint8_t* json_chunk, const char* str)
{
	CGLTF_CHECK_TOKTYPE(*tok, JSMN_STRING);
	int const str_len = strlen(str);
	int const name_length = tok->end - tok->start;
	if (name_length == str_len)
	{
		for (int i = 0; i < str_len; ++i)
		{
			char const a = cgltf_to_lower(*((const char*)json_chunk + tok->start + i));
			char const b = cgltf_to_lower(*(str + i));
			if (a < b)
			{
				return -1;
			}
			else if (a > b)
			{
				return 1;
			}
		}
		return 0;
	}
	return 128;
}

static int cgltf_json_to_int(jsmntok_t const* tok, const uint8_t* json_chunk)
{
	char tmp[128];
	CGLTF_CHECK_TOKTYPE(*tok, JSMN_PRIMITIVE);
	int size = tok->end - tok->start;
	strncpy(tmp,
		(const char*)json_chunk + tok->start,
		size);
	tmp[size] = 0;
	return atoi(tmp);
}

static cgltf_float cgltf_json_to_float(jsmntok_t const* tok, const uint8_t* json_chunk) {
	char tmp[128];
	CGLTF_CHECK_TOKTYPE(*tok, JSMN_PRIMITIVE);
	int size = tok->end - tok->start;
	strncpy(tmp,
		(const char*)json_chunk + tok->start,
		size);
	tmp[size] = 0;
	return atof(tmp);
}

static cgltf_bool cgltf_json_to_bool(jsmntok_t const* tok, const uint8_t* json_chunk) {
	//TODO: error handling?
	if (memcmp(json_chunk + tok->start, "true", 4) == 0)
		return 1;

	return 0;
}

static int cgltf_skip_json(jsmntok_t const* tokens, int i)
{
	if (tokens[i].type == JSMN_ARRAY)
	{
		int size = tokens[i].size;
		++i;
		for (int j = 0; j < size; ++j)
		{
			i = cgltf_skip_json(tokens, i);
		}
	}
	else if (tokens[i].type == JSMN_OBJECT)
	{
		int size = tokens[i].size;
		++i;
		for (int j = 0; j < size; ++j)
		{
			i = cgltf_skip_json(tokens, i);
			i = cgltf_skip_json(tokens, i);
		}
	}
	else if (tokens[i].type == JSMN_PRIMITIVE
		 || tokens[i].type == JSMN_STRING)
	{
		return i + 1;
	}
	return i;
}


static int cgltf_parse_json_primitive(cgltf_options* options, jsmntok_t const* tokens, int i,
				      const uint8_t* json_chunk,
				      cgltf_primitive* out_prim)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	int size = tokens[i].size;
	++i;

	out_prim->indices = (void* )-1;
	out_prim->material = NULL;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens+i, json_chunk, "mode") == 0)
		{
			++i;
			out_prim->type
					= (cgltf_primitive_type)
					cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "indices") == 0)
		{
			++i;
			out_prim->indices =
					(void*)(size_t)cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "material") == 0)
		{
			++i;
			out_prim->material =
				(void*)(size_t)cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "attributes") == 0)
		{
			++i;
			if (tokens[i].type != JSMN_OBJECT)
			{
				return -1;
			}
			out_prim->attributes_count = tokens[i].size;
			out_prim->attributes
					= options->memory_alloc(options->memory_user_data, sizeof(cgltf_attribute) * tokens[i].size);
			++i;
			for (cgltf_size iattr = 0; iattr < out_prim->attributes_count; ++iattr)
			{
				CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_STRING);
				out_prim->attributes[iattr].name = cgltf_attribute_type_invalid;
				if (cgltf_json_strcmp(tokens+i, json_chunk, "POSITION") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_position;
				}
				else if (cgltf_json_strcmp(tokens+i, json_chunk, "NORMAL") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_normal;
				}
				else if (cgltf_json_strcmp(tokens+i, json_chunk, "TANGENT") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_tangent;
				}
				else if (cgltf_json_strcmp(tokens+i, json_chunk, "TEXCOORD_0") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_texcoord_0;
				}
				else if (cgltf_json_strcmp(tokens+i, json_chunk, "TEXCOORD_1") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_texcoord_1;
				}
				else if (cgltf_json_strcmp(tokens+i, json_chunk, "COLOR_0") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_color_0;
				}
				else if (cgltf_json_strcmp(tokens+i, json_chunk, "JOINTS_0") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_joints_0;
				}
				else if (cgltf_json_strcmp(tokens+i, json_chunk, "WEIGHTS_0") == 0)
				{
					out_prim->attributes[iattr].name = cgltf_attribute_type_weights_0;
				}
				++i;
				out_prim->attributes[iattr].data =
						(void*)(size_t)cgltf_json_to_int(tokens+i, json_chunk);
				++i;
			}
		}
		else
		{
			i = cgltf_skip_json(tokens, i+1);
		}
	}

	return i;
}

static int cgltf_parse_json_mesh(cgltf_options* options, jsmntok_t const* tokens, int i,
				 const uint8_t* json_chunk, cgltf_size mesh_index,
				 cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	out_data->meshes[mesh_index].name = NULL;

	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens+i, json_chunk, "name") == 0)
		{
			++i;
			int strsize = tokens[i].end - tokens[i].start;
			out_data->meshes[mesh_index].name = options->memory_alloc(options->memory_user_data, strsize + 1);
			strncpy(out_data->meshes[mesh_index].name,
				(const char*)json_chunk + tokens[i].start,
				strsize);
			out_data->meshes[mesh_index].name[strsize] = 0;
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "primitives") == 0)
		{
			++i;
			if (tokens[i].type != JSMN_ARRAY)
			{
				return -1;
			}
			out_data->meshes[mesh_index].primitives_count = tokens[i].size;
			out_data->meshes[mesh_index].primitives = options->memory_alloc(options->memory_user_data, sizeof(cgltf_primitive) * tokens[i].size);
			++i;

			for (cgltf_size prim_index = 0;
			     prim_index < out_data->meshes[mesh_index].primitives_count;
			     ++prim_index)
			{
				i = cgltf_parse_json_primitive(options, tokens, i, json_chunk,
							       &out_data->meshes[mesh_index].primitives[prim_index]);
				if (i < 0)
				{
					return i;
				}
			}
		}
		else
		{
			i = cgltf_skip_json(tokens, i+1);
		}
	}

	return i;
}

static int cgltf_parse_json_meshes(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->meshes_count = tokens[i].size;
	out_data->meshes = options->memory_alloc(options->memory_user_data, sizeof(cgltf_mesh) * out_data->meshes_count);
	++i;
	for (cgltf_size j = 0 ; j < out_data->meshes_count; ++j)
	{
		i = cgltf_parse_json_mesh(options, tokens, i, json_chunk, j, out_data);
		if (i < 0)
		{
			return i;
		}
	}
	return i;
}

static int cgltf_parse_json_accessor(jsmntok_t const* tokens, int i,
				     const uint8_t* json_chunk, cgltf_size accessor_index,
				     cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	memset(&out_data->accessors[accessor_index], 0, sizeof(cgltf_accessor));
	out_data->accessors[accessor_index].buffer_view = (void*)-1;

	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens+i, json_chunk, "bufferView") == 0)
		{
			++i;
			out_data->accessors[accessor_index].buffer_view =
					(void*)(size_t)cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "byteOffset") == 0)
		{
			++i;
			out_data->accessors[accessor_index].offset =
					cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "componentType") == 0)
		{
			++i;
			int type = cgltf_json_to_int(tokens+i, json_chunk);
			switch (type)
			{
			case 5120:
				type = cgltf_component_type_r_8;
				break;
			case 5121:
				type = cgltf_component_type_r_8u;
				break;
			case 5122:
				type = cgltf_component_type_r_16;
				break;
			case 5123:
				type = cgltf_component_type_r_16u;
				break;
			case 5125:
				type = cgltf_component_type_r_32u;
				break;
			case 5126:
				type = cgltf_component_type_r_32f;
				break;
			default:
				type = cgltf_component_type_invalid;
				break;
			}
			out_data->accessors[accessor_index].component_type = type;
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "count") == 0)
		{
			++i;
			out_data->accessors[accessor_index].count =
					cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "type") == 0)
		{
			++i;
			if (cgltf_json_strcmp(tokens+i, json_chunk, "SCALAR") == 0)
			{
				out_data->accessors[accessor_index].type = cgltf_type_scalar;
			}
			else if (cgltf_json_strcmp(tokens+i, json_chunk, "VEC2") == 0)
			{
				out_data->accessors[accessor_index].type = cgltf_type_vec2;
			}
			else if (cgltf_json_strcmp(tokens+i, json_chunk, "VEC3") == 0)
			{
				out_data->accessors[accessor_index].type = cgltf_type_vec3;
			}
			else if (cgltf_json_strcmp(tokens+i, json_chunk, "VEC4") == 0)
			{
				out_data->accessors[accessor_index].type = cgltf_type_vec4;
			}
			else if (cgltf_json_strcmp(tokens+i, json_chunk, "MAT2") == 0)
			{
				out_data->accessors[accessor_index].type = cgltf_type_mat2;
			}
			else if (cgltf_json_strcmp(tokens+i, json_chunk, "MAT3") == 0)
			{
				out_data->accessors[accessor_index].type = cgltf_type_mat3;
			}
			else if (cgltf_json_strcmp(tokens+i, json_chunk, "MAT4") == 0)
			{
				out_data->accessors[accessor_index].type = cgltf_type_mat4;
			}
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i+1);
		}
	}

	return i;
}

static int cgltf_parse_json_rgba(jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_rgba* out)
{
	int components = tokens[i].size;
	if (components >= 2) {
		out->r = cgltf_json_to_float(tokens + ++i, json_chunk);
		out->g = cgltf_json_to_float(tokens + ++i, json_chunk);

		if (components > 2)
			out->b = cgltf_json_to_float(tokens + ++i, json_chunk);

		if (components > 3)
			out->a = cgltf_json_to_float(tokens + ++i, json_chunk);
	}
	else {
		out->r = cgltf_json_to_float(tokens + ++i, json_chunk);
		out->g = out->r;
		out->b = out->r;
		out->a = out->r;
	}

	return ++i;
}

static int cgltf_parse_json_texture_view(jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_texture_view* out) {

	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens + i, json_chunk, "index") == 0)
		{
			++i;
			out->texture = (void*)(size_t)cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "texCoord") == 0)
		{
			++i;
			out->texcoord = cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "scale") == 0) 
		{
			++i;
			out->scale = cgltf_json_to_float(tokens + i, json_chunk);
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i + 1);
		}
	}

	return i;
}

static int cgltf_parse_json_pbr(jsmntok_t const* tokens, int i,
				     const uint8_t* json_chunk, cgltf_size mat_index, cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens+i, json_chunk, "metallicFactor") == 0)
		{
			++i;
			out_data->materials[mat_index].pbr.metallic_factor = 
				cgltf_json_to_float(tokens + i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "roughnessFactor") == 0) 
		{
			++i;
			out_data->materials[mat_index].pbr.roughness_factor =
				cgltf_json_to_float(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "baseColorFactor") == 0)
		{
			i = cgltf_parse_json_rgba(tokens, i + 1, json_chunk,
					&(out_data->materials[mat_index].pbr.base_color));
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "baseColorTexture") == 0)
		{
			i = cgltf_parse_json_texture_view(tokens, i + 1, json_chunk,
				&(out_data->materials[mat_index].pbr.base_color_texture));
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "metallicRoughnessTexture") == 0)
		{
			i = cgltf_parse_json_texture_view(tokens, i + 1, json_chunk,
				&(out_data->materials[mat_index].pbr.metallic_roughness_texture));
		}
		else
		{
			i = cgltf_skip_json(tokens, i+1);
		}
	}

	return i;
}

static int cgltf_parse_json_image(cgltf_options* options, jsmntok_t const* tokens, int i,
	const uint8_t* json_chunk, cgltf_size img_index, cgltf_data* out_data) 
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	memset(&out_data->images[img_index], 0, sizeof(cgltf_image));
	int size = tokens[i].size;
	++i;

	out_data->images[img_index].buffer_view = (void*)-1;

	for (int j = 0; j < size; ++j) 
	{
		if (cgltf_json_strcmp(tokens + i, json_chunk, "uri") == 0) 
		{
			++i;
			int strsize = tokens[i].end - tokens[i].start;
			out_data->images[img_index].uri = options->memory_alloc(options->memory_user_data, strsize + 1);
			strncpy(out_data->images[img_index].uri,
				(const char*)json_chunk + tokens[i].start,
				strsize);
			out_data->images[img_index].uri[strsize] = 0;
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "bufferView") == 0)
		{
			++i;
			out_data->images[img_index].buffer_view =
					(void*)(size_t)cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "mimeType") == 0)
		{
			++i;
			int strsize = tokens[i].end - tokens[i].start;
			out_data->images[img_index].mime_type = options->memory_alloc(options->memory_user_data, strsize + 1);
			strncpy(out_data->images[img_index].mime_type,
				(const char*)json_chunk + tokens[i].start,
				strsize);
			out_data->images[img_index].mime_type[strsize] = 0;
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i + 1);
		}
	}

	return i;
}

static int cgltf_parse_json_sampler(cgltf_options* options, jsmntok_t const* tokens, int i,
	const uint8_t* json_chunk, cgltf_size smp_index, cgltf_data* out_data) {
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	memset(&out_data->samplers[smp_index], 0, sizeof(cgltf_sampler));
	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens + i, json_chunk, "magFilter") == 0) 
		{
			++i;
			out_data->samplers[smp_index].mag_filter
				= cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "minFilter") == 0)
		{
			++i;
			out_data->samplers[smp_index].min_filter
				= cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "wrapS") == 0)
		{
			++i;
			out_data->samplers[smp_index].wrap_s
				= cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "wrapT") == 0) 
		{
			++i;
			out_data->samplers[smp_index].wrap_t
				= cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i + 1);
		}
	}

	return i;
}


static int cgltf_parse_json_texture(cgltf_options* options, jsmntok_t const* tokens, int i,
	const uint8_t* json_chunk, cgltf_size tex_index, cgltf_data* out_data) {
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	memset(&out_data->textures[tex_index], 0, sizeof(cgltf_texture));
	out_data->textures[tex_index].image = (void*)-1;
	out_data->textures[tex_index].sampler = (void*)-1;

	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens + i, json_chunk, "sampler") == 0)
		{
			++i;
			out_data->textures[tex_index].sampler 
				= (void*)(size_t)cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "source") == 0) 
		{
			++i;
			out_data->textures[tex_index].image
				= (void*)(size_t)cgltf_json_to_int(tokens + i, json_chunk);
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i + 1);
		}
	}

	return i;
}

static int cgltf_parse_json_material(cgltf_options* options, jsmntok_t const* tokens, int i,
				     const uint8_t* json_chunk, cgltf_size mat_index, cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
	cgltf_material* material = &out_data->materials[mat_index];

	memset(material, 0, sizeof(cgltf_material));
	material->emissive_texture.texture = (void*)-1;
	material->emissive_texture.scale = 1.0f;

	material->normal_texture.texture = (void*)-1;
	material->normal_texture.scale = 1.0f;

	material->occlusion_texture.texture = (void*)-1;
	material->occlusion_texture.scale = 1.0f;

	material->pbr.base_color_texture.texture = (void*)-1;
	material->pbr.base_color_texture.scale = 1.0f;

	material->pbr.metallic_roughness_texture.texture = (void*)-1;
	material->pbr.metallic_roughness_texture.scale = 1.0f;

	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens+i, json_chunk, "name") == 0)
		{
			++i;
			int strsize = tokens[i].end - tokens[i].start;
			material->name = options->memory_alloc(options->memory_user_data, strsize + 1);
			strncpy(material->name,
				(const char*)json_chunk + tokens[i].start,
				strsize);
			material->name[strsize] = 0;
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "pbrMetallicRoughness") == 0) 
		{
			i = cgltf_parse_json_pbr(tokens, i+1, json_chunk, mat_index, out_data);
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "emissiveFactor") == 0) 
		{
			i = cgltf_parse_json_rgba(tokens, i + 1, json_chunk, 
				&(material->emissive_color));
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "normalTexture") == 0)
		{
			i = cgltf_parse_json_texture_view(tokens, i + 1, json_chunk,
				&(material->normal_texture));
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "emissiveTexture") == 0)
		{
			i = cgltf_parse_json_texture_view(tokens, i + 1, json_chunk,
				&(material->emissive_texture));
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "occlusionTexture") == 0) 
		{
			i = cgltf_parse_json_texture_view(tokens, i + 1, json_chunk,
				&(material->occlusion_texture));
		}
		else if (cgltf_json_strcmp(tokens + i, json_chunk, "doubleSided") == 0) 			
		{
			++i;
			material->double_sided =
				cgltf_json_to_bool(tokens + i, json_chunk);
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i+1);
		}
	}

	return i;
}

static int cgltf_parse_json_accessors(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->accessors_count = tokens[i].size;
	out_data->accessors = options->memory_alloc(options->memory_user_data, sizeof(cgltf_accessor) * out_data->accessors_count);
	++i;
	for (cgltf_size j = 0 ; j < out_data->accessors_count; ++j)
	{
		i = cgltf_parse_json_accessor(tokens, i, json_chunk, j, out_data);
		if (i < 0)
		{
			return i;
		}
	}
	return i;
}

static int cgltf_parse_json_materials(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->materials_count = tokens[i].size;
	out_data->materials = options->memory_alloc(options->memory_user_data, sizeof(cgltf_material) * out_data->materials_count);
	++i;
	for (cgltf_size j = 0; j < out_data->materials_count; ++j)
	{
		i = cgltf_parse_json_material(options, tokens, i, json_chunk, j, out_data);
		if (i < 0)
		{
			return i;
		}
	}
	return i;
}

static int cgltf_parse_json_images(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data) {
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->images_count = tokens[i].size;
	out_data->images = options->memory_alloc(options->memory_user_data, sizeof(cgltf_image) * out_data->images_count);
	++i;

	for (cgltf_size j = 0; j < out_data->images_count; ++j) {
		i = cgltf_parse_json_image(options, tokens, i, json_chunk, j, out_data);
		if (i < 0) {
			return i;
		}
	}
	return i;
}

static int cgltf_parse_json_textures(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data) {
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->textures_count = tokens[i].size;
	out_data->textures = options->memory_alloc(options->memory_user_data, sizeof(cgltf_texture) * out_data->textures_count);
	++i;

	for (cgltf_size j = 0; j < out_data->textures_count; ++j) {
		i = cgltf_parse_json_texture(options, tokens, i, json_chunk, j, out_data);
		if (i < 0) {
			return i;
		}
	}
	return i;
}

static int cgltf_parse_json_samplers(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data) {
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->samplers_count = tokens[i].size;
	out_data->samplers = options->memory_alloc(options->memory_user_data, sizeof(cgltf_sampler) * out_data->samplers_count);
	++i;

	for (cgltf_size j = 0; j < out_data->samplers_count; ++j) {
		i = cgltf_parse_json_sampler(options, tokens, i, json_chunk, j, out_data);
		if (i < 0) {
			return i;
		}
	}
	return i;
}

static int cgltf_parse_json_buffer_view(jsmntok_t const* tokens, int i,
					const uint8_t* json_chunk, cgltf_size buffer_view_index,
					cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	int size = tokens[i].size;
	++i;

	memset(&out_data->buffer_views[buffer_view_index], 0, sizeof(cgltf_buffer_view));

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens+i, json_chunk, "buffer") == 0)
		{
			++i;
			out_data->buffer_views[buffer_view_index].buffer =
					(void*)(size_t)cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "byteOffset") == 0)
		{
			++i;
			out_data->buffer_views[buffer_view_index].offset =
					cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "byteLength") == 0)
		{
			++i;
			out_data->buffer_views[buffer_view_index].size =
					cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "byteStride") == 0)
		{
			++i;
			out_data->buffer_views[buffer_view_index].stride =
					cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "target") == 0)
		{
			++i;
			int type = cgltf_json_to_int(tokens+i, json_chunk);
			switch (type)
			{
			case 34962:
				type = cgltf_buffer_view_type_vertices;
				break;
			case 34963:
				type = cgltf_buffer_view_type_indices;
				break;
			default:
				type = cgltf_buffer_view_type_invalid;
				break;
			}
			out_data->buffer_views[buffer_view_index].type = type;
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i+1);
		}
	}

	return i;
}

static int cgltf_parse_json_buffer_views(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->buffer_views_count = tokens[i].size;
	out_data->buffer_views = options->memory_alloc(options->memory_user_data, sizeof(cgltf_buffer_view) * out_data->buffer_views_count);
	++i;
	for (cgltf_size j = 0 ; j < out_data->buffer_views_count; ++j)
	{
		i = cgltf_parse_json_buffer_view(tokens, i, json_chunk, j, out_data);
		if (i < 0)
		{
			return i;
		}
	}
	return i;
}

static int cgltf_parse_json_buffer(cgltf_options* options, jsmntok_t const* tokens, int i,
				   const uint8_t* json_chunk, cgltf_size buffer_index,
				   cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);

	out_data->buffers[buffer_index].uri = NULL;

	int size = tokens[i].size;
	++i;

	for (int j = 0; j < size; ++j)
	{
		if (cgltf_json_strcmp(tokens+i, json_chunk, "byteLength") == 0)
		{
			++i;
			out_data->buffers[buffer_index].size =
					cgltf_json_to_int(tokens+i, json_chunk);
			++i;
		}
		else if (cgltf_json_strcmp(tokens+i, json_chunk, "uri") == 0)
		{
			++i;
			int strsize = tokens[i].end - tokens[i].start;
			out_data->buffers[buffer_index].uri = options->memory_alloc(options->memory_user_data, strsize + 1);
			strncpy(out_data->buffers[buffer_index].uri,
				(const char*)json_chunk + tokens[i].start,
				strsize);
			out_data->buffers[buffer_index].uri[strsize] = 0;
			++i;
		}
		else
		{
			i = cgltf_skip_json(tokens, i+1);
		}
	}

	return i;
}

static int cgltf_parse_json_buffers(cgltf_options* options, jsmntok_t const* tokens, int i, const uint8_t* json_chunk, cgltf_data* out_data)
{
	CGLTF_CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
	out_data->buffers_count = tokens[i].size;
	out_data->buffers = options->memory_alloc(options->memory_user_data, sizeof(cgltf_buffer) * out_data->buffers_count);
	++i;
	for (cgltf_size j = 0 ; j < out_data->buffers_count; ++j)
	{
		i = cgltf_parse_json_buffer(options, tokens, i, json_chunk, j, out_data);
		if (i < 0)
		{
			return i;
		}
	}
	return i;
}

static cgltf_size cgltf_calc_size(cgltf_type type, cgltf_component_type component_type)
{
	cgltf_type size = 0;

	switch (component_type)
	{
	case cgltf_component_type_rgb_32f:
		size = 12;
		break;
	case cgltf_component_type_rgba_32f:
		size = 16;
		break;
	case cgltf_component_type_rg_32f:
		size = 8;
		break;
	case cgltf_component_type_rg_8:
		size = 2;
		break;
	case cgltf_component_type_rg_16:
		size = 4;
		break;
	case cgltf_component_type_rgba_8:
		size = 4;
		break;
	case cgltf_component_type_rgba_16:
		size = 8;
		break;
	case cgltf_component_type_r_8:
	case cgltf_component_type_r_8u:
		size = 1;
		break;
	case cgltf_component_type_r_16:
	case cgltf_component_type_r_16u:
		size = 2;
		break;
	case cgltf_component_type_r_32u:
	case cgltf_component_type_r_32f:
		size = 4;
		break;
	case cgltf_component_type_invalid:
	default:
		size = 0;
		break;
	}

	switch (type)
	{
	case cgltf_type_vec2:
		size *= 2;
		break;
	case cgltf_type_vec3:
		size *= 3;
		break;
	case cgltf_type_vec4:
		size *= 4;
		break;
	case cgltf_type_mat2:
		size *= 4;
		break;
	case cgltf_type_mat3:
		size *= 9;
		break;
	case cgltf_type_mat4:
		size *= 16;
		break;
	case cgltf_type_invalid:
	case cgltf_type_scalar:
	default:
		size *= 1;
		break;
	}

	return size;
}

cgltf_result cgltf_parse_json(cgltf_options* options, const uint8_t* json_chunk, cgltf_size size, cgltf_data* out_data)
{
	jsmn_parser parser = {0};

	if (options->json_token_count == 0)
	{
		options->json_token_count = jsmn_parse(&parser, (const char*)json_chunk, size, NULL, 0);
	}

	jsmntok_t* tokens = options->memory_alloc(options->memory_user_data, sizeof(jsmntok_t) * options->json_token_count);

	jsmn_init(&parser);

	int token_count = jsmn_parse(&parser, (const char*)json_chunk, size, tokens, options->json_token_count);

	if (token_count < 0
			|| tokens[0].type != JSMN_OBJECT)
	{
		return cgltf_result_invalid_json;
	}

	// The root is an object.

	for (int i = 1; i < token_count; )
	{
		jsmntok_t const* tok = &tokens[i];
		if (tok->type == JSMN_STRING
				&& i + 1 < token_count)
		{
			int const name_length = tok->end - tok->start;
			if (name_length == 6
					&& strncmp((const char*)json_chunk + tok->start, "meshes", 6) == 0)
			{
				i = cgltf_parse_json_meshes(options, tokens, i+1, json_chunk, out_data);
			}
			else if (name_length == 9
				 && strncmp((const char*)json_chunk + tok->start, "accessors", 9) == 0)
			{
				i = cgltf_parse_json_accessors(options, tokens, i+1, json_chunk, out_data);
			}
			else if (name_length == 11
				 && strncmp((const char*)json_chunk + tok->start, "bufferViews", 11) == 0)
			{
				i = cgltf_parse_json_buffer_views(options, tokens, i+1, json_chunk, out_data);
			}
			else if (name_length == 7
				 && strncmp((const char*)json_chunk + tok->start, "buffers", 7) == 0)
			{
				i = cgltf_parse_json_buffers(options, tokens, i+1, json_chunk, out_data);
			}
			else if (name_length == 9
				&& strncmp((const char*)json_chunk + tok->start, "materials", 9) == 0) 		
			{
				i = cgltf_parse_json_materials(options, tokens, i+1, json_chunk, out_data);
			}
			else if (name_length == 6
				&& strncmp((const char*)json_chunk + tok->start, "images", 6) == 0) 
			{
				i = cgltf_parse_json_images(options, tokens, i + 1, json_chunk, out_data);
			}
			else if (name_length == 8
				&& strncmp((const char*)json_chunk + tok->start, "textures", 8) == 0) 
			{
				i = cgltf_parse_json_textures(options, tokens, i + 1, json_chunk, out_data);
			}
			else if (name_length == 8
				&& strncmp((const char*)json_chunk + tok->start, "samplers", 8) == 0)
			{
				i = cgltf_parse_json_samplers(options, tokens, i + 1, json_chunk, out_data);
			}
			else
			{
				i = cgltf_skip_json(tokens, i+1);
			}

			if (i < 0)
			{
				return cgltf_result_invalid_json;
			}
		}
	}

	options->memory_free(options->memory_user_data, tokens);

	/* Fix up pointers */
	for (cgltf_size i = 0; i < out_data->meshes_count; ++i)
	{
		for (cgltf_size j = 0; j < out_data->meshes[i].primitives_count; ++j)
		{
			if (out_data->meshes[i].primitives[j].indices ==(void*)-1)
			{
				out_data->meshes[i].primitives[j].indices = NULL;
			}
			else
			{
				out_data->meshes[i].primitives[j].indices
						= &out_data->accessors[(cgltf_size)out_data->meshes[i].primitives[j].indices];
			}

			for (cgltf_size k = 0; k < out_data->meshes[i].primitives[j].attributes_count; ++k)
			{
				out_data->meshes[i].primitives[j].attributes[k].data
						= &out_data->accessors[(cgltf_size)out_data->meshes[i].primitives[j].attributes[k].data];
			}
		}
	}

	for (cgltf_size i = 0; i < out_data->accessors_count; ++i)
	{
		if (out_data->accessors[i].buffer_view == (void*)-1)
		{
			out_data->accessors[i].buffer_view = NULL;
		}
		else
		{
			out_data->accessors[i].buffer_view
					= &out_data->buffer_views[(cgltf_size)out_data->accessors[i].buffer_view];
			out_data->accessors[i].stride = 0;
			if (out_data->accessors[i].buffer_view)
			{
				out_data->accessors[i].stride = out_data->accessors[i].buffer_view->stride;
			}
		}
		if (out_data->accessors[i].stride == 0)
		{
			out_data->accessors[i].stride = cgltf_calc_size(out_data->accessors[i].type, out_data->accessors[i].component_type);
		}
	}

	for (cgltf_size i = 0; i < out_data->textures_count; ++i) 
	{
		if (out_data->textures[i].image == (void*)-1) 
		{
			out_data->textures[i].image = NULL;
		}
		else
		{
			out_data->textures[i].image = 
				&out_data->images[(cgltf_size)out_data->textures[i].image];
		}

		if (out_data->textures[i].sampler == (void*)-1)
		{
			out_data->textures[i].sampler = NULL;
		}
		else 
		{
			out_data->textures[i].sampler =
				&out_data->samplers[(cgltf_size)out_data->textures[i].sampler];
		}
	}

	for (cgltf_size i = 0; i < out_data->images_count; ++i)
	{
		if (out_data->images[i].buffer_view == (void*)-1)
		{
			out_data->images[i].buffer_view = NULL;
		}
		else
		{
			out_data->images[i].buffer_view
					= &out_data->buffer_views[(cgltf_size)out_data->images[i].buffer_view];
		}
	}

	for (cgltf_size i = 0; i < out_data->materials_count; ++i)
	{
		if (out_data->materials[i].emissive_texture.texture == (void*)-1)
		{
			out_data->materials[i].emissive_texture.texture = NULL;
		}
		else
		{
			out_data->materials[i].emissive_texture.texture =
				&out_data->textures[(cgltf_size)out_data->materials[i].emissive_texture.texture];
		}

		if (out_data->materials[i].normal_texture.texture == (void*)-1)
		{
			out_data->materials[i].normal_texture.texture = NULL;
		}
		else 
		{
			out_data->materials[i].normal_texture.texture =
				&out_data->textures[(cgltf_size)out_data->materials[i].normal_texture.texture];
		}

		if (out_data->materials[i].occlusion_texture.texture == (void*)-1)
		{
			out_data->materials[i].occlusion_texture.texture = NULL;
		}
		else
		{
			out_data->materials[i].occlusion_texture.texture =
				&out_data->textures[(cgltf_size)out_data->materials[i].occlusion_texture.texture];
		}

		if (out_data->materials[i].pbr.base_color_texture.texture == (void*)-1)
		{
			out_data->materials[i].pbr.base_color_texture.texture = NULL;
		}
		else
		{
			out_data->materials[i].pbr.base_color_texture.texture =
				&out_data->textures[(cgltf_size)out_data->materials[i].pbr.base_color_texture.texture];
		}

		if (out_data->materials[i].pbr.metallic_roughness_texture.texture == (void*)-1)
		{
			out_data->materials[i].pbr.metallic_roughness_texture.texture = NULL;
		}
		else 
		{
			out_data->materials[i].pbr.metallic_roughness_texture.texture =
				&out_data->textures[(cgltf_size)out_data->materials[i].pbr.metallic_roughness_texture.texture];
		}
	}

	for (cgltf_size i = 0; i < out_data->buffer_views_count; ++i)
	{
		out_data->buffer_views[i].buffer
				= &out_data->buffers[(cgltf_size)out_data->buffer_views[i].buffer];
	}

	return cgltf_result_success;
}

/*
 * -- jsmn.c start --
 * Source: https://github.com/zserge/jsmn
 * License: MIT
 */
/**
 * Allocates a fresh unused token from the token pull.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser,
				   jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *tok;
	if (parser->toknext >= num_tokens) {
		return NULL;
	}
	tok = &tokens[parser->toknext++];
	tok->start = tok->end = -1;
	tok->size = 0;
#ifdef JSMN_PARENT_LINKS
	tok->parent = -1;
#endif
	return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, jsmntype_t type,
			    int start, int end) {
	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js,
				size_t len, jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *token;
	int start;

	start = parser->pos;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		switch (js[parser->pos]) {
#ifndef JSMN_STRICT
		/* In strict mode primitive must be followed by "," or "}" or "]" */
		case ':':
#endif
		case '\t' : case '\r' : case '\n' : case ' ' :
		case ','  : case ']'  : case '}' :
			goto found;
		}
		if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
			parser->pos = start;
			return JSMN_ERROR_INVAL;
		}
	}
#ifdef JSMN_STRICT
	/* In strict mode primitive must be followed by a comma/object/array */
	parser->pos = start;
	return JSMN_ERROR_PART;
#endif

found:
	if (tokens == NULL) {
		parser->pos--;
		return 0;
	}
	token = jsmn_alloc_token(parser, tokens, num_tokens);
	if (token == NULL) {
		parser->pos = start;
		return JSMN_ERROR_NOMEM;
	}
	jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
	token->parent = parser->toksuper;
#endif
	parser->pos--;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js,
			     size_t len, jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *token;

	int start = parser->pos;

	parser->pos++;

	/* Skip starting quote */
	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c = js[parser->pos];

		/* Quote: end of string */
		if (c == '\"') {
			if (tokens == NULL) {
				return 0;
			}
			token = jsmn_alloc_token(parser, tokens, num_tokens);
			if (token == NULL) {
				parser->pos = start;
				return JSMN_ERROR_NOMEM;
			}
			jsmn_fill_token(token, JSMN_STRING, start+1, parser->pos);
#ifdef JSMN_PARENT_LINKS
			token->parent = parser->toksuper;
#endif
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && parser->pos + 1 < len) {
			int i;
			parser->pos++;
			switch (js[parser->pos]) {
			/* Allowed escaped symbols */
			case '\"': case '/' : case '\\' : case 'b' :
			case 'f' : case 'r' : case 'n'  : case 't' :
				break;
				/* Allows escaped symbol \uXXXX */
			case 'u':
				parser->pos++;
				for(i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++) {
					/* If it isn't a hex character we have an error */
					if(!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
					     (js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
					     (js[parser->pos] >= 97 && js[parser->pos] <= 102))) { /* a-f */
						parser->pos = start;
						return JSMN_ERROR_INVAL;
					}
					parser->pos++;
				}
				parser->pos--;
				break;
				/* Unexpected symbol */
			default:
				parser->pos = start;
				return JSMN_ERROR_INVAL;
			}
		}
	}
	parser->pos = start;
	return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
	       jsmntok_t *tokens, unsigned int num_tokens) {
	int r;
	int i;
	jsmntok_t *token;
	int count = parser->toknext;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c;
		jsmntype_t type;

		c = js[parser->pos];
		switch (c) {
		case '{': case '[':
			count++;
			if (tokens == NULL) {
				break;
			}
			token = jsmn_alloc_token(parser, tokens, num_tokens);
			if (token == NULL)
				return JSMN_ERROR_NOMEM;
			if (parser->toksuper != -1) {
				tokens[parser->toksuper].size++;
#ifdef JSMN_PARENT_LINKS
				token->parent = parser->toksuper;
#endif
			}
			token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
			token->start = parser->pos;
			parser->toksuper = parser->toknext - 1;
			break;
		case '}': case ']':
			if (tokens == NULL)
				break;
			type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
			if (parser->toknext < 1) {
				return JSMN_ERROR_INVAL;
			}
			token = &tokens[parser->toknext - 1];
			for (;;) {
				if (token->start != -1 && token->end == -1) {
					if (token->type != type) {
						return JSMN_ERROR_INVAL;
					}
					token->end = parser->pos + 1;
					parser->toksuper = token->parent;
					break;
				}
				if (token->parent == -1) {
					if(token->type != type || parser->toksuper == -1) {
						return JSMN_ERROR_INVAL;
					}
					break;
				}
				token = &tokens[token->parent];
			}
#else
			for (i = parser->toknext - 1; i >= 0; i--) {
				token = &tokens[i];
				if (token->start != -1 && token->end == -1) {
					if (token->type != type) {
						return JSMN_ERROR_INVAL;
					}
					parser->toksuper = -1;
					token->end = parser->pos + 1;
					break;
				}
			}
			/* Error if unmatched closing bracket */
			if (i == -1) return JSMN_ERROR_INVAL;
			for (; i >= 0; i--) {
				token = &tokens[i];
				if (token->start != -1 && token->end == -1) {
					parser->toksuper = i;
					break;
				}
			}
#endif
			break;
		case '\"':
			r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
			if (r < 0) return r;
			count++;
			if (parser->toksuper != -1 && tokens != NULL)
				tokens[parser->toksuper].size++;
			break;
		case '\t' : case '\r' : case '\n' : case ' ':
			break;
		case ':':
			parser->toksuper = parser->toknext - 1;
			break;
		case ',':
			if (tokens != NULL && parser->toksuper != -1 &&
					tokens[parser->toksuper].type != JSMN_ARRAY &&
					tokens[parser->toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
				parser->toksuper = tokens[parser->toksuper].parent;
#else
				for (i = parser->toknext - 1; i >= 0; i--) {
					if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT) {
						if (tokens[i].start != -1 && tokens[i].end == -1) {
							parser->toksuper = i;
							break;
						}
					}
				}
#endif
			}
			break;
#ifdef JSMN_STRICT
			/* In strict mode primitives are: numbers and booleans */
		case '-': case '0': case '1' : case '2': case '3' : case '4':
		case '5': case '6': case '7' : case '8': case '9':
		case 't': case 'f': case 'n' :
			/* And they must not be keys of the object */
			if (tokens != NULL && parser->toksuper != -1) {
				jsmntok_t *t = &tokens[parser->toksuper];
				if (t->type == JSMN_OBJECT ||
						(t->type == JSMN_STRING && t->size != 0)) {
					return JSMN_ERROR_INVAL;
				}
			}
#else
			/* In non-strict mode every unquoted value is a primitive */
		default:
#endif
			r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
			if (r < 0) return r;
			count++;
			if (parser->toksuper != -1 && tokens != NULL)
				tokens[parser->toksuper].size++;
			break;

#ifdef JSMN_STRICT
			/* Unexpected char in strict mode */
		default:
			return JSMN_ERROR_INVAL;
#endif
		}
	}

	if (tokens != NULL) {
		for (i = parser->toknext - 1; i >= 0; i--) {
			/* Unmatched opened object or array */
			if (tokens[i].start != -1 && tokens[i].end == -1) {
				return JSMN_ERROR_PART;
			}
		}
	}

	return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser) {
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = -1;
}
/*
 * -- jsmn.c end --
 */

#endif /* #ifdef CGLTF_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif
