/*
   The MIT License (MIT)

   Copyright (c) 2016 - 2019 Syoyo Fujita and many contributors.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
   */
#ifndef TINOBJ_LOADER_C_H_
#define TINOBJ_LOADER_C_H_

/* @todo { Remove stddef dependency. size_t? } */
#include <stddef.h>

typedef struct {
  char *name;

  float ambient[3];
  float diffuse[3];
  float specular[3];
  float transmittance[3];
  float emission[3];
  float shininess;
  float ior;      /* index of refraction */
  float dissolve; /* 1 == opaque; 0 == fully transparent */
  /* illumination model (see http://www.fileformat.info/format/material/) */
  int illum;

  int pad0;

  char *ambient_texname;            /* map_Ka */
  char *diffuse_texname;            /* map_Kd */
  char *specular_texname;           /* map_Ks */
  char *specular_highlight_texname; /* map_Ns */
  char *bump_texname;               /* map_bump, bump */
  char *displacement_texname;       /* disp */
  char *alpha_texname;              /* map_d */
} tinyobj_material_t;

typedef struct {
  char *name; /* group name or object name. */
  unsigned int face_offset;
  unsigned int length;
} tinyobj_shape_t;

typedef struct { int v_idx, vt_idx, vn_idx; } tinyobj_vertex_index_t;

typedef struct {
  unsigned int num_vertices;
  unsigned int num_normals;
  unsigned int num_texcoords;
  unsigned int num_faces;
  unsigned int num_face_num_verts;

  int pad0;

  float *vertices;
  float *normals;
  float *texcoords;
  tinyobj_vertex_index_t *faces;
  int *face_num_verts;
  int *material_ids;
} tinyobj_attrib_t;


#define TINYOBJ_FLAG_TRIANGULATE (1 << 0)

#define TINYOBJ_INVALID_INDEX (0x80000000)

#define TINYOBJ_SUCCESS (0)
#define TINYOBJ_ERROR_EMPTY (-1)
#define TINYOBJ_ERROR_INVALID_PARAMETER (-2)
#define TINYOBJ_ERROR_FILE_OPERATION (-3)

/* Parse wavefront .obj(.obj string data is expanded to linear char array `buf')
 * flags are combination of TINYOBJ_FLAG_***
 * Returns TINYOBJ_SUCCESS if things goes well.
 * Returns TINYOBJ_ERR_*** when there is an error.
 */
extern int tinyobj_parse_obj(tinyobj_attrib_t *attrib, tinyobj_shape_t **shapes,
                             size_t *num_shapes, tinyobj_material_t **materials,
                             size_t *num_materials, const char *buf, size_t len,
                             unsigned int flags);
extern int tinyobj_parse_mtl_file(tinyobj_material_t **materials_out,
                                  size_t *num_materials_out,
                                  const char *filename);

extern void tinyobj_attrib_init(tinyobj_attrib_t *attrib);
extern void tinyobj_attrib_free(tinyobj_attrib_t *attrib);
extern void tinyobj_shapes_free(tinyobj_shape_t *shapes, size_t num_shapes);
extern void tinyobj_materials_free(tinyobj_material_t *materials,
                                   size_t num_materials);

#ifdef TINYOBJ_LOADER_C_IMPLEMENTATION
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#if defined(TINYOBJ_MALLOC) && defined(TINYOBJ_REALLOC) && defined(TINYOBJ_CALLOC) && defined(TINYOBJ_FREE)
/* ok */
#elif !defined(TINYOBJ_MALLOC) && !defined(TINYOBJ_REALLOC) && !defined(TINYOBJ_CALLOC) && !defined(TINYOBJ_FREE)
/* ok */
#else
#error "Must define all or none of TINYOBJ_MALLOC, TINYOBJ_REALLOC, TINYOBJ_CALLOC and TINYOBJ_FREE."
#endif

#ifndef TINYOBJ_MALLOC
#include <stdlib.h>
#define TINYOBJ_MALLOC malloc
#define TINYOBJ_REALLOC realloc
#define TINYOBJ_CALLOC calloc
#define TINYOBJ_FREE free
#endif

#define TINYOBJ_MAX_FACES_PER_F_LINE (16)

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_DIGIT(x) ((unsigned int)((x) - '0') < (unsigned int)(10))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

static void skip_space(const char **token) {
  while ((*token)[0] == ' ' || (*token)[0] == '\t') {
    (*token)++;
  }
}

static void skip_space_and_cr(const char **token) {
  while ((*token)[0] == ' ' || (*token)[0] == '\t' || (*token)[0] == '\r') {
    (*token)++;
  }
}

static int until_space(const char *token) {
  const char *p = token;
  while (p[0] != '\0' && p[0] != ' ' && p[0] != '\t' && p[0] != '\r') {
    p++;
  }

  return (int)(p - token);
}

static size_t length_until_newline(const char *token, size_t n) {
  size_t len = 0;

  /* Assume token[n-1] = '\0' */
  for (len = 0; len < n - 1; len++) {
    if (token[len] == '\n') {
      break;
    }
    if ((token[len] == '\r') && ((len < (n - 2)) && (token[len + 1] != '\n'))) {
      break;
    }
  }

  return len;
}

static size_t length_until_line_feed(const char *token, size_t n) {
  size_t len = 0;

  /* Assume token[n-1] = '\0' */
  for (len = 0; len < n; len++) {
    if ((token[len] == '\n') || (token[len] == '\r')) {
      break;
    }
  }

  return len;
}

/* http://stackoverflow.com/questions/5710091/how-does-atoi-function-in-c-work
*/
static int my_atoi(const char *c) {
  int value = 0;
  int sign = 1;
  if (*c == '+' || *c == '-') {
    if (*c == '-') sign = -1;
    c++;
  }
  while (((*c) >= '0') && ((*c) <= '9')) { /* isdigit(*c) */
    value *= 10;
    value += (int)(*c - '0');
    c++;
  }
  return value * sign;
}

/* Make index zero-base, and also support relative index. */
static int fixIndex(int idx, size_t n) {
  if (idx > 0) return idx - 1;
  if (idx == 0) return 0;
  return (int)n + idx; /* negative value = relative */
}

/* Parse raw triples: i, i/j/k, i//k, i/j */
static tinyobj_vertex_index_t parseRawTriple(const char **token) {
  tinyobj_vertex_index_t vi;
  /* 0x80000000 = -2147483648 = invalid */
  vi.v_idx = (int)(0x80000000);
  vi.vn_idx = (int)(0x80000000);
  vi.vt_idx = (int)(0x80000000);

  vi.v_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
         (*token)[0] != '\t' && (*token)[0] != '\r') {
    (*token)++;
  }
  if ((*token)[0] != '/') {
    return vi;
  }
  (*token)++;

  /* i//k */
  if ((*token)[0] == '/') {
    (*token)++;
    vi.vn_idx = my_atoi((*token));
    while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
           (*token)[0] != '\t' && (*token)[0] != '\r') {
      (*token)++;
    }
    return vi;
  }

  /* i/j/k or i/j */
  vi.vt_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
         (*token)[0] != '\t' && (*token)[0] != '\r') {
    (*token)++;
  }
  if ((*token)[0] != '/') {
    return vi;
  }

  /* i/j/k */
  (*token)++; /* skip '/' */
  vi.vn_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
         (*token)[0] != '\t' && (*token)[0] != '\r') {
    (*token)++;
  }
  return vi;
}

static int parseInt(const char **token) {
  int i = 0;
  skip_space(token);
  i = my_atoi((*token));
  (*token) += until_space((*token));
  return i;
}

/*
 * Tries to parse a floating point number located at s.
 *
 * s_end should be a location in the string where reading should absolutely
 * stop. For example at the end of the string, to prevent buffer overflows.
 *
 * Parses the following EBNF grammar:
 *   sign    = "+" | "-" ;
 *   END     = ? anything not in digit ?
 *   digit   = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
 *   integer = [sign] , digit , {digit} ;
 *   decimal = integer , ["." , integer] ;
 *   float   = ( decimal , END ) | ( decimal , ("E" | "e") , integer , END ) ;
 *
 *  Valid strings are for example:
 *   -0  +3.1417e+2  -0.0E-3  1.0324  -1.41   11e2
 *
 * If the parsing is a success, result is set to the parsed value and true
 * is returned.
 *
 * The function is greedy and will parse until any of the following happens:
 *  - a non-conforming character is encountered.
 *  - s_end is reached.
 *
 * The following situations triggers a failure:
 *  - s >= s_end.
 *  - parse failure.
 */
static int tryParseDouble(const char *s, const char *s_end, double *result) {
  double mantissa = 0.0;
  /* This exponent is base 2 rather than 10.
   * However the exponent we parse is supposed to be one of ten,
   * thus we must take care to convert the exponent/and or the
   * mantissa to a * 2^E, where a is the mantissa and E is the
   * exponent.
   * To get the final double we will use ldexp, it requires the
   * exponent to be in base 2.
   */
  int exponent = 0;

  /* NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
   * TO JUMP OVER DEFINITIONS.
   */
  char sign = '+';
  char exp_sign = '+';
  char const *curr = s;

  /* How many characters were read in a loop. */
  int read = 0;
  /* Tells whether a loop terminated due to reaching s_end. */
  int end_not_reached = 0;

  /*
     BEGIN PARSING.
     */

  if (s >= s_end) {
    return 0; /* fail */
  }

  /* Find out what sign we've got. */
  if (*curr == '+' || *curr == '-') {
    sign = *curr;
    curr++;
  } else if (IS_DIGIT(*curr)) { /* Pass through. */
  } else {
    goto fail;
  }

  /* Read the integer part. */
  end_not_reached = (curr != s_end);
  while (end_not_reached && IS_DIGIT(*curr)) {
    mantissa *= 10;
    mantissa += (int)(*curr - 0x30);
    curr++;
    read++;
    end_not_reached = (curr != s_end);
  }

  /* We must make sure we actually got something. */
  if (read == 0) goto fail;
  /* We allow numbers of form "#", "###" etc. */
  if (!end_not_reached) goto assemble;

  /* Read the decimal part. */
  if (*curr == '.') {
    curr++;
    read = 1;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      /* pow(10.0, -read) */
      double frac_value = 1.0;
      int f;
      for (f = 0; f < read; f++) {
        frac_value *= 0.1;
      }
      mantissa += (int)(*curr - 0x30) * frac_value;
      read++;
      curr++;
      end_not_reached = (curr != s_end);
    }
  } else if (*curr == 'e' || *curr == 'E') {
  } else {
    goto assemble;
  }

  if (!end_not_reached) goto assemble;

  /* Read the exponent part. */
  if (*curr == 'e' || *curr == 'E') {
    curr++;
    /* Figure out if a sign is present and if it is. */
    end_not_reached = (curr != s_end);
    if (end_not_reached && (*curr == '+' || *curr == '-')) {
      exp_sign = *curr;
      curr++;
    } else if (IS_DIGIT(*curr)) { /* Pass through. */
    } else {
      /* Empty E is not allowed. */
      goto fail;
    }

    read = 0;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      exponent *= 10;
      exponent += (int)(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    if (read == 0) goto fail;
  }

assemble :

  {
    double a = 1.0; /* = pow(5.0, exponent); */
    double b  = 1.0; /* = 2.0^exponent */
    int i;
    for (i = 0; i < exponent; i++) {
      a = a * 5.0;
    }

    for (i = 0; i < exponent; i++) {
      b = b * 2.0;
    }

    if (exp_sign == '-') {
      a = 1.0 / a;
      b = 1.0 / b;
    }

    *result =
      /* (sign == '+' ? 1 : -1) * ldexp(mantissa * pow(5.0, exponent),
         exponent); */
      (sign == '+' ? 1 : -1) * (mantissa * a * b);
  }

  return 1;
fail:
  return 0;
}

static float parseFloat(const char **token) {
  const char *end;
  double val = 0.0;
  float f = 0.0f;
  skip_space(token);
  end = (*token) + until_space((*token));
  val = 0.0;
  tryParseDouble((*token), end, &val);
  f = (float)(val);
  (*token) = end;
  return f;
}

static void parseFloat2(float *x, float *y, const char **token) {
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
}

static void parseFloat3(float *x, float *y, float *z, const char **token) {
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
  (*z) = parseFloat(token);
}

static char *my_strdup(const char *s, size_t max_length) {
  char *d;
  size_t len;

  if (s == NULL) return NULL;

  /* Do not consider CRLF line ending(#19) */
  len = length_until_line_feed(s, max_length);
  /* len = strlen(s); */

  /* trim line ending and append '\0' */
  d = (char *)TINYOBJ_MALLOC(len + 1); /* + '\0' */
  memcpy(d, s, (size_t)(len));
  d[len] = '\0';

  return d;
}

static char *my_strndup(const char *s, size_t len) {
  char *d;
  size_t slen;

  if (s == NULL) return NULL;
  if (len == 0) return NULL;

  d = (char *)TINYOBJ_MALLOC(len + 1); /* + '\0' */
  slen = strlen(s);
  if (slen < len) {
    memcpy(d, s, slen);
    d[slen] = '\0';
  } else {
    memcpy(d, s, len);
    d[len] = '\0';
  }

  return d;
}

char *dynamic_fgets(char **buf, size_t *size, FILE *file) {
  char *offset;
  char *ret;
  size_t old_size;

  if (!(ret = fgets(*buf, (int)*size, file))) {
    return ret;
  }

  if (NULL != strchr(*buf, '\n')) {
    return ret;
  }

  do {
    old_size = *size;
    *size *= 2;
    *buf = (char*)TINYOBJ_REALLOC(*buf, *size);
    offset = &((*buf)[old_size - 1]);

    ret = fgets(offset, (int)(old_size + 1), file);
  } while(ret && (NULL == strchr(*buf, '\n')));

  return ret;
}

static void initMaterial(tinyobj_material_t *material) {
  int i;
  material->name = NULL;
  material->ambient_texname = NULL;
  material->diffuse_texname = NULL;
  material->specular_texname = NULL;
  material->specular_highlight_texname = NULL;
  material->bump_texname = NULL;
  material->displacement_texname = NULL;
  material->alpha_texname = NULL;
  for (i = 0; i < 3; i++) {
    material->ambient[i] = 0.f;
    material->diffuse[i] = 0.f;
    material->specular[i] = 0.f;
    material->transmittance[i] = 0.f;
    material->emission[i] = 0.f;
  }
  material->illum = 0;
  material->dissolve = 1.f;
  material->shininess = 1.f;
  material->ior = 1.f;
}

/* Implementation of string to int hashtable */

#define HASH_TABLE_ERROR 1 
#define HASH_TABLE_SUCCESS 0

#define HASH_TABLE_DEFAULT_SIZE 10

typedef struct hash_table_entry_t
{
  unsigned long hash;
  int filled;
  int pad0;
  long value;

  struct hash_table_entry_t* next;
} hash_table_entry_t;

typedef struct
{
  unsigned long* hashes;
  hash_table_entry_t* entries;
  size_t capacity;
  size_t n;
} hash_table_t;

static unsigned long hash_djb2(const unsigned char* str)
{
  unsigned long hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + (unsigned long)(c);
  }

  return hash;
}

static void create_hash_table(size_t start_capacity, hash_table_t* hash_table)
{
  if (start_capacity < 1)
    start_capacity = HASH_TABLE_DEFAULT_SIZE;
  hash_table->hashes = (unsigned long*) TINYOBJ_MALLOC(start_capacity * sizeof(unsigned long));
  hash_table->entries = (hash_table_entry_t*) TINYOBJ_CALLOC(start_capacity, sizeof(hash_table_entry_t));
  hash_table->capacity = start_capacity;
  hash_table->n = 0;
}

static void destroy_hash_table(hash_table_t* hash_table)
{
  TINYOBJ_FREE(hash_table->entries);
  TINYOBJ_FREE(hash_table->hashes);
}

/* Insert with quadratic probing */
static int hash_table_insert_value(unsigned long hash, long value, hash_table_t* hash_table)
{
  /* Insert value */
  size_t start_index = hash % hash_table->capacity;
  size_t index = start_index;
  hash_table_entry_t* start_entry = hash_table->entries + start_index;
  size_t i;
  hash_table_entry_t* entry;

  for (i = 1; hash_table->entries[index].filled; i++)
  {
    if (i >= hash_table->capacity)
      return HASH_TABLE_ERROR;
    index = (start_index + (i * i)) % hash_table->capacity; 
  }

  entry = hash_table->entries + index;
  entry->hash = hash;
  entry->filled = 1;
  entry->value = value;

  if (index != start_index) {
    /* This is a new entry, but not the start entry, hence we need to add a next pointer to our entry */
    entry->next = start_entry->next;
    start_entry->next = entry;
  }

  return HASH_TABLE_SUCCESS;
}

static int hash_table_insert(unsigned long hash, long value, hash_table_t* hash_table)
{
  int ret = hash_table_insert_value(hash, value, hash_table);
  if (ret == HASH_TABLE_SUCCESS)
  {
    hash_table->hashes[hash_table->n] = hash;
    hash_table->n++;
  }
  return ret;
}

static hash_table_entry_t* hash_table_find(unsigned long hash, hash_table_t* hash_table)
{
  hash_table_entry_t* entry = hash_table->entries + (hash % hash_table->capacity);
  while (entry)
  {
    if (entry->hash == hash && entry->filled)
    {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

static void hash_table_maybe_grow(size_t new_n, hash_table_t* hash_table)
{
  size_t new_capacity;
  hash_table_t new_hash_table;
  size_t i;

  if (new_n <= hash_table->capacity) {
    return;
  }
  new_capacity = 2 * ((2 * hash_table->capacity) > new_n ? hash_table->capacity : new_n);
  /* Create a new hash table. We're not calling create_hash_table because we want to realloc the hash array */
  new_hash_table.hashes = hash_table->hashes = (unsigned long*) TINYOBJ_REALLOC((void*) hash_table->hashes, sizeof(unsigned long) * new_capacity);
  new_hash_table.entries = (hash_table_entry_t*) TINYOBJ_CALLOC(new_capacity, sizeof(hash_table_entry_t));
  new_hash_table.capacity = new_capacity;
  new_hash_table.n = hash_table->n;

  /* Rehash */
  for (i = 0; i < hash_table->capacity; i++)
  {
    hash_table_entry_t* entry = hash_table_find(hash_table->hashes[i], hash_table);
    hash_table_insert_value(hash_table->hashes[i], entry->value, &new_hash_table);
  }

  TINYOBJ_FREE(hash_table->entries);
  (*hash_table) = new_hash_table;
}

static int hash_table_exists(const char* name, hash_table_t* hash_table)
{
  return hash_table_find(hash_djb2((const unsigned char*)name), hash_table) != NULL;
}

static void hash_table_set(const char* name, size_t val, hash_table_t* hash_table)
{
  /* Hash name */
  unsigned long hash = hash_djb2((const unsigned char *)name);

  hash_table_entry_t* entry = hash_table_find(hash, hash_table);
  if (entry)
  {
    entry->value = (long)val;
    return;
  }

  /* Expand if necessary
   * Grow until the element has been added
   */
  do
  {
    hash_table_maybe_grow(hash_table->n + 1, hash_table);
  }
  while (hash_table_insert(hash, (long)val, hash_table) != HASH_TABLE_SUCCESS);
}

static long hash_table_get(const char* name, hash_table_t* hash_table)
{
  hash_table_entry_t* ret = hash_table_find(hash_djb2((const unsigned char*)(name)), hash_table);
  return ret->value;
}

static tinyobj_material_t *tinyobj_material_add(tinyobj_material_t *prev,
                                                size_t num_materials,
                                                tinyobj_material_t *new_mat) {
  tinyobj_material_t *dst;
  dst = (tinyobj_material_t *)TINYOBJ_REALLOC(
                                      prev, sizeof(tinyobj_material_t) * (num_materials + 1));

  dst[num_materials] = (*new_mat); /* Just copy pointer for char* members */
  return dst;
}

static int tinyobj_parse_and_index_mtl_file(tinyobj_material_t **materials_out,
                                            size_t *num_materials_out,
                                            const char *filename,
                                            hash_table_t* material_table) {
  tinyobj_material_t material;
  size_t buffer_size = 128;
  char *linebuf;
  FILE *fp;
  size_t num_materials = 0;
  tinyobj_material_t *materials = NULL;
  int has_previous_material = 0;
  const char *line_end = NULL;

  if (materials_out == NULL) {
    return TINYOBJ_ERROR_INVALID_PARAMETER;
  }

  if (num_materials_out == NULL) {
    return TINYOBJ_ERROR_INVALID_PARAMETER;
  }

  (*materials_out) = NULL;
  (*num_materials_out) = 0;

  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "TINYOBJ: Error reading file '%s': %s (%d)\n", filename, strerror(errno), errno);
    return TINYOBJ_ERROR_FILE_OPERATION;
  }

  /* Create a default material */
  initMaterial(&material);

  linebuf = (char*)TINYOBJ_MALLOC(buffer_size);
  while (NULL != dynamic_fgets(&linebuf, &buffer_size, fp)) {
    const char *token = linebuf;

    line_end = token + strlen(token);

    /* Skip leading space. */
    token += strspn(token, " \t");

    assert(token);
    if (token[0] == '\0') continue; /* empty line */

    if (token[0] == '#') continue; /* comment line */

    /* new mtl */
    if ((0 == strncmp(token, "newmtl", 6)) && IS_SPACE((token[6]))) {
      char namebuf[4096];

      /* flush previous material. */
      if (has_previous_material) {
        materials = tinyobj_material_add(materials, num_materials, &material);
        num_materials++;
      } else {
        has_previous_material = 1;
      }

      /* initial temporary material */
      initMaterial(&material);

      /* set new mtl name */
      token += 7;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif
      material.name = my_strdup(namebuf, (size_t) (line_end - token));

      /* Add material to material table */
      if (material_table)
        hash_table_set(material.name, num_materials, material_table);

      continue;
    }

    /* ambient */
    if (token[0] == 'K' && token[1] == 'a' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.ambient[0] = r;
      material.ambient[1] = g;
      material.ambient[2] = b;
      continue;
    }

    /* diffuse */
    if (token[0] == 'K' && token[1] == 'd' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.diffuse[0] = r;
      material.diffuse[1] = g;
      material.diffuse[2] = b;
      continue;
    }

    /* specular */
    if (token[0] == 'K' && token[1] == 's' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.specular[0] = r;
      material.specular[1] = g;
      material.specular[2] = b;
      continue;
    }

    /* transmittance */
    if (token[0] == 'K' && token[1] == 't' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.transmittance[0] = r;
      material.transmittance[1] = g;
      material.transmittance[2] = b;
      continue;
    }

    /* ior(index of refraction) */
    if (token[0] == 'N' && token[1] == 'i' && IS_SPACE((token[2]))) {
      token += 2;
      material.ior = parseFloat(&token);
      continue;
    }

    /* emission */
    if (token[0] == 'K' && token[1] == 'e' && IS_SPACE(token[2])) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.emission[0] = r;
      material.emission[1] = g;
      material.emission[2] = b;
      continue;
    }

    /* shininess */
    if (token[0] == 'N' && token[1] == 's' && IS_SPACE(token[2])) {
      token += 2;
      material.shininess = parseFloat(&token);
      continue;
    }

    /* illum model */
    if (0 == strncmp(token, "illum", 5) && IS_SPACE(token[5])) {
      token += 6;
      material.illum = parseInt(&token);
      continue;
    }

    /* dissolve */
    if ((token[0] == 'd' && IS_SPACE(token[1]))) {
      token += 1;
      material.dissolve = parseFloat(&token);
      continue;
    }
    if (token[0] == 'T' && token[1] == 'r' && IS_SPACE(token[2])) {
      token += 2;
      /* Invert value of Tr(assume Tr is in range [0, 1]) */
      material.dissolve = 1.0f - parseFloat(&token);
      continue;
    }

    /* ambient texture */
    if ((0 == strncmp(token, "map_Ka", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.ambient_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* diffuse texture */
    if ((0 == strncmp(token, "map_Kd", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.diffuse_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* specular texture */
    if ((0 == strncmp(token, "map_Ks", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.specular_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* specular highlight texture */
    if ((0 == strncmp(token, "map_Ns", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.specular_highlight_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* bump texture */
    if ((0 == strncmp(token, "map_bump", 8)) && IS_SPACE(token[8])) {
      token += 9;
      material.bump_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* alpha texture */
    if ((0 == strncmp(token, "map_d", 5)) && IS_SPACE(token[5])) {
      token += 6;
      material.alpha_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* bump texture */
    if ((0 == strncmp(token, "bump", 4)) && IS_SPACE(token[4])) {
      token += 5;
      material.bump_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* displacement texture */
    if ((0 == strncmp(token, "disp", 4)) && IS_SPACE(token[4])) {
      token += 5;
      material.displacement_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* @todo { unknown parameter } */
  }

  if (material.name) {
    /* Flush last material element */
    materials = tinyobj_material_add(materials, num_materials, &material);
    num_materials++;
  }

  (*num_materials_out) = num_materials;
  (*materials_out) = materials;

  if (linebuf) {
    TINYOBJ_FREE(linebuf);
  }

  return TINYOBJ_SUCCESS;
}

int tinyobj_parse_mtl_file(tinyobj_material_t **materials_out,
                           size_t *num_materials_out,
                           const char *filename) {
  return tinyobj_parse_and_index_mtl_file(materials_out, num_materials_out, filename, NULL);
} 


typedef enum {
  COMMAND_EMPTY,
  COMMAND_V,
  COMMAND_VN,
  COMMAND_VT,
  COMMAND_F,
  COMMAND_G,
  COMMAND_O,
  COMMAND_USEMTL,
  COMMAND_MTLLIB

} CommandType;

typedef struct {
  float vx, vy, vz;
  float nx, ny, nz;
  float tx, ty;

  /* @todo { Use dynamic array } */
  tinyobj_vertex_index_t f[TINYOBJ_MAX_FACES_PER_F_LINE];
  size_t num_f;

  int f_num_verts[TINYOBJ_MAX_FACES_PER_F_LINE];
  size_t num_f_num_verts;

  const char *group_name;
  unsigned int group_name_len;
  int pad0;

  const char *object_name;
  unsigned int object_name_len;
  int pad1;

  const char *material_name;
  unsigned int material_name_len;
  int pad2;

  const char *mtllib_name;
  unsigned int mtllib_name_len;

  CommandType type;
} Command;

static int parseLine(Command *command, const char *p, size_t p_len,
                     int triangulate) {
  char linebuf[4096];
  const char *token;
  assert(p_len < 4095);

  memcpy(linebuf, p, p_len);
  linebuf[p_len] = '\0';

  token = linebuf;

  command->type = COMMAND_EMPTY;

  /* Skip leading space. */
  skip_space(&token);

  assert(token);
  if (token[0] == '\0') { /* empty line */
    return 0;
  }

  if (token[0] == '#') { /* comment line */
    return 0;
  }

  /* vertex */
  if (token[0] == 'v' && IS_SPACE((token[1]))) {
    float x, y, z;
    token += 2;
    parseFloat3(&x, &y, &z, &token);
    command->vx = x;
    command->vy = y;
    command->vz = z;
    command->type = COMMAND_V;
    return 1;
  }

  /* normal */
  if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
    float x, y, z;
    token += 3;
    parseFloat3(&x, &y, &z, &token);
    command->nx = x;
    command->ny = y;
    command->nz = z;
    command->type = COMMAND_VN;
    return 1;
  }

  /* texcoord */
  if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
    float x, y;
    token += 3;
    parseFloat2(&x, &y, &token);
    command->tx = x;
    command->ty = y;
    command->type = COMMAND_VT;
    return 1;
  }

  /* face */
  if (token[0] == 'f' && IS_SPACE((token[1]))) {
    size_t num_f = 0;

    tinyobj_vertex_index_t f[TINYOBJ_MAX_FACES_PER_F_LINE];
    token += 2;
    skip_space(&token);

    while (!IS_NEW_LINE(token[0])) {
      tinyobj_vertex_index_t vi = parseRawTriple(&token);
      skip_space_and_cr(&token);

      f[num_f] = vi;
      num_f++;
    }

    command->type = COMMAND_F;

    if (triangulate) {
      size_t k;
      size_t n = 0;

      tinyobj_vertex_index_t i0 = f[0];
      tinyobj_vertex_index_t i1;
      tinyobj_vertex_index_t i2 = f[1];

      assert(3 * num_f < TINYOBJ_MAX_FACES_PER_F_LINE);

      for (k = 2; k < num_f; k++) {
        i1 = i2;
        i2 = f[k];
        command->f[3 * n + 0] = i0;
        command->f[3 * n + 1] = i1;
        command->f[3 * n + 2] = i2;

        command->f_num_verts[n] = 3;
        n++;
      }
      command->num_f = 3 * n;
      command->num_f_num_verts = n;

    } else {
      size_t k = 0;
      assert(num_f < TINYOBJ_MAX_FACES_PER_F_LINE);
      for (k = 0; k < num_f; k++) {
        command->f[k] = f[k];
      }

      command->num_f = num_f;
      command->f_num_verts[0] = (int)num_f;
      command->num_f_num_verts = 1;
    }

    return 1;
  }

  /* use mtl */
  if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
    token += 7;

    skip_space(&token);
    command->material_name = p + (token - linebuf);
    command->material_name_len = (unsigned int)length_until_newline(
                                                                    token, (p_len - (size_t)(token - linebuf)) + 1);
    command->type = COMMAND_USEMTL;

    return 1;
  }

  /* load mtl */
  if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
    /* By specification, `mtllib` should be appear only once in .obj */
    token += 7;

    skip_space(&token);
    command->mtllib_name = p + (token - linebuf);
    command->mtllib_name_len = (unsigned int)length_until_newline(
                                                                  token, p_len - (size_t)(token - linebuf)) +
      1;
    command->type = COMMAND_MTLLIB;

    return 1;
  }

  /* group name */
  if (token[0] == 'g' && IS_SPACE((token[1]))) {
    /* @todo { multiple group name. } */
    token += 2;

    command->group_name = p + (token - linebuf);
    command->group_name_len = (unsigned int)length_until_newline(
                                                                 token, p_len - (size_t)(token - linebuf)) +
      1;
    command->type = COMMAND_G;

    return 1;
  }

  /* object name */
  if (token[0] == 'o' && IS_SPACE((token[1]))) {
    /* @todo { multiple object name? } */
    token += 2;

    command->object_name = p + (token - linebuf);
    command->object_name_len = (unsigned int)length_until_newline(
                                                                  token, p_len - (size_t)(token - linebuf)) +
      1;
    command->type = COMMAND_O;

    return 1;
  }

  return 0;
}

typedef struct {
  size_t pos;
  size_t len;
} LineInfo;

static int is_line_ending(const char *p, size_t i, size_t end_i) {
  if (p[i] == '\0') return 1;
  if (p[i] == '\n') return 1; /* this includes \r\n */
  if (p[i] == '\r') {
    if (((i + 1) < end_i) && (p[i + 1] != '\n')) { /* detect only \r case */
      return 1;
    }
  }
  return 0;
}

int tinyobj_parse_obj(tinyobj_attrib_t *attrib, tinyobj_shape_t **shapes,
                      size_t *num_shapes, tinyobj_material_t **materials_out,
                      size_t *num_materials_out, const char *buf, size_t len,
                      unsigned int flags) {
  LineInfo *line_infos = NULL;
  Command *commands = NULL;
  size_t num_lines = 0;

  size_t num_v = 0;
  size_t num_vn = 0;
  size_t num_vt = 0;
  size_t num_f = 0;
  size_t num_faces = 0;

  int mtllib_line_index = -1;

  tinyobj_material_t *materials = NULL;
  size_t num_materials = 0;

  hash_table_t material_table;

  if (len < 1) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (attrib == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (shapes == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (num_shapes == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (buf == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (materials_out == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (num_materials_out == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;

  tinyobj_attrib_init(attrib);
   /* 1. Find '\n' and create line data. */
  {
    size_t i;
    size_t end_idx = len;
    size_t prev_pos = 0;
    size_t line_no = 0;
    size_t last_line_ending = 0;

    /* Count # of lines. */
    for (i = 0; i < end_idx; i++) {
      if (is_line_ending(buf, i, end_idx)) {
        num_lines++;
        last_line_ending = i;
      }
    }
    /* The last char from the input may not be a line
     * ending character so add an extra line if there
     * are more characters after the last line ending
     * that was found. */
    if (end_idx - last_line_ending > 0) {
        num_lines++;
    }

    if (num_lines == 0) return TINYOBJ_ERROR_EMPTY;

    line_infos = (LineInfo *)TINYOBJ_MALLOC(sizeof(LineInfo) * num_lines);

    /* Fill line infos. */
    for (i = 0; i < end_idx; i++) {
      if (is_line_ending(buf, i, end_idx)) {
        line_infos[line_no].pos = prev_pos;
        line_infos[line_no].len = i - prev_pos;
        prev_pos = i + 1;
        line_no++;
      }
    }
    if (end_idx - last_line_ending > 0) {
      line_infos[line_no].pos = prev_pos;
      line_infos[line_no].len = end_idx - 1 - last_line_ending;
    }
  }

  commands = (Command *)TINYOBJ_MALLOC(sizeof(Command) * num_lines); 

  create_hash_table(HASH_TABLE_DEFAULT_SIZE, &material_table);

  /* 2. parse each line */
  {
    size_t i = 0;
    for (i = 0; i < num_lines; i++) {
      int ret = parseLine(&commands[i], &buf[line_infos[i].pos],
                          line_infos[i].len, flags & TINYOBJ_FLAG_TRIANGULATE);
      if (ret) {
        if (commands[i].type == COMMAND_V) {
          num_v++;
        } else if (commands[i].type == COMMAND_VN) {
          num_vn++;
        } else if (commands[i].type == COMMAND_VT) {
          num_vt++;
        } else if (commands[i].type == COMMAND_F) {
          num_f += commands[i].num_f;
          num_faces += commands[i].num_f_num_verts;
        }

        if (commands[i].type == COMMAND_MTLLIB) {
          mtllib_line_index = (int)i;
        }
      }
    }
  }

  /* line_infos are not used anymore. Release memory. */
  if (line_infos) {
    TINYOBJ_FREE(line_infos);
  }

  /* Load material(if exits) */
  if (mtllib_line_index >= 0 && commands[mtllib_line_index].mtllib_name &&
      commands[mtllib_line_index].mtllib_name_len > 0) {
    char *filename = my_strndup(commands[mtllib_line_index].mtllib_name,
                                commands[mtllib_line_index].mtllib_name_len);

    int ret = tinyobj_parse_and_index_mtl_file(&materials, &num_materials, filename, &material_table);

    if (ret != TINYOBJ_SUCCESS) {
      /* warning. */
      fprintf(stderr, "TINYOBJ: Failed to parse material file '%s': %d\n", filename, ret);
    }

    TINYOBJ_FREE(filename);

  }

  /* Construct attributes */

  {
    size_t v_count = 0;
    size_t n_count = 0;
    size_t t_count = 0;
    size_t f_count = 0;
    size_t face_count = 0;
    int material_id = -1; /* -1 = default unknown material. */
    size_t i = 0;

    attrib->vertices = (float *)TINYOBJ_MALLOC(sizeof(float) * num_v * 3);
    attrib->num_vertices = (unsigned int)num_v;
    attrib->normals = (float *)TINYOBJ_MALLOC(sizeof(float) * num_vn * 3);
    attrib->num_normals = (unsigned int)num_vn;
    attrib->texcoords = (float *)TINYOBJ_MALLOC(sizeof(float) * num_vt * 2);
    attrib->num_texcoords = (unsigned int)num_vt;
    attrib->faces = (tinyobj_vertex_index_t *)TINYOBJ_MALLOC(
                                                     sizeof(tinyobj_vertex_index_t) * num_f);
    attrib->num_faces = (unsigned int)num_f;
    attrib->face_num_verts = (int *)TINYOBJ_MALLOC(sizeof(int) * num_faces);
    attrib->material_ids = (int *)TINYOBJ_MALLOC(sizeof(int) * num_faces);
    attrib->num_face_num_verts = (unsigned int)num_faces;

    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_EMPTY) {
        continue;
      } else if (commands[i].type == COMMAND_USEMTL) {
        /* @todo
           if (commands[t][i].material_name &&
           commands[t][i].material_name_len > 0) {
           std::string material_name(commands[t][i].material_name,
           commands[t][i].material_name_len);

           if (material_map.find(material_name) != material_map.end()) {
           material_id = material_map[material_name];
           } else {
        // Assign invalid material ID
        material_id = -1;
        }
        }
        */
        if (commands[i].material_name &&
           commands[i].material_name_len >0) 
        {
          /* Create a null terminated string */
          char* material_name_null_term = (char*) TINYOBJ_MALLOC(commands[i].material_name_len + 1);
          memcpy((void*) material_name_null_term, (const void*) commands[i].material_name, commands[i].material_name_len);
          material_name_null_term[commands[i].material_name_len - 1] = 0;

          if (hash_table_exists(material_name_null_term, &material_table))
            material_id = (int)hash_table_get(material_name_null_term, &material_table);
          else
            material_id = -1;

          TINYOBJ_FREE(material_name_null_term);
        }
      } else if (commands[i].type == COMMAND_V) {
        attrib->vertices[3 * v_count + 0] = commands[i].vx;
        attrib->vertices[3 * v_count + 1] = commands[i].vy;
        attrib->vertices[3 * v_count + 2] = commands[i].vz;
        v_count++;
      } else if (commands[i].type == COMMAND_VN) {
        attrib->normals[3 * n_count + 0] = commands[i].nx;
        attrib->normals[3 * n_count + 1] = commands[i].ny;
        attrib->normals[3 * n_count + 2] = commands[i].nz;
        n_count++;
      } else if (commands[i].type == COMMAND_VT) {
        attrib->texcoords[2 * t_count + 0] = commands[i].tx;
        attrib->texcoords[2 * t_count + 1] = commands[i].ty;
        t_count++;
      } else if (commands[i].type == COMMAND_F) {
        size_t k = 0;
        for (k = 0; k < commands[i].num_f; k++) {
          tinyobj_vertex_index_t vi = commands[i].f[k];
          int v_idx = fixIndex(vi.v_idx, v_count);
          int vn_idx = fixIndex(vi.vn_idx, n_count);
          int vt_idx = fixIndex(vi.vt_idx, t_count);
          attrib->faces[f_count + k].v_idx = v_idx;
          attrib->faces[f_count + k].vn_idx = vn_idx;
          attrib->faces[f_count + k].vt_idx = vt_idx;
        }

        for (k = 0; k < commands[i].num_f_num_verts; k++) {
          attrib->material_ids[face_count + k] = material_id;
          attrib->face_num_verts[face_count + k] = commands[i].f_num_verts[k];
        }

        f_count += commands[i].num_f;
        face_count += commands[i].num_f_num_verts;
      }
    }
  }

  /* 5. Construct shape information. */
  {
    unsigned int face_count = 0;
    size_t i = 0;
    size_t n = 0;
    size_t shape_idx = 0;

    const char *shape_name = NULL;
    unsigned int shape_name_len = 0;
    const char *prev_shape_name = NULL;
    unsigned int prev_shape_name_len = 0;
    unsigned int prev_shape_face_offset = 0;
    unsigned int prev_face_offset = 0;
    tinyobj_shape_t prev_shape = {NULL, 0, 0};

    /* Find the number of shapes in .obj */
    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_O || commands[i].type == COMMAND_G) {
        n++;
      }
    }

    /* Allocate array of shapes with maximum possible size(+1 for unnamed
     * group/object).
     * Actual # of shapes found in .obj is determined in the later */
    (*shapes) = (tinyobj_shape_t*)TINYOBJ_MALLOC(sizeof(tinyobj_shape_t) * (n + 1));

    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_O || commands[i].type == COMMAND_G) {
        if (commands[i].type == COMMAND_O) {
          shape_name = commands[i].object_name;
          shape_name_len = commands[i].object_name_len;
        } else {
          shape_name = commands[i].group_name;
          shape_name_len = commands[i].group_name_len;
        }

        if (face_count == 0) {
          /* 'o' or 'g' appears before any 'f' */
          prev_shape_name = shape_name;
          prev_shape_name_len = shape_name_len;
          prev_shape_face_offset = face_count;
          prev_face_offset = face_count;
        } else {
          if (shape_idx == 0) {
            /* 'o' or 'g' after some 'v' lines. */
            (*shapes)[shape_idx].name = my_strndup(
                                                   prev_shape_name, prev_shape_name_len); /* may be NULL */
            (*shapes)[shape_idx].face_offset = prev_shape.face_offset;
            (*shapes)[shape_idx].length = face_count - prev_face_offset;
            shape_idx++;

            prev_face_offset = face_count;

          } else {
            if ((face_count - prev_face_offset) > 0) {
              (*shapes)[shape_idx].name =
                my_strndup(prev_shape_name, prev_shape_name_len);
              (*shapes)[shape_idx].face_offset = prev_face_offset;
              (*shapes)[shape_idx].length = face_count - prev_face_offset;
              shape_idx++;
              prev_face_offset = face_count;
            }
          }

          /* Record shape info for succeeding 'o' or 'g' command. */
          prev_shape_name = shape_name;
          prev_shape_name_len = shape_name_len;
          prev_shape_face_offset = face_count;
        }
      }
      if (commands[i].type == COMMAND_F) {
        face_count++;
      }
    }

    if ((face_count - prev_face_offset) > 0) {
      size_t length = face_count - prev_shape_face_offset;
      if (length > 0) {
        (*shapes)[shape_idx].name =
          my_strndup(prev_shape_name, prev_shape_name_len);
        (*shapes)[shape_idx].face_offset = prev_face_offset;
        (*shapes)[shape_idx].length = face_count - prev_face_offset;
        shape_idx++;
      }
    } else {
      /* Guess no 'v' line occurrence after 'o' or 'g', so discards current
       * shape information. */
    }

    (*num_shapes) = shape_idx;
  }

  if (commands) {
    TINYOBJ_FREE(commands);
  }

  destroy_hash_table(&material_table);
  
  (*materials_out) = materials;
  (*num_materials_out) = num_materials;

  return TINYOBJ_SUCCESS;
}

void tinyobj_attrib_init(tinyobj_attrib_t *attrib) {
  attrib->vertices = NULL;
  attrib->num_vertices = 0;
  attrib->normals = NULL;
  attrib->num_normals = 0;
  attrib->texcoords = NULL;
  attrib->num_texcoords = 0;
  attrib->faces = NULL;
  attrib->num_faces = 0;
  attrib->face_num_verts = NULL;
  attrib->num_face_num_verts = 0;
  attrib->material_ids = NULL;
}

void tinyobj_attrib_free(tinyobj_attrib_t *attrib) {
  if (attrib->vertices) TINYOBJ_FREE(attrib->vertices);
  if (attrib->normals) TINYOBJ_FREE(attrib->normals);
  if (attrib->texcoords) TINYOBJ_FREE(attrib->texcoords);
  if (attrib->faces) TINYOBJ_FREE(attrib->faces);
  if (attrib->face_num_verts) TINYOBJ_FREE(attrib->face_num_verts);
  if (attrib->material_ids) TINYOBJ_FREE(attrib->material_ids);
}

void tinyobj_shapes_free(tinyobj_shape_t *shapes, size_t num_shapes) {
  size_t i;
  if (shapes == NULL) return;

  for (i = 0; i < num_shapes; i++) {
    if (shapes[i].name) TINYOBJ_FREE(shapes[i].name);
  }

  TINYOBJ_FREE(shapes);
}

void tinyobj_materials_free(tinyobj_material_t *materials,
                            size_t num_materials) {
  size_t i;
  if (materials == NULL) return;

  for (i = 0; i < num_materials; i++) {
    if (materials[i].name) TINYOBJ_FREE(materials[i].name);
    if (materials[i].ambient_texname) TINYOBJ_FREE(materials[i].ambient_texname);
    if (materials[i].diffuse_texname) TINYOBJ_FREE(materials[i].diffuse_texname);
    if (materials[i].specular_texname) TINYOBJ_FREE(materials[i].specular_texname);
    if (materials[i].specular_highlight_texname)
      TINYOBJ_FREE(materials[i].specular_highlight_texname);
    if (materials[i].bump_texname) TINYOBJ_FREE(materials[i].bump_texname);
    if (materials[i].displacement_texname)
      TINYOBJ_FREE(materials[i].displacement_texname);
    if (materials[i].alpha_texname) TINYOBJ_FREE(materials[i].alpha_texname);
  }

  TINYOBJ_FREE(materials);
}
#endif /* TINYOBJ_LOADER_C_IMPLEMENTATION */

#endif /* TINOBJ_LOADER_C_H_ */
