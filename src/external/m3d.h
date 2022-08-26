/*
 * m3d.h
 * https://gitlab.com/bztsrc/model3d
 *
 * Copyright (C) 2020 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @brief ANSI C89 / C++11 single header importer / exporter SDK for the Model 3D (.M3D) format
 * https://gitlab.com/bztsrc/model3d
 *
 * PNG decompressor included from (with minor modifications to make it C89 valid):
 *  stb_image - v2.13 - public domain image loader - http://nothings.org/stb_image.h
 *
 * @version: 1.0.0
 */

#ifndef _M3D_H_
#define _M3D_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*** configuration ***/
#ifndef M3D_MALLOC
# define M3D_MALLOC(sz)     malloc(sz)
#endif
#ifndef M3D_REALLOC
# define M3D_REALLOC(p,nsz) realloc(p,nsz)
#endif
#ifndef M3D_FREE
# define M3D_FREE(p)        free(p)
#endif
#ifndef M3D_LOG
# define M3D_LOG(x)
#endif
#ifndef M3D_APIVERSION
#define M3D_APIVERSION      0x0100
#ifndef M3D_DOUBLE
typedef float M3D_FLOAT;
#ifndef M3D_EPSILON
/* carefully choosen for IEEE 754 don't change */
#define M3D_EPSILON ((M3D_FLOAT)1e-7)
#endif
#else
typedef double M3D_FLOAT;
#ifndef M3D_EPSILON
#define M3D_EPSILON ((M3D_FLOAT)1e-14)
#endif
#endif
#if !defined(M3D_SMALLINDEX)
typedef uint32_t M3D_INDEX;
typedef uint16_t M3D_VOXEL;
#define M3D_UNDEF 0xffffffff
#define M3D_INDEXMAX 0xfffffffe
#define M3D_VOXUNDEF 0xffff
#define M3D_VOXCLEAR 0xfffe
#else
typedef uint16_t M3D_INDEX;
typedef uint8_t M3D_VOXEL;
#define M3D_UNDEF 0xffff
#define M3D_INDEXMAX 0xfffe
#define M3D_VOXUNDEF 0xff
#define M3D_VOXCLEAR 0xfe
#endif
#define M3D_NOTDEFINED 0xffffffff
#ifndef M3D_NUMBONE
#define M3D_NUMBONE 4
#endif
#ifndef M3D_BONEMAXLEVEL
#define M3D_BONEMAXLEVEL 8
#endif
#ifndef _MSC_VER
#ifndef _inline
#define _inline __inline__
#endif
#define _pack __attribute__((packed))
#define _unused __attribute__((unused))
#else
#define _inline
#define _pack
#define _unused __pragma(warning(suppress:4100))
#endif
#ifndef  __cplusplus
#define _register register
#else
#define _register
#endif

/*** File format structures ***/

/**
 * M3D file format structure
 *  3DMO m3dchunk_t file header chunk, may followed by compressed data
 *  PRVW preview chunk (optional)
 *  HEAD m3dhdr_t model header chunk
 *  n x m3dchunk_t more chunks follow
 *      CMAP color map chunk (optional)
 *      TMAP texture map chunk (optional)
 *      VRTS vertex data chunk (optional if it's a material library)
 *      BONE bind-pose skeleton, bone hierarchy chunk (optional)
 *          n x m3db_t contains propably more, but at least one bone
 *          n x m3ds_t skin group records
 *      MTRL* material chunk(s), can be more (optional)
 *          n x m3dp_t each material contains propapbly more, but at least one property
 *                     the properties are configurable with a static array, see m3d_propertytypes
 *      n x m3dchunk_t at least one, but maybe more face chunks
 *          PROC* procedural face, or
 *          MESH* triangle mesh (vertex index list) or
 *          VOXT, VOXD* voxel image (converted to mesh) or
 *          SHPE* mathematical shapes like parameterized surfaces
 *      LBLS* annotation label chunks, can be more (optional)
 *      ACTN* action chunk(s), animation-pose skeletons, can be more (optional)
 *          n x m3dfr_t each action contains probably more, but at least one frame
 *              n x m3dtr_t each frame contains probably more, but at least one transformation
 *      ASET* inlined asset chunk(s), can be more (optional)
 *  OMD3 end chunk
 *
 * Typical chunks for a game engine: 3DMO, HEAD, CMAP, TMAP, VRTS, BONE, MTRL, MESH, ACTN, OMD3
 * Typical chunks for distibution:   3DMO, PRVW, HEAD, CMAP, TMAP, VRTS, BONE, MTRL, MESH, ACTN, ASET, OMD3
 * Typical chunks for voxel image:   3DMO, HEAD, CMAP, MTRL, VOXT, VOXD, VOXD, VOXD, OMD3
 * Typical chunks for CAD software:  3DMO, PRVW, HEAD, CMAP, TMAP, VRTS, MTRL, SHPE, LBLS, OMD3
 */
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif

typedef struct {
    char magic[4];
    uint32_t length;
    float scale; /* deliberately not M3D_FLOAT */
    uint32_t types;
} _pack m3dhdr_t;

typedef struct {
    char magic[4];
    uint32_t length;
} _pack m3dchunk_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

/*** in-memory model structure ***/

/* textmap entry */
typedef struct {
    M3D_FLOAT u;
    M3D_FLOAT v;
} m3dti_t;
#define m3d_textureindex_t m3dti_t

/* texture */
typedef struct {
    char *name;                 /* texture name */
    uint8_t *d;                 /* pixels data */
    uint16_t w;                 /* width */
    uint16_t h;                 /* height */
    uint8_t f;                  /* format, 1 = grayscale, 2 = grayscale+alpha, 3 = rgb, 4 = rgba */
} m3dtx_t;
#define m3d_texturedata_t m3dtx_t

typedef struct {
    M3D_INDEX vertexid;
    M3D_FLOAT weight;
} m3dw_t;
#define m3d_weight_t m3dw_t

/* bone entry */
typedef struct {
    M3D_INDEX parent;           /* parent bone index */
    char *name;                 /* name for this bone */
    M3D_INDEX pos;              /* vertex index position */
    M3D_INDEX ori;              /* vertex index orientation (quaternion) */
    M3D_INDEX numweight;        /* number of controlled vertices */
    m3dw_t *weight;             /* weights for those vertices */
    M3D_FLOAT mat4[16];         /* transformation matrix */
} m3db_t;
#define m3d_bone_t m3db_t

/* skin: bone per vertex entry */
typedef struct {
    M3D_INDEX boneid[M3D_NUMBONE];
    M3D_FLOAT weight[M3D_NUMBONE];
} m3ds_t;
#define m3d_skin_t m3ds_t

/* vertex entry */
typedef struct {
    M3D_FLOAT x;                /* 3D coordinates and weight */
    M3D_FLOAT y;
    M3D_FLOAT z;
    M3D_FLOAT w;
    uint32_t color;             /* default vertex color */
    M3D_INDEX skinid;           /* skin index */
#ifdef M3D_VERTEXTYPE
    uint8_t type;
#endif
} m3dv_t;
#define m3d_vertex_t m3dv_t

/* material property formats */
enum {
    m3dpf_color,
    m3dpf_uint8,
    m3dpf_uint16,
    m3dpf_uint32,
    m3dpf_float,
    m3dpf_map
};
typedef struct {
    uint8_t format;
    uint8_t id;
#ifdef M3D_ASCII
#define M3D_PROPERTYDEF(f,i,n) { (f), (i), (char*)(n) }
    char *key;
#else
#define M3D_PROPERTYDEF(f,i,n) { (f), (i) }
#endif
} m3dpd_t;

/* material property types */
/* You shouldn't change the first 8 display and first 4 physical property. Assign the rest as you like. */
enum {
    m3dp_Kd = 0,                /* scalar display properties */
    m3dp_Ka,
    m3dp_Ks,
    m3dp_Ns,
    m3dp_Ke,
    m3dp_Tf,
    m3dp_Km,
    m3dp_d,
    m3dp_il,

    m3dp_Pr = 64,               /* scalar physical properties */
    m3dp_Pm,
    m3dp_Ps,
    m3dp_Ni,
    m3dp_Nt,

    m3dp_map_Kd = 128,          /* textured display map properties */
    m3dp_map_Ka,
    m3dp_map_Ks,
    m3dp_map_Ns,
    m3dp_map_Ke,
    m3dp_map_Tf,
    m3dp_map_Km, /* bump map */
    m3dp_map_D,
    m3dp_map_N,  /* normal map */

    m3dp_map_Pr = 192,          /* textured physical map properties */
    m3dp_map_Pm,
    m3dp_map_Ps,
    m3dp_map_Ni,
    m3dp_map_Nt
};
enum {                          /* aliases */
    m3dp_bump = m3dp_map_Km,
    m3dp_map_il = m3dp_map_N,
    m3dp_refl = m3dp_map_Pm
};

/* material property */
typedef struct {
    uint8_t type;               /* property type, see "m3dp_*" enumeration */
    union {
        uint32_t color;         /* if value is a color, m3dpf_color */
        uint32_t num;           /* if value is a number, m3dpf_uint8, m3pf_uint16, m3dpf_uint32 */
        float    fnum;          /* if value is a floating point number, m3dpf_float */
        M3D_INDEX textureid;    /* if value is a texture, m3dpf_map */
    } value;
} m3dp_t;
#define m3d_property_t m3dp_t

/* material entry */
typedef struct {
    char *name;                 /* name of the material */
    uint8_t numprop;            /* number of properties */
    m3dp_t *prop;               /* properties array */
} m3dm_t;
#define m3d_material_t m3dm_t

/* face entry */
typedef struct {
    M3D_INDEX materialid;       /* material index */
    M3D_INDEX vertex[3];        /* 3D points of the triangle in CCW order */
    M3D_INDEX normal[3];        /* normal vectors */
    M3D_INDEX texcoord[3];      /* UV coordinates */
#ifdef M3D_VERTEXMAX
    M3D_INDEX paramid;          /* parameter index */
    M3D_INDEX vertmax[3];       /* maximum 3D points of the triangle in CCW order */
#endif
} m3df_t;
#define m3d_face_t m3df_t

typedef struct {
    uint16_t count;
    char *name;
} m3dvi_t;
#define m3d_voxelitem_t m3dvi_t
#define m3d_parameter_t m3dvi_t

/* voxel types (voxel palette) */
typedef struct {
    char *name;                 /* technical name of the voxel */
    uint8_t rotation;           /* rotation info */
    uint16_t voxshape;          /* voxel shape */
    M3D_INDEX materialid;       /* material index */
    uint32_t color;             /* default voxel color */
    M3D_INDEX skinid;           /* skin index */
    uint8_t numitem;            /* number of sub-voxels */
    m3dvi_t *item;              /* list of sub-voxels */
} m3dvt_t;
#define m3d_voxeltype_t m3dvt_t

/* voxel data blocks */
typedef struct {
    char *name;                 /* name of the block */
    int32_t x, y, z;            /* position */
    uint32_t w, h, d;           /* dimension */
    uint8_t uncertain;          /* probability */
    uint8_t groupid;            /* block group id */
    M3D_VOXEL *data;            /* voxel data, indices to voxel type */
} m3dvx_t;
#define m3d_voxel_t m3dvx_t

/* shape command types. must match the row in m3d_commandtypes */
enum {
    /* special commands */
    m3dc_use = 0,               /* use material */
    m3dc_inc,                   /* include another shape */
    m3dc_mesh,                  /* include part of polygon mesh */
    /* approximations */
    m3dc_div,                   /* subdivision by constant resolution for both u, v */
    m3dc_sub,                   /* subdivision by constant, different for u and v */
    m3dc_len,                   /* spacial subdivision by maxlength */
    m3dc_dist,                  /* subdivision by maxdistance and maxangle */
    /* modifiers */
    m3dc_degu,                  /* degree for both u, v */
    m3dc_deg,                   /* separate degree for u and v */
    m3dc_rangeu,                /* range for u */
    m3dc_range,                 /* range for u and v */
    m3dc_paru,                  /* u parameters (knots) */
    m3dc_parv,                  /* v parameters */
    m3dc_trim,                  /* outer trimming curve */
    m3dc_hole,                  /* inner trimming curve */
    m3dc_scrv,                  /* spacial curve */
    m3dc_sp,                    /* special points */
    /* helper curves */
    m3dc_bez1,                  /* Bezier 1D */
    m3dc_bsp1,                  /* B-spline 1D */
    m3dc_bez2,                  /* bezier 2D */
    m3dc_bsp2,                  /* B-spline 2D */
    /* surfaces */
    m3dc_bezun,                 /* Bezier 3D with control, UV, normal */
    m3dc_bezu,                  /* with control and UV */
    m3dc_bezn,                  /* with control and normal */
    m3dc_bez,                   /* control points only */
    m3dc_nurbsun,               /* B-spline 3D */
    m3dc_nurbsu,
    m3dc_nurbsn,
    m3dc_nurbs,
    m3dc_conn,                 /* connect surfaces */
    /* geometrical */
    m3dc_line,
    m3dc_polygon,
    m3dc_circle,
    m3dc_cylinder,
    m3dc_shpere,
    m3dc_torus,
    m3dc_cone,
    m3dc_cube
};

/* shape command argument types */
enum {
    m3dcp_mi_t = 1,             /* material index */
    m3dcp_hi_t,                 /* shape index */
    m3dcp_fi_t,                 /* face index */
    m3dcp_ti_t,                 /* texture map index */
    m3dcp_vi_t,                 /* vertex index */
    m3dcp_qi_t,                 /* vertex index for quaternions */
    m3dcp_vc_t,                 /* coordinate or radius, float scalar */
    m3dcp_i1_t,                 /* int8 scalar */
    m3dcp_i2_t,                 /* int16 scalar */
    m3dcp_i4_t,                 /* int32 scalar */
    m3dcp_va_t                  /* variadic arguments */
};

#define M3D_CMDMAXARG 8         /* if you increase this, add more arguments to the macro below */
typedef struct {
#ifdef M3D_ASCII
#define M3D_CMDDEF(t,n,p,a,b,c,d,e,f,g,h) { (char*)(n), (p), { (a), (b), (c), (d), (e), (f), (g), (h) } }
    char *key;
#else
#define M3D_CMDDEF(t,n,p,a,b,c,d,e,f,g,h) { (p), { (a), (b), (c), (d), (e), (f), (g), (h) } }
#endif
    uint8_t p;
    uint8_t a[M3D_CMDMAXARG];
} m3dcd_t;

/* shape command */
typedef struct {
    uint16_t type;              /* shape type */
    uint32_t *arg;              /* arguments array */
} m3dc_t;
#define m3d_shapecommand_t m3dc_t

/* shape entry */
typedef struct {
    char *name;                 /* name of the mathematical shape */
    M3D_INDEX group;            /* group this shape belongs to or -1 */
    uint32_t numcmd;            /* number of commands */
    m3dc_t *cmd;                /* commands array */
} m3dh_t;
#define m3d_shape_t m3dh_t

/* label entry */
typedef struct {
    char *name;                 /* name of the annotation layer or NULL */
    char *lang;                 /* language code or NULL */
    char *text;                 /* the label text */
    uint32_t color;             /* color */
    M3D_INDEX vertexid;         /* the vertex the label refers to */
} m3dl_t;
#define m3d_label_t m3dl_t

/* frame transformations / working copy skeleton entry */
typedef struct {
    M3D_INDEX boneid;           /* selects a node in bone hierarchy */
    M3D_INDEX pos;              /* vertex index new position */
    M3D_INDEX ori;              /* vertex index new orientation (quaternion) */
} m3dtr_t;
#define m3d_transform_t m3dtr_t

/* animation frame entry */
typedef struct {
    uint32_t msec;              /* frame's position on the timeline, timestamp */
    M3D_INDEX numtransform;     /* number of transformations in this frame */
    m3dtr_t *transform;         /* transformations */
} m3dfr_t;
#define m3d_frame_t m3dfr_t

/* model action entry */
typedef struct {
    char *name;                 /* name of the action */
    uint32_t durationmsec;      /* duration in millisec (1/1000 sec) */
    M3D_INDEX numframe;         /* number of frames in this animation */
    m3dfr_t *frame;             /* frames array */
} m3da_t;
#define m3d_action_t m3da_t

/* inlined asset */
typedef struct {
    char *name;                 /* asset name (same pointer as in texture[].name) */
    uint8_t *data;              /* compressed asset data */
    uint32_t length;            /* compressed data length */
} m3di_t;
#define m3d_inlinedasset_t m3di_t

/*** in-memory model structure ***/
#define M3D_FLG_FREERAW     (1<<0)
#define M3D_FLG_FREESTR     (1<<1)
#define M3D_FLG_MTLLIB      (1<<2)
#define M3D_FLG_GENNORM     (1<<3)

typedef struct {
    m3dhdr_t *raw;              /* pointer to raw data */
    char flags;                 /* internal flags */
    signed char errcode;        /* returned error code */
    char vc_s, vi_s, si_s, ci_s, ti_s, bi_s, nb_s, sk_s, fc_s, hi_s, fi_s, vd_s, vp_s;  /* decoded sizes for types */
    char *name;                 /* name of the model, like "Utah teapot" */
    char *license;              /* usage condition or license, like "MIT", "LGPL" or "BSD-3clause" */
    char *author;               /* nickname, email, homepage or github URL etc. */
    char *desc;                 /* comments, descriptions. May contain '\n' newline character */
    M3D_FLOAT scale;            /* the model's bounding cube's size in SI meters */
    M3D_INDEX numcmap;
    uint32_t *cmap;             /* color map */
    M3D_INDEX numtmap;
    m3dti_t *tmap;              /* texture map indices */
    M3D_INDEX numtexture;
    m3dtx_t *texture;           /* uncompressed textures */
    M3D_INDEX numbone;
    m3db_t *bone;               /* bone hierarchy */
    M3D_INDEX numvertex;
    m3dv_t *vertex;             /* vertex data */
    M3D_INDEX numskin;
    m3ds_t *skin;               /* skin data */
    M3D_INDEX nummaterial;
    m3dm_t *material;           /* material list */
#ifdef M3D_VERTEXMAX
    M3D_INDEX numparam;
    m3dvi_t *param;             /* parameters and their values list */
#endif
    M3D_INDEX numface;
    m3df_t *face;               /* model face, polygon (triangle) mesh */
    M3D_INDEX numvoxtype;
    m3dvt_t *voxtype;           /* model face, voxel types */
    M3D_INDEX numvoxel;
    m3dvx_t *voxel;             /* model face, cubes compressed into voxels */
    M3D_INDEX numshape;
    m3dh_t *shape;              /* model face, shape commands */
    M3D_INDEX numlabel;
    m3dl_t *label;              /* annotation labels */
    M3D_INDEX numaction;
    m3da_t *action;             /* action animations */
    M3D_INDEX numinlined;
    m3di_t *inlined;            /* inlined assets */
    M3D_INDEX numextra;
    m3dchunk_t **extra;         /* unknown chunks, application / engine specific data probably */
    m3di_t preview;             /* preview chunk */
} m3d_t;

/*** export parameters ***/
#define M3D_EXP_INT8        0
#define M3D_EXP_INT16       1
#define M3D_EXP_FLOAT       2
#define M3D_EXP_DOUBLE      3

#define M3D_EXP_NOCMAP      (1<<0)
#define M3D_EXP_NOMATERIAL  (1<<1)
#define M3D_EXP_NOFACE      (1<<2)
#define M3D_EXP_NONORMAL    (1<<3)
#define M3D_EXP_NOTXTCRD    (1<<4)
#define M3D_EXP_FLIPTXTCRD  (1<<5)
#define M3D_EXP_NORECALC    (1<<6)
#define M3D_EXP_IDOSUCK     (1<<7)
#define M3D_EXP_NOBONE      (1<<8)
#define M3D_EXP_NOACTION    (1<<9)
#define M3D_EXP_INLINE      (1<<10)
#define M3D_EXP_EXTRA       (1<<11)
#define M3D_EXP_NOZLIB      (1<<14)
#define M3D_EXP_ASCII       (1<<15)
#define M3D_EXP_NOVRTMAX    (1<<16)

/*** error codes ***/
#define M3D_SUCCESS         0
#define M3D_ERR_ALLOC       -1
#define M3D_ERR_BADFILE     -2
#define M3D_ERR_UNIMPL      -65
#define M3D_ERR_UNKPROP     -66
#define M3D_ERR_UNKMESH     -67
#define M3D_ERR_UNKIMG      -68
#define M3D_ERR_UNKFRAME    -69
#define M3D_ERR_UNKCMD      -70
#define M3D_ERR_UNKVOX      -71
#define M3D_ERR_TRUNC       -72
#define M3D_ERR_CMAP        -73
#define M3D_ERR_TMAP        -74
#define M3D_ERR_VRTS        -75
#define M3D_ERR_BONE        -76
#define M3D_ERR_MTRL        -77
#define M3D_ERR_SHPE        -78
#define M3D_ERR_VOXT        -79

#define M3D_ERR_ISFATAL(x)  ((x) < 0 && (x) > -65)

/* callbacks */
typedef unsigned char *(*m3dread_t)(char *filename, unsigned int *size);                        /* read file contents into buffer */
typedef void (*m3dfree_t)(void *buffer);                                                        /* free file contents buffer */
typedef int (*m3dtxsc_t)(const char *name, const void *script, uint32_t len, m3dtx_t *output);  /* interpret texture script */
typedef int (*m3dprsc_t)(const char *name, const void *script, uint32_t len, m3d_t *model);     /* interpret surface script */
#endif /* ifndef M3D_APIVERSION */

/*** C prototypes ***/
/* import / export */
m3d_t *m3d_load(unsigned char *data, m3dread_t readfilecb, m3dfree_t freecb, m3d_t *mtllib);
unsigned char *m3d_save(m3d_t *model, int quality, int flags, unsigned int *size);
void m3d_free(m3d_t *model);
/* generate animation pose skeleton */
m3dtr_t *m3d_frame(m3d_t *model, M3D_INDEX actionid, M3D_INDEX frameid, m3dtr_t *skeleton);
m3db_t *m3d_pose(m3d_t *model, M3D_INDEX actionid, uint32_t msec);

/* private prototypes used by both importer and exporter */
char *_m3d_safestr(char *in, int morelines);

/*** C implementation ***/
#ifdef M3D_IMPLEMENTATION
#if !defined(M3D_NOIMPORTER) || defined(M3D_EXPORTER)
/* material property definitions */
static m3dpd_t m3d_propertytypes[] = {
    M3D_PROPERTYDEF(m3dpf_color, m3dp_Kd, "Kd"),    /* diffuse color */
    M3D_PROPERTYDEF(m3dpf_color, m3dp_Ka, "Ka"),    /* ambient color */
    M3D_PROPERTYDEF(m3dpf_color, m3dp_Ks, "Ks"),    /* specular color */
    M3D_PROPERTYDEF(m3dpf_float, m3dp_Ns, "Ns"),    /* specular exponent */
    M3D_PROPERTYDEF(m3dpf_color, m3dp_Ke, "Ke"),    /* emissive (emitting light of this color) */
    M3D_PROPERTYDEF(m3dpf_color, m3dp_Tf, "Tf"),    /* transmission color */
    M3D_PROPERTYDEF(m3dpf_float, m3dp_Km, "Km"),    /* bump strength */
    M3D_PROPERTYDEF(m3dpf_float, m3dp_d,  "d"),     /* dissolve (transparency) */
    M3D_PROPERTYDEF(m3dpf_uint8, m3dp_il, "il"),    /* illumination model (informational, ignored by PBR-shaders) */

    M3D_PROPERTYDEF(m3dpf_float, m3dp_Pr, "Pr"),    /* roughness */
    M3D_PROPERTYDEF(m3dpf_float, m3dp_Pm, "Pm"),    /* metallic, also reflection */
    M3D_PROPERTYDEF(m3dpf_float, m3dp_Ps, "Ps"),    /* sheen */
    M3D_PROPERTYDEF(m3dpf_float, m3dp_Ni, "Ni"),    /* index of refraction (optical density) */
    M3D_PROPERTYDEF(m3dpf_float, m3dp_Nt, "Nt"),    /* thickness of face in millimeter, for printing */

    /* aliases, note that "map_*" aliases are handled automatically */
    M3D_PROPERTYDEF(m3dpf_map, m3dp_map_Km, "bump"),
    M3D_PROPERTYDEF(m3dpf_map, m3dp_map_N, "map_N"),/* as normal map has no scalar version, it's counterpart is 'il' */
    M3D_PROPERTYDEF(m3dpf_map, m3dp_map_Pm, "refl")
};
/* shape command definitions. if more commands start with the same string, the longer must come first */
static m3dcd_t m3d_commandtypes[] = {
    /* technical */
    M3D_CMDDEF(m3dc_use,     "use",     1, m3dcp_mi_t, 0, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_inc,     "inc",     3, m3dcp_hi_t, m3dcp_vi_t, m3dcp_qi_t, m3dcp_vi_t, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_mesh,    "mesh",    1, m3dcp_fi_t, m3dcp_fi_t, m3dcp_vi_t, m3dcp_qi_t, m3dcp_vi_t, 0, 0, 0),
    /* approximations */
    M3D_CMDDEF(m3dc_div,     "div",     1, m3dcp_vc_t, 0, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_sub,     "sub",     2, m3dcp_vc_t, m3dcp_vc_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_len,     "len",     1, m3dcp_vc_t, 0, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_dist,    "dist",    2, m3dcp_vc_t, m3dcp_vc_t, 0, 0, 0, 0, 0, 0),
    /* modifiers */
    M3D_CMDDEF(m3dc_degu,    "degu",    1, m3dcp_i1_t, 0, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_deg,     "deg",     2, m3dcp_i1_t, m3dcp_i1_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_rangeu,  "rangeu",  1, m3dcp_ti_t, 0, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_range,   "range",   2, m3dcp_ti_t, m3dcp_ti_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_paru,    "paru",    2, m3dcp_va_t, m3dcp_vc_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_parv,    "parv",    2, m3dcp_va_t, m3dcp_vc_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_trim,    "trim",    3, m3dcp_va_t, m3dcp_ti_t, m3dcp_i2_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_hole,    "hole",    3, m3dcp_va_t, m3dcp_ti_t, m3dcp_i2_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_scrv,    "scrv",    3, m3dcp_va_t, m3dcp_ti_t, m3dcp_i2_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_sp,      "sp",      2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    /* helper curves */
    M3D_CMDDEF(m3dc_bez1,    "bez1",    2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_bsp1,    "bsp1",    2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_bez2,    "bez2",    2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_bsp2,    "bsp2",    2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    /* surfaces */
    M3D_CMDDEF(m3dc_bezun,   "bezun",   4, m3dcp_va_t, m3dcp_vi_t, m3dcp_ti_t, m3dcp_vi_t, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_bezu,    "bezu",    3, m3dcp_va_t, m3dcp_vi_t, m3dcp_ti_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_bezn,    "bezn",    3, m3dcp_va_t, m3dcp_vi_t, m3dcp_vi_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_bez,     "bez",     2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_nurbsun, "nurbsun", 4, m3dcp_va_t, m3dcp_vi_t, m3dcp_ti_t, m3dcp_vi_t, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_nurbsu,  "nurbsu",  3, m3dcp_va_t, m3dcp_vi_t, m3dcp_ti_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_nurbsn,  "nurbsn",  3, m3dcp_va_t, m3dcp_vi_t, m3dcp_vi_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_nurbs,   "nurbs",   2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_conn,    "conn",    6, m3dcp_i2_t, m3dcp_ti_t, m3dcp_i2_t, m3dcp_i2_t, m3dcp_ti_t, m3dcp_i2_t, 0, 0),
    /* geometrical */
    M3D_CMDDEF(m3dc_line,    "line",    2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_polygon, "polygon", 2, m3dcp_va_t, m3dcp_vi_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_circle,  "circle",  3, m3dcp_vi_t, m3dcp_qi_t, m3dcp_vc_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_cylinder,"cylinder",6, m3dcp_vi_t, m3dcp_qi_t, m3dcp_vc_t, m3dcp_vi_t, m3dcp_qi_t, m3dcp_vc_t, 0, 0),
    M3D_CMDDEF(m3dc_shpere,  "shpere",  2, m3dcp_vi_t, m3dcp_vc_t, 0, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_torus,   "torus",   4, m3dcp_vi_t, m3dcp_qi_t, m3dcp_vc_t, m3dcp_vc_t, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_cone,    "cone",    3, m3dcp_vi_t, m3dcp_vi_t, m3dcp_vi_t, 0, 0, 0, 0, 0),
    M3D_CMDDEF(m3dc_cube,    "cube",    3, m3dcp_vi_t, m3dcp_vi_t, m3dcp_vi_t, 0, 0, 0, 0, 0)
};
#endif

#include <stdlib.h>
#include <string.h>

#if !defined(M3D_NOIMPORTER) && !defined(STBI_INCLUDE_STB_IMAGE_H)
/* PNG decompressor from

   stb_image - v2.23 - public domain image loader - http://nothings.org/stb_image.h
*/
static const char *_m3dstbi__g_failure_reason;

enum
{
   STBI_default = 0,

   STBI_grey       = 1,
   STBI_grey_alpha = 2,
   STBI_rgb        = 3,
   STBI_rgb_alpha  = 4
};

enum
{
   STBI__SCAN_load=0,
   STBI__SCAN_type,
   STBI__SCAN_header
};

typedef unsigned short _m3dstbi_us;

typedef uint16_t _m3dstbi__uint16;
typedef int16_t  _m3dstbi__int16;
typedef uint32_t _m3dstbi__uint32;
typedef int32_t  _m3dstbi__int32;

typedef struct
{
   _m3dstbi__uint32 img_x, img_y;
   int img_n, img_out_n;

   void *io_user_data;

   int read_from_callbacks;
   int buflen;
   unsigned char buffer_start[128];

   unsigned char *img_buffer, *img_buffer_end;
   unsigned char *img_buffer_original, *img_buffer_original_end;
} _m3dstbi__context;

typedef struct
{
   int bits_per_channel;
   int num_channels;
   int channel_order;
} _m3dstbi__result_info;

#define STBI_ASSERT(v)
#define STBI_NOTUSED(v)  (void)sizeof(v)
#define STBI__BYTECAST(x)  ((unsigned char) ((x) & 255))
#define STBI_MALLOC(sz)           M3D_MALLOC(sz)
#define STBI_REALLOC(p,newsz)     M3D_REALLOC(p,newsz)
#define STBI_FREE(p)              M3D_FREE(p)
#define STBI_REALLOC_SIZED(p,oldsz,newsz) STBI_REALLOC(p,newsz)

_inline static unsigned char _m3dstbi__get8(_m3dstbi__context *s)
{
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;
   return 0;
}

_inline static int _m3dstbi__at_eof(_m3dstbi__context *s)
{
   return s->img_buffer >= s->img_buffer_end;
}

static void _m3dstbi__skip(_m3dstbi__context *s, int n)
{
   if (n < 0) {
      s->img_buffer = s->img_buffer_end;
      return;
   }
   s->img_buffer += n;
}

static int _m3dstbi__getn(_m3dstbi__context *s, unsigned char *buffer, int n)
{
   if (s->img_buffer+n <= s->img_buffer_end) {
      memcpy(buffer, s->img_buffer, n);
      s->img_buffer += n;
      return 1;
   } else
      return 0;
}

static int _m3dstbi__get16be(_m3dstbi__context *s)
{
   int z = _m3dstbi__get8(s);
   return (z << 8) + _m3dstbi__get8(s);
}

static _m3dstbi__uint32 _m3dstbi__get32be(_m3dstbi__context *s)
{
   _m3dstbi__uint32 z = _m3dstbi__get16be(s);
   return (z << 16) + _m3dstbi__get16be(s);
}

#define _m3dstbi__err(x,y)  _m3dstbi__errstr(y)
static int _m3dstbi__errstr(const char *str)
{
   _m3dstbi__g_failure_reason = str;
   return 0;
}

_inline static void *_m3dstbi__malloc(size_t size)
{
    return STBI_MALLOC(size);
}

static int _m3dstbi__addsizes_valid(int a, int b)
{
   if (b < 0) return 0;
   return a <= 2147483647 - b;
}

static int _m3dstbi__mul2sizes_valid(int a, int b)
{
   if (a < 0 || b < 0) return 0;
   if (b == 0) return 1;
   return a <= 2147483647/b;
}

static int _m3dstbi__mad2sizes_valid(int a, int b, int add)
{
   return _m3dstbi__mul2sizes_valid(a, b) && _m3dstbi__addsizes_valid(a*b, add);
}

static int _m3dstbi__mad3sizes_valid(int a, int b, int c, int add)
{
   return _m3dstbi__mul2sizes_valid(a, b) && _m3dstbi__mul2sizes_valid(a*b, c) &&
      _m3dstbi__addsizes_valid(a*b*c, add);
}

static void *_m3dstbi__malloc_mad2(int a, int b, int add)
{
   if (!_m3dstbi__mad2sizes_valid(a, b, add)) return NULL;
   return _m3dstbi__malloc(a*b + add);
}

static void *_m3dstbi__malloc_mad3(int a, int b, int c, int add)
{
   if (!_m3dstbi__mad3sizes_valid(a, b, c, add)) return NULL;
   return _m3dstbi__malloc(a*b*c + add);
}

static unsigned char _m3dstbi__compute_y(int r, int g, int b)
{
   return (unsigned char) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static unsigned char *_m3dstbi__convert_format(unsigned char *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
   int i,j;
   unsigned char *good;

   if (req_comp == img_n) return data;
   STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) _m3dstbi__malloc_mad3(req_comp, x, y, 0);
   if (good == NULL) {
      STBI_FREE(data);
      _m3dstbi__err("outofmem", "Out of memory");
      return NULL;
   }

   for (j=0; j < (int) y; ++j) {
      unsigned char *src  = data + j * x * img_n   ;
      unsigned char *dest = good + j * x * req_comp;

      #define STBI__COMBO(a,b)  ((a)*8+(b))
      #define STBI__CASE(a,b)   case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
      switch (STBI__COMBO(img_n, req_comp)) {
         STBI__CASE(1,2) { dest[0]=src[0], dest[1]=255;                                     } break;
         STBI__CASE(1,3) { dest[0]=dest[1]=dest[2]=src[0];                                  } break;
         STBI__CASE(1,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=255;                     } break;
         STBI__CASE(2,1) { dest[0]=src[0];                                                  } break;
         STBI__CASE(2,3) { dest[0]=dest[1]=dest[2]=src[0];                                  } break;
         STBI__CASE(2,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1];                  } break;
         STBI__CASE(3,4) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255;        } break;
         STBI__CASE(3,1) { dest[0]=_m3dstbi__compute_y(src[0],src[1],src[2]);                   } break;
         STBI__CASE(3,2) { dest[0]=_m3dstbi__compute_y(src[0],src[1],src[2]), dest[1] = 255;    } break;
         STBI__CASE(4,1) { dest[0]=_m3dstbi__compute_y(src[0],src[1],src[2]);                   } break;
         STBI__CASE(4,2) { dest[0]=_m3dstbi__compute_y(src[0],src[1],src[2]), dest[1] = src[3]; } break;
         STBI__CASE(4,3) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2];                    } break;
         default: STBI_ASSERT(0);
      }
      #undef STBI__CASE
   }

   STBI_FREE(data);
   return good;
}

static _m3dstbi__uint16 _m3dstbi__compute_y_16(int r, int g, int b)
{
   return (_m3dstbi__uint16) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static _m3dstbi__uint16 *_m3dstbi__convert_format16(_m3dstbi__uint16 *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
   int i,j;
   _m3dstbi__uint16 *good;

   if (req_comp == img_n) return data;
   STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

   good = (_m3dstbi__uint16 *) _m3dstbi__malloc(req_comp * x * y * 2);
   if (good == NULL) {
      STBI_FREE(data);
      _m3dstbi__err("outofmem", "Out of memory");
      return NULL;
   }

   for (j=0; j < (int) y; ++j) {
      _m3dstbi__uint16 *src  = data + j * x * img_n   ;
      _m3dstbi__uint16 *dest = good + j * x * req_comp;

      #define STBI__COMBO(a,b)  ((a)*8+(b))
      #define STBI__CASE(a,b)   case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
      switch (STBI__COMBO(img_n, req_comp)) {
         STBI__CASE(1,2) { dest[0]=src[0], dest[1]=0xffff;                                     } break;
         STBI__CASE(1,3) { dest[0]=dest[1]=dest[2]=src[0];                                     } break;
         STBI__CASE(1,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=0xffff;                     } break;
         STBI__CASE(2,1) { dest[0]=src[0];                                                     } break;
         STBI__CASE(2,3) { dest[0]=dest[1]=dest[2]=src[0];                                     } break;
         STBI__CASE(2,4) { dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1];                     } break;
         STBI__CASE(3,4) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=0xffff;        } break;
         STBI__CASE(3,1) { dest[0]=_m3dstbi__compute_y_16(src[0],src[1],src[2]);                   } break;
         STBI__CASE(3,2) { dest[0]=_m3dstbi__compute_y_16(src[0],src[1],src[2]), dest[1] = 0xffff; } break;
         STBI__CASE(4,1) { dest[0]=_m3dstbi__compute_y_16(src[0],src[1],src[2]);                   } break;
         STBI__CASE(4,2) { dest[0]=_m3dstbi__compute_y_16(src[0],src[1],src[2]), dest[1] = src[3]; } break;
         STBI__CASE(4,3) { dest[0]=src[0],dest[1]=src[1],dest[2]=src[2];                       } break;
         default: STBI_ASSERT(0);
      }
      #undef STBI__CASE
   }

   STBI_FREE(data);
   return good;
}

#define STBI__ZFAST_BITS  9
#define STBI__ZFAST_MASK  ((1 << STBI__ZFAST_BITS) - 1)

typedef struct
{
   _m3dstbi__uint16 fast[1 << STBI__ZFAST_BITS];
   _m3dstbi__uint16 firstcode[16];
   int maxcode[17];
   _m3dstbi__uint16 firstsymbol[16];
   unsigned char  size[288];
   _m3dstbi__uint16 value[288];
} _m3dstbi__zhuffman;

_inline static int _m3dstbi__bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

_inline static int _m3dstbi__bit_reverse(int v, int bits)
{
   STBI_ASSERT(bits <= 16);
   return _m3dstbi__bitreverse16(v) >> (16-bits);
}

static int _m3dstbi__zbuild_huffman(_m3dstbi__zhuffman *z, unsigned char *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   memset(sizes, 0, sizeof(sizes));
   memset(z->fast, 0, sizeof(z->fast));
   for (i=0; i < num; ++i)
      ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
      if (sizes[i] > (1 << i))
         return _m3dstbi__err("bad sizes", "Corrupt PNG");
   code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (_m3dstbi__uint16) code;
      z->firstsymbol[i] = (_m3dstbi__uint16) k;
      code = (code + sizes[i]);
      if (sizes[i])
         if (code-1 >= (1 << i)) return _m3dstbi__err("bad codelengths","Corrupt PNG");
      z->maxcode[i] = code << (16-i);
      code <<= 1;
      k += sizes[i];
   }
   z->maxcode[16] = 0x10000;
   for (i=0; i < num; ++i) {
      int s = sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         _m3dstbi__uint16 fastv = (_m3dstbi__uint16) ((s << 9) | i);
         z->size [c] = (unsigned char     ) s;
         z->value[c] = (_m3dstbi__uint16) i;
         if (s <= STBI__ZFAST_BITS) {
            int j = _m3dstbi__bit_reverse(next_code[s],s);
            while (j < (1 << STBI__ZFAST_BITS)) {
               z->fast[j] = fastv;
               j += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}

typedef struct
{
   unsigned char *zbuffer, *zbuffer_end;
   int num_bits;
   _m3dstbi__uint32 code_buffer;

   char *zout;
   char *zout_start;
   char *zout_end;
   int   z_expandable;

   _m3dstbi__zhuffman z_length, z_distance;
} _m3dstbi__zbuf;

_inline static unsigned char _m3dstbi__zget8(_m3dstbi__zbuf *z)
{
   if (z->zbuffer >= z->zbuffer_end) return 0;
   return *z->zbuffer++;
}

static void _m3dstbi__fill_bits(_m3dstbi__zbuf *z)
{
   do {
      STBI_ASSERT(z->code_buffer < (1U << z->num_bits));
      z->code_buffer |= (unsigned int) _m3dstbi__zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

_inline static unsigned int _m3dstbi__zreceive(_m3dstbi__zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) _m3dstbi__fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

static int _m3dstbi__zhuffman_decode_slowpath(_m3dstbi__zbuf *a, _m3dstbi__zhuffman *z)
{
   int b,s,k;
   k = _m3dstbi__bit_reverse(a->code_buffer, 16);
   for (s=STBI__ZFAST_BITS+1; ; ++s)
      if (k < z->maxcode[s])
         break;
   if (s == 16) return -1;
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   STBI_ASSERT(z->size[b] == s);
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

_inline static int _m3dstbi__zhuffman_decode(_m3dstbi__zbuf *a, _m3dstbi__zhuffman *z)
{
   int b,s;
   if (a->num_bits < 16) _m3dstbi__fill_bits(a);
   b = z->fast[a->code_buffer & STBI__ZFAST_MASK];
   if (b) {
      s = b >> 9;
      a->code_buffer >>= s;
      a->num_bits -= s;
      return b & 511;
   }
   return _m3dstbi__zhuffman_decode_slowpath(a, z);
}

static int _m3dstbi__zexpand(_m3dstbi__zbuf *z, char *zout, int n)
{
   char *q;
   int cur, limit, old_limit;
   z->zout = zout;
   if (!z->z_expandable) return _m3dstbi__err("output buffer limit","Corrupt PNG");
   cur   = (int) (z->zout     - z->zout_start);
   limit = old_limit = (int) (z->zout_end - z->zout_start);
   while (cur + n > limit)
      limit *= 2;
   q = (char *) STBI_REALLOC_SIZED(z->zout_start, old_limit, limit);
   STBI_NOTUSED(old_limit);
   if (q == NULL) return _m3dstbi__err("outofmem", "Out of memory");
   z->zout_start = q;
   z->zout       = q + cur;
   z->zout_end   = q + limit;
   return 1;
}

static int _m3dstbi__zlength_base[31] = {
   3,4,5,6,7,8,9,10,11,13,
   15,17,19,23,27,31,35,43,51,59,
   67,83,99,115,131,163,195,227,258,0,0 };

static int _m3dstbi__zlength_extra[31]=
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };

static int _m3dstbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};

static int _m3dstbi__zdist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int _m3dstbi__parse_huffman_block(_m3dstbi__zbuf *a)
{
   char *zout = a->zout;
   for(;;) {
      int z = _m3dstbi__zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return _m3dstbi__err("bad huffman code","Corrupt PNG");
         if (zout >= a->zout_end) {
            if (!_m3dstbi__zexpand(a, zout, 1)) return 0;
            zout = a->zout;
         }
         *zout++ = (char) z;
      } else {
         unsigned char *p;
         int len,dist;
         if (z == 256) {
            a->zout = zout;
            return 1;
         }
         z -= 257;
         len = _m3dstbi__zlength_base[z];
         if (_m3dstbi__zlength_extra[z]) len += _m3dstbi__zreceive(a, _m3dstbi__zlength_extra[z]);
         z = _m3dstbi__zhuffman_decode(a, &a->z_distance);
         if (z < 0) return _m3dstbi__err("bad huffman code","Corrupt PNG");
         dist = _m3dstbi__zdist_base[z];
         if (_m3dstbi__zdist_extra[z]) dist += _m3dstbi__zreceive(a, _m3dstbi__zdist_extra[z]);
         if (zout - a->zout_start < dist) return _m3dstbi__err("bad dist","Corrupt PNG");
         if (zout + len > a->zout_end) {
            if (!_m3dstbi__zexpand(a, zout, len)) return 0;
            zout = a->zout;
         }
         p = (unsigned char *) (zout - dist);
         if (dist == 1) {
            unsigned char v = *p;
            if (len) { do *zout++ = v; while (--len); }
         } else {
            if (len) { do *zout++ = *p++; while (--len); }
         }
      }
   }
}

static int _m3dstbi__compute_huffman_codes(_m3dstbi__zbuf *a)
{
   static unsigned char length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   _m3dstbi__zhuffman z_codelength;
   unsigned char lencodes[286+32+137];
   unsigned char codelength_sizes[19];
   int i,n;

   int hlit  = _m3dstbi__zreceive(a,5) + 257;
   int hdist = _m3dstbi__zreceive(a,5) + 1;
   int hclen = _m3dstbi__zreceive(a,4) + 4;
   int ntot  = hlit + hdist;

   memset(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = _m3dstbi__zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (unsigned char) s;
   }
   if (!_m3dstbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < ntot) {
      int c = _m3dstbi__zhuffman_decode(a, &z_codelength);
      if (c < 0 || c >= 19) return _m3dstbi__err("bad codelengths", "Corrupt PNG");
      if (c < 16)
         lencodes[n++] = (unsigned char) c;
      else {
         unsigned char fill = 0;
         if (c == 16) {
            c = _m3dstbi__zreceive(a,2)+3;
            if (n == 0) return _m3dstbi__err("bad codelengths", "Corrupt PNG");
            fill = lencodes[n-1];
         } else if (c == 17)
            c = _m3dstbi__zreceive(a,3)+3;
         else {
            STBI_ASSERT(c == 18);
            c = _m3dstbi__zreceive(a,7)+11;
         }
         if (ntot - n < c) return _m3dstbi__err("bad codelengths", "Corrupt PNG");
         memset(lencodes+n, fill, c);
         n += c;
      }
   }
   if (n != ntot) return _m3dstbi__err("bad codelengths","Corrupt PNG");
   if (!_m3dstbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!_m3dstbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

_inline static int _m3dstbi__parse_uncompressed_block(_m3dstbi__zbuf *a)
{
   unsigned char header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
      _m3dstbi__zreceive(a, a->num_bits & 7);
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (unsigned char) (a->code_buffer & 255);
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   STBI_ASSERT(a->num_bits == 0);
   while (k < 4)
      header[k++] = _m3dstbi__zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return _m3dstbi__err("zlib corrupt","Corrupt PNG");
   if (a->zbuffer + len > a->zbuffer_end) return _m3dstbi__err("read past buffer","Corrupt PNG");
   if (a->zout + len > a->zout_end)
      if (!_m3dstbi__zexpand(a, a->zout, len)) return 0;
   memcpy(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static int _m3dstbi__parse_zlib_header(_m3dstbi__zbuf *a)
{
   int cmf   = _m3dstbi__zget8(a);
   int cm    = cmf & 15;
   /* int cinfo = cmf >> 4; */
   int flg   = _m3dstbi__zget8(a);
   if ((cmf*256+flg) % 31 != 0) return _m3dstbi__err("bad zlib header","Corrupt PNG");
   if (flg & 32) return _m3dstbi__err("no preset dict","Corrupt PNG");
   if (cm != 8) return _m3dstbi__err("bad compression","Corrupt PNG");
   return 1;
}

static unsigned char _m3dstbi__zdefault_length[288], _m3dstbi__zdefault_distance[32];
static void _m3dstbi__init_zdefaults(void)
{
   int i;
   for (i=0; i <= 143; ++i)     _m3dstbi__zdefault_length[i]   = 8;
   for (   ; i <= 255; ++i)     _m3dstbi__zdefault_length[i]   = 9;
   for (   ; i <= 279; ++i)     _m3dstbi__zdefault_length[i]   = 7;
   for (   ; i <= 287; ++i)     _m3dstbi__zdefault_length[i]   = 8;

   for (i=0; i <=  31; ++i)     _m3dstbi__zdefault_distance[i] = 5;
}

static int _m3dstbi__parse_zlib(_m3dstbi__zbuf *a, int parse_header)
{
   int final, type;
   if (parse_header)
      if (!_m3dstbi__parse_zlib_header(a)) return 0;
   a->num_bits = 0;
   a->code_buffer = 0;
   do {
      final = _m3dstbi__zreceive(a,1);
      type = _m3dstbi__zreceive(a,2);
      if (type == 0) {
         if (!_m3dstbi__parse_uncompressed_block(a)) return 0;
      } else if (type == 3) {
         return 0;
      } else {
         if (type == 1) {
            if (!_m3dstbi__zbuild_huffman(&a->z_length  , _m3dstbi__zdefault_length  , 288)) return 0;
            if (!_m3dstbi__zbuild_huffman(&a->z_distance, _m3dstbi__zdefault_distance,  32)) return 0;
         } else {
            if (!_m3dstbi__compute_huffman_codes(a)) return 0;
         }
         if (!_m3dstbi__parse_huffman_block(a)) return 0;
      }
   } while (!final);
   return 1;
}

static int _m3dstbi__do_zlib(_m3dstbi__zbuf *a, char *obuf, int olen, int exp, int parse_header)
{
   a->zout_start = obuf;
   a->zout       = obuf;
   a->zout_end   = obuf + olen;
   a->z_expandable = exp;
   _m3dstbi__init_zdefaults();
   return _m3dstbi__parse_zlib(a, parse_header);
}

char *_m3dstbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header)
{
   _m3dstbi__zbuf a;
   char *p = (char *) _m3dstbi__malloc(initial_size);
   if (p == NULL) return NULL;
   a.zbuffer = (unsigned char *) buffer;
   a.zbuffer_end = (unsigned char *) buffer + len;
   if (_m3dstbi__do_zlib(&a, p, initial_size, 1, parse_header)) {
      if (outlen) *outlen = (int) (a.zout - a.zout_start);
      return a.zout_start;
   } else {
      STBI_FREE(a.zout_start);
      return NULL;
   }
}

typedef struct
{
   _m3dstbi__uint32 length;
   _m3dstbi__uint32 type;
} _m3dstbi__pngchunk;

static _m3dstbi__pngchunk _m3dstbi__get_chunk_header(_m3dstbi__context *s)
{
   _m3dstbi__pngchunk c;
   c.length = _m3dstbi__get32be(s);
   c.type   = _m3dstbi__get32be(s);
   return c;
}

_inline static int _m3dstbi__check_png_header(_m3dstbi__context *s)
{
   static unsigned char png_sig[8] = { 137,80,78,71,13,10,26,10 };
   int i;
   for (i=0; i < 8; ++i)
      if (_m3dstbi__get8(s) != png_sig[i]) return _m3dstbi__err("bad png sig","Not a PNG");
   return 1;
}

typedef struct
{
   _m3dstbi__context *s;
   unsigned char *idata, *expanded, *out;
   int depth;
} _m3dstbi__png;


enum {
   STBI__F_none=0,
   STBI__F_sub=1,
   STBI__F_up=2,
   STBI__F_avg=3,
   STBI__F_paeth=4,
   STBI__F_avg_first,
   STBI__F_paeth_first
};

static unsigned char first_row_filter[5] =
{
   STBI__F_none,
   STBI__F_sub,
   STBI__F_none,
   STBI__F_avg_first,
   STBI__F_paeth_first
};

static int _m3dstbi__paeth(int a, int b, int c)
{
   int p = a + b - c;
   int pa = abs(p-a);
   int pb = abs(p-b);
   int pc = abs(p-c);
   if (pa <= pb && pa <= pc) return a;
   if (pb <= pc) return b;
   return c;
}

static unsigned char _m3dstbi__depth_scale_table[9] = { 0, 0xff, 0x55, 0, 0x11, 0,0,0, 0x01 };

static int _m3dstbi__create_png_image_raw(_m3dstbi__png *a, unsigned char *raw, _m3dstbi__uint32 raw_len, int out_n, _m3dstbi__uint32 x, _m3dstbi__uint32 y, int depth, int color)
{
   int bytes = (depth == 16? 2 : 1);
   _m3dstbi__context *s = a->s;
   _m3dstbi__uint32 i,j,stride = x*out_n*bytes;
   _m3dstbi__uint32 img_len, img_width_bytes;
   int k;
   int img_n = s->img_n;

   int output_bytes = out_n*bytes;
   int filter_bytes = img_n*bytes;
   int width = x;

   STBI_ASSERT(out_n == s->img_n || out_n == s->img_n+1);
   a->out = (unsigned char *) _m3dstbi__malloc_mad3(x, y, output_bytes, 0);
   if (!a->out) return _m3dstbi__err("outofmem", "Out of memory");

   if (!_m3dstbi__mad3sizes_valid(img_n, x, depth, 7)) return _m3dstbi__err("too large", "Corrupt PNG");
   img_width_bytes = (((img_n * x * depth) + 7) >> 3);
   img_len = (img_width_bytes + 1) * y;
   if (s->img_x == x && s->img_y == y) {
      if (raw_len != img_len) return _m3dstbi__err("not enough pixels","Corrupt PNG");
   } else {
      if (raw_len < img_len) return _m3dstbi__err("not enough pixels","Corrupt PNG");
   }

   for (j=0; j < y; ++j) {
      unsigned char *cur = a->out + stride*j;
      unsigned char *prior = cur - stride;
      int filter = *raw++;

      if (filter > 4)
         return _m3dstbi__err("invalid filter","Corrupt PNG");

      if (depth < 8) {
         STBI_ASSERT(img_width_bytes <= x);
         cur += x*out_n - img_width_bytes;
         filter_bytes = 1;
         width = img_width_bytes;
      }
      prior = cur - stride;

      if (j == 0) filter = first_row_filter[filter];

      for (k=0; k < filter_bytes; ++k) {
         switch (filter) {
            case STBI__F_none       : cur[k] = raw[k]; break;
            case STBI__F_sub        : cur[k] = raw[k]; break;
            case STBI__F_up         : cur[k] = STBI__BYTECAST(raw[k] + prior[k]); break;
            case STBI__F_avg        : cur[k] = STBI__BYTECAST(raw[k] + (prior[k]>>1)); break;
            case STBI__F_paeth      : cur[k] = STBI__BYTECAST(raw[k] + _m3dstbi__paeth(0,prior[k],0)); break;
            case STBI__F_avg_first  : cur[k] = raw[k]; break;
            case STBI__F_paeth_first: cur[k] = raw[k]; break;
         }
      }

      if (depth == 8) {
         if (img_n != out_n)
            cur[img_n] = 255;
         raw += img_n;
         cur += out_n;
         prior += out_n;
      } else if (depth == 16) {
         if (img_n != out_n) {
            cur[filter_bytes]   = 255;
            cur[filter_bytes+1] = 255;
         }
         raw += filter_bytes;
         cur += output_bytes;
         prior += output_bytes;
      } else {
         raw += 1;
         cur += 1;
         prior += 1;
      }

      if (depth < 8 || img_n == out_n) {
         int nk = (width - 1)*filter_bytes;
         #define STBI__CASE(f) \
             case f:     \
                for (k=0; k < nk; ++k)
         switch (filter) {
            case STBI__F_none:         memcpy(cur, raw, nk); break;
            STBI__CASE(STBI__F_sub)          { cur[k] = STBI__BYTECAST(raw[k] + cur[k-filter_bytes]); } break;
            STBI__CASE(STBI__F_up)           { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg)          { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k-filter_bytes])>>1)); } break;
            STBI__CASE(STBI__F_paeth)        { cur[k] = STBI__BYTECAST(raw[k] + _m3dstbi__paeth(cur[k-filter_bytes],prior[k],prior[k-filter_bytes])); } break;
            STBI__CASE(STBI__F_avg_first)    { cur[k] = STBI__BYTECAST(raw[k] + (cur[k-filter_bytes] >> 1)); } break;
            STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + _m3dstbi__paeth(cur[k-filter_bytes],0,0)); } break;
         }
         #undef STBI__CASE
         raw += nk;
      } else {
         STBI_ASSERT(img_n+1 == out_n);
         #define STBI__CASE(f) \
             case f:     \
                for (i=x-1; i >= 1; --i, cur[filter_bytes]=255,raw+=filter_bytes,cur+=output_bytes,prior+=output_bytes) \
                   for (k=0; k < filter_bytes; ++k)
         switch (filter) {
            STBI__CASE(STBI__F_none)         { cur[k] = raw[k]; } break;
            STBI__CASE(STBI__F_sub)          { cur[k] = STBI__BYTECAST(raw[k] + cur[k- output_bytes]); } break;
            STBI__CASE(STBI__F_up)           { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
            STBI__CASE(STBI__F_avg)          { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k- output_bytes])>>1)); } break;
            STBI__CASE(STBI__F_paeth)        { cur[k] = STBI__BYTECAST(raw[k] + _m3dstbi__paeth(cur[k- output_bytes],prior[k],prior[k- output_bytes])); } break;
            STBI__CASE(STBI__F_avg_first)    { cur[k] = STBI__BYTECAST(raw[k] + (cur[k- output_bytes] >> 1)); } break;
            STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + _m3dstbi__paeth(cur[k- output_bytes],0,0)); } break;
         }
         #undef STBI__CASE

         if (depth == 16) {
            cur = a->out + stride*j;
            for (i=0; i < x; ++i,cur+=output_bytes) {
               cur[filter_bytes+1] = 255;
            }
         }
      }
   }

   if (depth < 8) {
      for (j=0; j < y; ++j) {
         unsigned char *cur = a->out + stride*j;
         unsigned char *in  = a->out + stride*j + x*out_n - img_width_bytes;
         unsigned char scale = (color == 0) ? _m3dstbi__depth_scale_table[depth] : 1;

         if (depth == 4) {
            for (k=x*img_n; k >= 2; k-=2, ++in) {
               *cur++ = scale * ((*in >> 4)       );
               *cur++ = scale * ((*in     ) & 0x0f);
            }
            if (k > 0) *cur++ = scale * ((*in >> 4)       );
         } else if (depth == 2) {
            for (k=x*img_n; k >= 4; k-=4, ++in) {
               *cur++ = scale * ((*in >> 6)       );
               *cur++ = scale * ((*in >> 4) & 0x03);
               *cur++ = scale * ((*in >> 2) & 0x03);
               *cur++ = scale * ((*in     ) & 0x03);
            }
            if (k > 0) *cur++ = scale * ((*in >> 6)       );
            if (k > 1) *cur++ = scale * ((*in >> 4) & 0x03);
            if (k > 2) *cur++ = scale * ((*in >> 2) & 0x03);
         } else if (depth == 1) {
            for (k=x*img_n; k >= 8; k-=8, ++in) {
               *cur++ = scale * ((*in >> 7)       );
               *cur++ = scale * ((*in >> 6) & 0x01);
               *cur++ = scale * ((*in >> 5) & 0x01);
               *cur++ = scale * ((*in >> 4) & 0x01);
               *cur++ = scale * ((*in >> 3) & 0x01);
               *cur++ = scale * ((*in >> 2) & 0x01);
               *cur++ = scale * ((*in >> 1) & 0x01);
               *cur++ = scale * ((*in     ) & 0x01);
            }
            if (k > 0) *cur++ = scale * ((*in >> 7)       );
            if (k > 1) *cur++ = scale * ((*in >> 6) & 0x01);
            if (k > 2) *cur++ = scale * ((*in >> 5) & 0x01);
            if (k > 3) *cur++ = scale * ((*in >> 4) & 0x01);
            if (k > 4) *cur++ = scale * ((*in >> 3) & 0x01);
            if (k > 5) *cur++ = scale * ((*in >> 2) & 0x01);
            if (k > 6) *cur++ = scale * ((*in >> 1) & 0x01);
         }
         if (img_n != out_n) {
            int q;
            cur = a->out + stride*j;
            if (img_n == 1) {
               for (q=x-1; q >= 0; --q) {
                  cur[q*2+1] = 255;
                  cur[q*2+0] = cur[q];
               }
            } else {
               STBI_ASSERT(img_n == 3);
               for (q=x-1; q >= 0; --q) {
                  cur[q*4+3] = 255;
                  cur[q*4+2] = cur[q*3+2];
                  cur[q*4+1] = cur[q*3+1];
                  cur[q*4+0] = cur[q*3+0];
               }
            }
         }
      }
   } else if (depth == 16) {
      unsigned char *cur = a->out;
      _m3dstbi__uint16 *cur16 = (_m3dstbi__uint16*)cur;

      for(i=0; i < x*y*out_n; ++i,cur16++,cur+=2) {
         *cur16 = (cur[0] << 8) | cur[1];
      }
   }

   return 1;
}

static int _m3dstbi__create_png_image(_m3dstbi__png *a, unsigned char *image_data, _m3dstbi__uint32 image_data_len, int out_n, int depth, int color, int interlaced)
{
   int bytes = (depth == 16 ? 2 : 1);
   int out_bytes = out_n * bytes;
   unsigned char *final;
   int p;
   if (!interlaced)
      return _m3dstbi__create_png_image_raw(a, image_data, image_data_len, out_n, a->s->img_x, a->s->img_y, depth, color);

   final = (unsigned char *) _m3dstbi__malloc_mad3(a->s->img_x, a->s->img_y, out_bytes, 0);
   for (p=0; p < 7; ++p) {
      int xorig[] = { 0,4,0,2,0,1,0 };
      int yorig[] = { 0,0,4,0,2,0,1 };
      int xspc[]  = { 8,8,4,4,2,2,1 };
      int yspc[]  = { 8,8,8,4,4,2,2 };
      int i,j,x,y;
      x = (a->s->img_x - xorig[p] + xspc[p]-1) / xspc[p];
      y = (a->s->img_y - yorig[p] + yspc[p]-1) / yspc[p];
      if (x && y) {
         _m3dstbi__uint32 img_len = ((((a->s->img_n * x * depth) + 7) >> 3) + 1) * y;
         if (!_m3dstbi__create_png_image_raw(a, image_data, image_data_len, out_n, x, y, depth, color)) {
            STBI_FREE(final);
            return 0;
         }
         for (j=0; j < y; ++j) {
            for (i=0; i < x; ++i) {
               int out_y = j*yspc[p]+yorig[p];
               int out_x = i*xspc[p]+xorig[p];
               memcpy(final + out_y*a->s->img_x*out_bytes + out_x*out_bytes,
                      a->out + (j*x+i)*out_bytes, out_bytes);
            }
         }
         STBI_FREE(a->out);
         image_data += img_len;
         image_data_len -= img_len;
      }
   }
   a->out = final;

   return 1;
}

static int _m3dstbi__compute_transparency(_m3dstbi__png *z, unsigned char tc[3], int out_n)
{
   _m3dstbi__context *s = z->s;
   _m3dstbi__uint32 i, pixel_count = s->img_x * s->img_y;
   unsigned char *p = z->out;

   STBI_ASSERT(out_n == 2 || out_n == 4);

   if (out_n == 2) {
      for (i=0; i < pixel_count; ++i) {
         p[1] = (p[0] == tc[0] ? 0 : 255);
         p += 2;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
            p[3] = 0;
         p += 4;
      }
   }
   return 1;
}

static int _m3dstbi__compute_transparency16(_m3dstbi__png *z, _m3dstbi__uint16 tc[3], int out_n)
{
   _m3dstbi__context *s = z->s;
   _m3dstbi__uint32 i, pixel_count = s->img_x * s->img_y;
   _m3dstbi__uint16 *p = (_m3dstbi__uint16*) z->out;

   STBI_ASSERT(out_n == 2 || out_n == 4);

   if (out_n == 2) {
      for (i = 0; i < pixel_count; ++i) {
         p[1] = (p[0] == tc[0] ? 0 : 65535);
         p += 2;
      }
   } else {
      for (i = 0; i < pixel_count; ++i) {
         if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
            p[3] = 0;
         p += 4;
      }
   }
   return 1;
}

static int _m3dstbi__expand_png_palette(_m3dstbi__png *a, unsigned char *palette, int len, int pal_img_n)
{
   _m3dstbi__uint32 i, pixel_count = a->s->img_x * a->s->img_y;
   unsigned char *p, *temp_out, *orig = a->out;

   p = (unsigned char *) _m3dstbi__malloc_mad2(pixel_count, pal_img_n, 0);
   if (p == NULL) return _m3dstbi__err("outofmem", "Out of memory");

   temp_out = p;

   if (pal_img_n == 3) {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p += 3;
      }
   } else {
      for (i=0; i < pixel_count; ++i) {
         int n = orig[i]*4;
         p[0] = palette[n  ];
         p[1] = palette[n+1];
         p[2] = palette[n+2];
         p[3] = palette[n+3];
         p += 4;
      }
   }
   STBI_FREE(a->out);
   a->out = temp_out;

   STBI_NOTUSED(len);

   return 1;
}

#define STBI__PNG_TYPE(a,b,c,d)  (((unsigned) (a) << 24) + ((unsigned) (b) << 16) + ((unsigned) (c) << 8) + (unsigned) (d))

static int _m3dstbi__parse_png_file(_m3dstbi__png *z, int scan, int req_comp)
{
   unsigned char palette[1024], pal_img_n=0;
   unsigned char has_trans=0, tc[3];
   _m3dstbi__uint16 tc16[3];
   _m3dstbi__uint32 ioff=0, idata_limit=0, i, pal_len=0;
   int first=1,k,interlace=0, color=0;
   _m3dstbi__context *s = z->s;

   z->expanded = NULL;
   z->idata = NULL;
   z->out = NULL;

   if (!_m3dstbi__check_png_header(s)) return 0;

   if (scan == STBI__SCAN_type) return 1;

   for (;;) {
      _m3dstbi__pngchunk c = _m3dstbi__get_chunk_header(s);
      switch (c.type) {
         case STBI__PNG_TYPE('C','g','B','I'):
            _m3dstbi__skip(s, c.length);
            break;
         case STBI__PNG_TYPE('I','H','D','R'): {
            int comp,filter;
            if (!first) return _m3dstbi__err("multiple IHDR","Corrupt PNG");
            first = 0;
            if (c.length != 13) return _m3dstbi__err("bad IHDR len","Corrupt PNG");
            s->img_x = _m3dstbi__get32be(s); if (s->img_x > (1 << 24)) return _m3dstbi__err("too large","Very large image (corrupt?)");
            s->img_y = _m3dstbi__get32be(s); if (s->img_y > (1 << 24)) return _m3dstbi__err("too large","Very large image (corrupt?)");
            z->depth = _m3dstbi__get8(s);  if (z->depth != 1 && z->depth != 2 && z->depth != 4 && z->depth != 8 && z->depth != 16)  return _m3dstbi__err("1/2/4/8/16-bit only","PNG not supported: 1/2/4/8/16-bit only");
            color = _m3dstbi__get8(s);  if (color > 6)         return _m3dstbi__err("bad ctype","Corrupt PNG");
            if (color == 3 && z->depth == 16)                  return _m3dstbi__err("bad ctype","Corrupt PNG");
            if (color == 3) pal_img_n = 3; else if (color & 1) return _m3dstbi__err("bad ctype","Corrupt PNG");
            comp  = _m3dstbi__get8(s);  if (comp) return _m3dstbi__err("bad comp method","Corrupt PNG");
            filter= _m3dstbi__get8(s);  if (filter) return _m3dstbi__err("bad filter method","Corrupt PNG");
            interlace = _m3dstbi__get8(s); if (interlace>1) return _m3dstbi__err("bad interlace method","Corrupt PNG");
            if (!s->img_x || !s->img_y) return _m3dstbi__err("0-pixel image","Corrupt PNG");
            if (!pal_img_n) {
               s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
               if ((1 << 30) / s->img_x / s->img_n < s->img_y) return _m3dstbi__err("too large", "Image too large to decode");
               if (scan == STBI__SCAN_header) return 1;
            } else {
               s->img_n = 1;
               if ((1 << 30) / s->img_x / 4 < s->img_y) return _m3dstbi__err("too large","Corrupt PNG");
            }
            break;
         }

         case STBI__PNG_TYPE('P','L','T','E'):  {
            if (first) return _m3dstbi__err("first not IHDR", "Corrupt PNG");
            if (c.length > 256*3) return _m3dstbi__err("invalid PLTE","Corrupt PNG");
            pal_len = c.length / 3;
            if (pal_len * 3 != c.length) return _m3dstbi__err("invalid PLTE","Corrupt PNG");
            for (i=0; i < pal_len; ++i) {
               palette[i*4+0] = _m3dstbi__get8(s);
               palette[i*4+1] = _m3dstbi__get8(s);
               palette[i*4+2] = _m3dstbi__get8(s);
               palette[i*4+3] = 255;
            }
            break;
         }

         case STBI__PNG_TYPE('t','R','N','S'): {
            if (first) return _m3dstbi__err("first not IHDR", "Corrupt PNG");
            if (z->idata) return _m3dstbi__err("tRNS after IDAT","Corrupt PNG");
            if (pal_img_n) {
               if (scan == STBI__SCAN_header) { s->img_n = 4; return 1; }
               if (pal_len == 0) return _m3dstbi__err("tRNS before PLTE","Corrupt PNG");
               if (c.length > pal_len) return _m3dstbi__err("bad tRNS len","Corrupt PNG");
               pal_img_n = 4;
               for (i=0; i < c.length; ++i)
                  palette[i*4+3] = _m3dstbi__get8(s);
            } else {
               if (!(s->img_n & 1)) return _m3dstbi__err("tRNS with alpha","Corrupt PNG");
               if (c.length != (_m3dstbi__uint32) s->img_n*2) return _m3dstbi__err("bad tRNS len","Corrupt PNG");
               has_trans = 1;
               if (z->depth == 16) {
                  for (k = 0; k < s->img_n; ++k) tc16[k] = (_m3dstbi__uint16)_m3dstbi__get16be(s);
               } else {
                  for (k = 0; k < s->img_n; ++k) tc[k] = (unsigned char)(_m3dstbi__get16be(s) & 255) * _m3dstbi__depth_scale_table[z->depth];
               }
            }
            break;
         }

         case STBI__PNG_TYPE('I','D','A','T'): {
            if (first) return _m3dstbi__err("first not IHDR", "Corrupt PNG");
            if (pal_img_n && !pal_len) return _m3dstbi__err("no PLTE","Corrupt PNG");
            if (scan == STBI__SCAN_header) { s->img_n = pal_img_n; return 1; }
            if ((int)(ioff + c.length) < (int)ioff) return 0;
            if (ioff + c.length > idata_limit) {
               _m3dstbi__uint32 idata_limit_old = idata_limit;
               unsigned char *p;
               if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
               while (ioff + c.length > idata_limit)
                  idata_limit *= 2;
               STBI_NOTUSED(idata_limit_old);
               p = (unsigned char *) STBI_REALLOC_SIZED(z->idata, idata_limit_old, idata_limit); if (p == NULL) return _m3dstbi__err("outofmem", "Out of memory");
               z->idata = p;
            }
            if (!_m3dstbi__getn(s, z->idata+ioff,c.length)) return _m3dstbi__err("outofdata","Corrupt PNG");
            ioff += c.length;
            break;
         }

         case STBI__PNG_TYPE('I','E','N','D'): {
            _m3dstbi__uint32 raw_len, bpl;
            if (first) return _m3dstbi__err("first not IHDR", "Corrupt PNG");
            if (scan != STBI__SCAN_load) return 1;
            if (z->idata == NULL) return _m3dstbi__err("no IDAT","Corrupt PNG");
            bpl = (s->img_x * z->depth + 7) / 8;
            raw_len = bpl * s->img_y * s->img_n /* pixels */ + s->img_y /* filter mode per row */;
            z->expanded = (unsigned char *) _m3dstbi_zlib_decode_malloc_guesssize_headerflag((char *) z->idata, ioff, raw_len, (int *) &raw_len, 1);
            if (z->expanded == NULL) return 0;
            STBI_FREE(z->idata); z->idata = NULL;
            if ((req_comp == s->img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
               s->img_out_n = s->img_n+1;
            else
               s->img_out_n = s->img_n;
            if (!_m3dstbi__create_png_image(z, z->expanded, raw_len, s->img_out_n, z->depth, color, interlace)) return 0;
            if (has_trans) {
               if (z->depth == 16) {
                  if (!_m3dstbi__compute_transparency16(z, tc16, s->img_out_n)) return 0;
               } else {
                  if (!_m3dstbi__compute_transparency(z, tc, s->img_out_n)) return 0;
               }
            }
            if (pal_img_n) {
               s->img_n = pal_img_n;
               s->img_out_n = pal_img_n;
               if (req_comp >= 3) s->img_out_n = req_comp;
               if (!_m3dstbi__expand_png_palette(z, palette, pal_len, s->img_out_n))
                  return 0;
            } else if (has_trans) {
               ++s->img_n;
            }
            STBI_FREE(z->expanded); z->expanded = NULL;
            return 1;
         }

         default:
            if (first) return _m3dstbi__err("first not IHDR", "Corrupt PNG");
            if ((c.type & (1 << 29)) == 0) {
               return _m3dstbi__err("invalid_chunk", "PNG not supported: unknown PNG chunk type");
            }
            _m3dstbi__skip(s, c.length);
            break;
      }
      _m3dstbi__get32be(s);
   }
}

static void *_m3dstbi__do_png(_m3dstbi__png *p, int *x, int *y, int *n, int req_comp, _m3dstbi__result_info *ri)
{
   void *result=NULL;
   if (req_comp < 0 || req_comp > 4) { _m3dstbi__err("bad req_comp", "Internal error"); return NULL; }
   if (_m3dstbi__parse_png_file(p, STBI__SCAN_load, req_comp)) {
      if (p->depth < 8)
         ri->bits_per_channel = 8;
      else
         ri->bits_per_channel = p->depth;
      result = p->out;
      p->out = NULL;
      if (req_comp && req_comp != p->s->img_out_n) {
         if (ri->bits_per_channel == 8)
            result = _m3dstbi__convert_format((unsigned char *) result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
         else
            result = _m3dstbi__convert_format16((_m3dstbi__uint16 *) result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
         p->s->img_out_n = req_comp;
         if (result == NULL) return result;
      }
      *x = p->s->img_x;
      *y = p->s->img_y;
      if (n) *n = p->s->img_n;
   }
   STBI_FREE(p->out);      p->out      = NULL;
   STBI_FREE(p->expanded); p->expanded = NULL;
   STBI_FREE(p->idata);    p->idata    = NULL;

   return result;
}

static void *_m3dstbi__png_load(_m3dstbi__context *s, int *x, int *y, int *comp, int req_comp, _m3dstbi__result_info *ri)
{
   _m3dstbi__png p;
   p.s = s;
   return _m3dstbi__do_png(&p, x,y,comp,req_comp, ri);
}
#define stbi__context _m3dstbi__context
#define stbi__result_info _m3dstbi__result_info
#define stbi__png_load _m3dstbi__png_load
#define stbi_zlib_decode_malloc_guesssize_headerflag _m3dstbi_zlib_decode_malloc_guesssize_headerflag
#endif
#if !defined(M3D_NOIMPORTER) && defined(STBI_INCLUDE_STB_IMAGE_H) && !defined(STB_IMAGE_IMPLEMENTATION)
#error "stb_image.h included without STB_IMAGE_IMPLEMENTATION. Sorry, we need some stuff defined inside the ifguard for proper integration"
#endif

#if defined(M3D_EXPORTER) && !defined(INCLUDE_STB_IMAGE_WRITE_H)
/* zlib_compressor from

   stb_image_write - v1.13 - public domain - http://nothings.org/stb/stb_image_write.h
*/
typedef unsigned char _m3dstbiw__uc;
typedef unsigned short _m3dstbiw__us;

typedef uint16_t _m3dstbiw__uint16;
typedef int16_t  _m3dstbiw__int16;
typedef uint32_t _m3dstbiw__uint32;
typedef int32_t  _m3dstbiw__int32;

#define STBIW_MALLOC(s)     M3D_MALLOC(s)
#define STBIW_REALLOC(p,ns) M3D_REALLOC(p,ns)
#define STBIW_REALLOC_SIZED(p,oldsz,newsz) STBIW_REALLOC(p,newsz)
#define STBIW_FREE          M3D_FREE
#define STBIW_MEMMOVE       memmove
#define STBIW_UCHAR         (uint8_t)
#define STBIW_ASSERT(x)
#define _m3dstbiw___sbraw(a)     ((int *) (a) - 2)
#define _m3dstbiw___sbm(a)       _m3dstbiw___sbraw(a)[0]
#define _m3dstbiw___sbn(a)       _m3dstbiw___sbraw(a)[1]

#define _m3dstbiw___sbneedgrow(a,n)  ((a)==0 || _m3dstbiw___sbn(a)+n >= _m3dstbiw___sbm(a))
#define _m3dstbiw___sbmaybegrow(a,n) (_m3dstbiw___sbneedgrow(a,(n)) ? _m3dstbiw___sbgrow(a,n) : 0)
#define _m3dstbiw___sbgrow(a,n)  _m3dstbiw___sbgrowf((void **) &(a), (n), sizeof(*(a)))

#define _m3dstbiw___sbpush(a, v)      (_m3dstbiw___sbmaybegrow(a,1), (a)[_m3dstbiw___sbn(a)++] = (v))
#define _m3dstbiw___sbcount(a)        ((a) ? _m3dstbiw___sbn(a) : 0)
#define _m3dstbiw___sbfree(a)         ((a) ? STBIW_FREE(_m3dstbiw___sbraw(a)),0 : 0)

static void *_m3dstbiw___sbgrowf(void **arr, int increment, int itemsize)
{
   int m = *arr ? 2*_m3dstbiw___sbm(*arr)+increment : increment+1;
   void *p = STBIW_REALLOC_SIZED(*arr ? _m3dstbiw___sbraw(*arr) : 0, *arr ? (_m3dstbiw___sbm(*arr)*itemsize + sizeof(int)*2) : 0, itemsize * m + sizeof(int)*2);
   STBIW_ASSERT(p);
   if (p) {
      if (!*arr) ((int *) p)[1] = 0;
      *arr = (void *) ((int *) p + 2);
      _m3dstbiw___sbm(*arr) = m;
   }
   return *arr;
}

static unsigned char *_m3dstbiw___zlib_flushf(unsigned char *data, unsigned int *bitbuffer, int *bitcount)
{
   while (*bitcount >= 8) {
      _m3dstbiw___sbpush(data, STBIW_UCHAR(*bitbuffer));
      *bitbuffer >>= 8;
      *bitcount -= 8;
   }
   return data;
}

static int _m3dstbiw___zlib_bitrev(int code, int codebits)
{
   int res=0;
   while (codebits--) {
      res = (res << 1) | (code & 1);
      code >>= 1;
   }
   return res;
}

static unsigned int _m3dstbiw___zlib_countm(unsigned char *a, unsigned char *b, int limit)
{
   int i;
   for (i=0; i < limit && i < 258; ++i)
      if (a[i] != b[i]) break;
   return i;
}

static unsigned int _m3dstbiw___zhash(unsigned char *data)
{
   _m3dstbiw__uint32 hash = data[0] + (data[1] << 8) + (data[2] << 16);
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

#define _m3dstbiw___zlib_flush() (out = _m3dstbiw___zlib_flushf(out, &bitbuf, &bitcount))
#define _m3dstbiw___zlib_add(code,codebits) \
      (bitbuf |= (code) << bitcount, bitcount += (codebits), _m3dstbiw___zlib_flush())
#define _m3dstbiw___zlib_huffa(b,c)  _m3dstbiw___zlib_add(_m3dstbiw___zlib_bitrev(b,c),c)
#define _m3dstbiw___zlib_huff1(n)  _m3dstbiw___zlib_huffa(0x30 + (n), 8)
#define _m3dstbiw___zlib_huff2(n)  _m3dstbiw___zlib_huffa(0x190 + (n)-144, 9)
#define _m3dstbiw___zlib_huff3(n)  _m3dstbiw___zlib_huffa(0 + (n)-256,7)
#define _m3dstbiw___zlib_huff4(n)  _m3dstbiw___zlib_huffa(0xc0 + (n)-280,8)
#define _m3dstbiw___zlib_huff(n)  ((n) <= 143 ? _m3dstbiw___zlib_huff1(n) : (n) <= 255 ? _m3dstbiw___zlib_huff2(n) : (n) <= 279 ? _m3dstbiw___zlib_huff3(n) : _m3dstbiw___zlib_huff4(n))
#define _m3dstbiw___zlib_huffb(n) ((n) <= 143 ? _m3dstbiw___zlib_huff1(n) : _m3dstbiw___zlib_huff2(n))

#define _m3dstbiw___ZHASH   16384

unsigned char * _m3dstbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality)
{
   static unsigned short lengthc[] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258, 259 };
   static unsigned char  lengtheb[]= { 0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0 };
   static unsigned short distc[]   = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577, 32768 };
   static unsigned char  disteb[]  = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13 };
   unsigned int bitbuf=0;
   int i,j, bitcount=0;
   unsigned char *out = NULL;
   unsigned char ***hash_table = (unsigned char***) STBIW_MALLOC(_m3dstbiw___ZHASH * sizeof(char**));
   if (hash_table == NULL)
      return NULL;
   if (quality < 5) quality = 5;

   _m3dstbiw___sbpush(out, 0x78);
   _m3dstbiw___sbpush(out, 0x5e);
   _m3dstbiw___zlib_add(1,1);
   _m3dstbiw___zlib_add(1,2);

   for (i=0; i < _m3dstbiw___ZHASH; ++i)
      hash_table[i] = NULL;

   i=0;
   while (i < data_len-3) {
      int h = _m3dstbiw___zhash(data+i)&(_m3dstbiw___ZHASH-1), best=3;
      unsigned char *bestloc = 0;
      unsigned char **hlist = hash_table[h];
      int n = _m3dstbiw___sbcount(hlist);
      for (j=0; j < n; ++j) {
         if (hlist[j]-data > i-32768) {
            int d = _m3dstbiw___zlib_countm(hlist[j], data+i, data_len-i);
            if (d >= best) best=d,bestloc=hlist[j];
         }
      }
      if (hash_table[h] && _m3dstbiw___sbn(hash_table[h]) == 2*quality) {
         STBIW_MEMMOVE(hash_table[h], hash_table[h]+quality, sizeof(hash_table[h][0])*quality);
         _m3dstbiw___sbn(hash_table[h]) = quality;
      }
      _m3dstbiw___sbpush(hash_table[h],data+i);

      if (bestloc) {
         h = _m3dstbiw___zhash(data+i+1)&(_m3dstbiw___ZHASH-1);
         hlist = hash_table[h];
         n = _m3dstbiw___sbcount(hlist);
         for (j=0; j < n; ++j) {
            if (hlist[j]-data > i-32767) {
               int e = _m3dstbiw___zlib_countm(hlist[j], data+i+1, data_len-i-1);
               if (e > best) {
                  bestloc = NULL;
                  break;
               }
            }
         }
      }

      if (bestloc) {
         int d = (int) (data+i - bestloc);
         STBIW_ASSERT(d <= 32767 && best <= 258);
         for (j=0; best > lengthc[j+1]-1; ++j);
         _m3dstbiw___zlib_huff(j+257);
         if (lengtheb[j]) _m3dstbiw___zlib_add(best - lengthc[j], lengtheb[j]);
         for (j=0; d > distc[j+1]-1; ++j);
         _m3dstbiw___zlib_add(_m3dstbiw___zlib_bitrev(j,5),5);
         if (disteb[j]) _m3dstbiw___zlib_add(d - distc[j], disteb[j]);
         i += best;
      } else {
         _m3dstbiw___zlib_huffb(data[i]);
         ++i;
      }
   }
   for (;i < data_len; ++i)
      _m3dstbiw___zlib_huffb(data[i]);
   _m3dstbiw___zlib_huff(256);
   while (bitcount)
      _m3dstbiw___zlib_add(0,1);

   for (i=0; i < _m3dstbiw___ZHASH; ++i)
      (void) _m3dstbiw___sbfree(hash_table[i]);
   STBIW_FREE(hash_table);

   {
      unsigned int s1=1, s2=0;
      int blocklen = (int) (data_len % 5552);
      j=0;
      while (j < data_len) {
         for (i=0; i < blocklen; ++i) s1 += data[j+i], s2 += s1;
         s1 %= 65521, s2 %= 65521;
         j += blocklen;
         blocklen = 5552;
      }
      _m3dstbiw___sbpush(out, STBIW_UCHAR(s2 >> 8));
      _m3dstbiw___sbpush(out, STBIW_UCHAR(s2));
      _m3dstbiw___sbpush(out, STBIW_UCHAR(s1 >> 8));
      _m3dstbiw___sbpush(out, STBIW_UCHAR(s1));
   }
   *out_len = _m3dstbiw___sbn(out);
   STBIW_MEMMOVE(_m3dstbiw___sbraw(out), out, *out_len);
   return (unsigned char *) _m3dstbiw___sbraw(out);
}
#define stbi_zlib_compress _m3dstbi_zlib_compress
#else
unsigned char * _m3dstbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality);
#endif

#define M3D_CHUNKMAGIC(m, a,b,c,d) ((m)[0]==(a) && (m)[1]==(b) && (m)[2]==(c) && (m)[3]==(d))

#ifdef M3D_ASCII
#include <stdio.h>          /* get sprintf */
#include <locale.h>         /* sprintf and strtod cares about number locale */
#endif
#ifdef M3D_PROFILING
#include <sys/time.h>
#endif

#if !defined(M3D_NOIMPORTER) && defined(M3D_ASCII)
/* helper functions for the ASCII parser */
static char *_m3d_findarg(char *s) {
    while(s && *s && *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n') s++;
    while(s && *s && (*s == ' ' || *s == '\t')) s++;
    return s;
}
static char *_m3d_findnl(char *s) {
    while(s && *s && *s != '\r' && *s != '\n') s++;
    if(*s == '\r') s++;
    if(*s == '\n') s++;
    return s;
}
static char *_m3d_gethex(char *s, uint32_t *ret)
{
    if(*s == '#') s++;
    *ret = 0;
    for(; *s; s++) {
        if(*s >= '0' && *s <= '9') {      *ret <<= 4; *ret += (uint32_t)(*s-'0'); }
        else if(*s >= 'a' && *s <= 'f') { *ret <<= 4; *ret += (uint32_t)(*s-'a'+10); }
        else if(*s >= 'A' && *s <= 'F') { *ret <<= 4; *ret += (uint32_t)(*s-'A'+10); }
        else break;
    }
    return _m3d_findarg(s);
}
static char *_m3d_getint(char *s, uint32_t *ret)
{
    char *e = s;
    if(!s || !*s || *s == '\r' || *s == '\n') return s;
    for(; *e >= '0' && *e <= '9'; e++);
    *ret = atoi(s);
    return e;
}
static char *_m3d_getfloat(char *s, M3D_FLOAT *ret)
{
    char *e = s;
    if(!s || !*s || *s == '\r' || *s == '\n') return s;
    for(; *e == '-' || *e == '+' || *e == '.' || (*e >= '0' && *e <= '9') || *e == 'e' || *e == 'E'; e++);
    *ret = (M3D_FLOAT)strtod(s, NULL);
    return _m3d_findarg(e);
}
#endif
#if !defined(M3D_NODUP) && (!defined(M3D_NOIMPORTER) || defined(M3D_ASCII) || defined(M3D_EXPORTER))
/* helper function to create safe strings */
char *_m3d_safestr(char *in, int morelines)
{
    char *out, *o, *i = in;
    int l;
    if(!in || !*in) {
        out = (char*)M3D_MALLOC(1);
        if(!out) return NULL;
        out[0] =0;
    } else {
        for(o = in, l = 0; *o && ((morelines & 1) || (*o != '\r' && *o != '\n')) && l < 256; o++, l++);
        out = o = (char*)M3D_MALLOC(l+1);
        if(!out) return NULL;
        while(*i == ' ' || *i == '\t' || *i == '\r' || (morelines && *i == '\n')) i++;
        for(; *i && (morelines || (*i != '\r' && *i != '\n')); i++) {
            if(*i == '\r') continue;
            if(*i == '\n') {
                if(morelines >= 3 && o > out && *(o-1) == '\n') break;
                if(i > in && *(i-1) == '\n') continue;
                if(morelines & 1) {
                    if(morelines == 1) *o++ = '\r';
                    *o++ = '\n';
                } else
                    break;
            } else
            if(*i == ' ' || *i == '\t') {
                *o++ = morelines? ' ' : '_';
            } else
                *o++ = !morelines && (*i == '/' || *i == '\\') ? '_' : *i;
        }
        for(; o > out && (*(o-1) == ' ' || *(o-1) == '\t' || *(o-1) == '\r' || *(o-1) == '\n'); o--);
        *o = 0;
        out = (char*)M3D_REALLOC(out, (uintptr_t)o - (uintptr_t)out + 1);
    }
    return out;
}
#endif
#ifndef M3D_NOIMPORTER
/* helper function to load and decode/generate a texture */
M3D_INDEX _m3d_gettx(m3d_t *model, m3dread_t readfilecb, m3dfree_t freecb, char *fn)
{
    unsigned int i, len = 0;
    unsigned char *buff = NULL;
    char *fn2;
    unsigned int w, h;
    stbi__context s;
    stbi__result_info ri;

    /* do we have loaded this texture already? */
    for(i = 0; i < model->numtexture; i++)
        if(!strcmp(fn, model->texture[i].name)) return i;
    /* see if it's inlined in the model */
    if(model->inlined) {
        for(i = 0; i < model->numinlined; i++)
            if(!strcmp(fn, model->inlined[i].name)) {
                buff = model->inlined[i].data;
                len = model->inlined[i].length;
                freecb = NULL;
                break;
            }
    }
    /* try to load from external source */
    if(!buff && readfilecb) {
        i = (unsigned int)strlen(fn);
        if(i < 5 || fn[i - 4] != '.') {
            fn2 = (char*)M3D_MALLOC(i + 5);
            if(!fn2) { model->errcode = M3D_ERR_ALLOC; return M3D_UNDEF; }
            memcpy(fn2, fn, i);
            memcpy(fn2+i, ".png", 5);
            buff = (*readfilecb)(fn2, &len);
            M3D_FREE(fn2);
        }
        if(!buff) {
            buff = (*readfilecb)(fn, &len);
            if(!buff) return M3D_UNDEF;
        }
    }
    /* add to textures array */
    i = model->numtexture++;
    model->texture = (m3dtx_t*)M3D_REALLOC(model->texture, model->numtexture * sizeof(m3dtx_t));
    if(!model->texture) {
        if(buff && freecb) (*freecb)(buff);
        model->errcode = M3D_ERR_ALLOC;
        return M3D_UNDEF;
    }
    model->texture[i].name = fn;
    model->texture[i].w = model->texture[i].h = 0; model->texture[i].d = NULL;
    if(buff) {
        if(buff[0] == 0x89 && buff[1] == 'P' && buff[2] == 'N' && buff[3] == 'G') {
            s.read_from_callbacks = 0;
            s.img_buffer = s.img_buffer_original = (unsigned char *) buff;
            s.img_buffer_end = s.img_buffer_original_end = (unsigned char *) buff+len;
            /* don't use model->texture[i].w directly, it's a uint16_t */
            w = h = len = 0;
            ri.bits_per_channel = 8;
            model->texture[i].d = (uint8_t*)stbi__png_load(&s, (int*)&w, (int*)&h, (int*)&len, 0, &ri);
            model->texture[i].w = w;
            model->texture[i].h = h;
            model->texture[i].f = (uint8_t)len;
        } else {
#ifdef M3D_TX_INTERP
            if((model->errcode = M3D_TX_INTERP(fn, buff, len, &model->texture[i])) != M3D_SUCCESS) {
                M3D_LOG("Unable to generate texture");
                M3D_LOG(fn);
            }
#else
            M3D_LOG("Unimplemented interpreter");
            M3D_LOG(fn);
#endif
        }
        if(freecb) (*freecb)(buff);
    }
    if(!model->texture[i].d)
        model->errcode = M3D_ERR_UNKIMG;
    return i;
}

/* helper function to load and generate a procedural surface */
void _m3d_getpr(m3d_t *model, _unused m3dread_t readfilecb, _unused  m3dfree_t freecb, _unused char *fn)
{
#ifdef M3D_PR_INTERP
    unsigned int i, len = 0;
    unsigned char *buff = readfilecb ? (*readfilecb)(fn, &len) : NULL;

    if(!buff && model->inlined) {
        for(i = 0; i < model->numinlined; i++)
            if(!strcmp(fn, model->inlined[i].name)) {
                buff = model->inlined[i].data;
                len = model->inlined[i].length;
                freecb = NULL;
                break;
            }
    }
    if(!buff || !len || (model->errcode = M3D_PR_INTERP(fn, buff, len, model)) != M3D_SUCCESS) {
        M3D_LOG("Unable to generate procedural surface");
        M3D_LOG(fn);
        model->errcode = M3D_ERR_UNKIMG;
    }
    if(freecb && buff) (*freecb)(buff);
#else
    (void)readfilecb;
    (void)freecb;
    (void)fn;
    M3D_LOG("Unimplemented interpreter");
    M3D_LOG(fn);
    model->errcode = M3D_ERR_UNIMPL;
#endif
}
/* helpers to read indices from data stream */
#define M3D_GETSTR(x) do{offs=0;data=_m3d_getidx(data,model->si_s,&offs);x=offs?((char*)model->raw+16+offs):NULL;}while(0)
_inline static unsigned char *_m3d_getidx(unsigned char *data, char type, M3D_INDEX *idx)
{
    switch(type) {
        case 1: *idx = data[0] > 253 ? (int8_t)data[0] : data[0]; data++; break;
        case 2: *idx = *((uint16_t*)data) > 65533 ? *((int16_t*)data) : *((uint16_t*)data); data += 2; break;
        case 4: *idx = *((int32_t*)data); data += 4; break;
    }
    return data;
}

#ifndef M3D_NOANIMATION
/* multiply 4 x 4 matrices. Do not use float *r[16] as argument, because some compilers misinterpret that as
 * 16 pointers each pointing to a float, but we need a single pointer to 16 floats. */
void _m3d_mul(M3D_FLOAT *r, M3D_FLOAT *a, M3D_FLOAT *b)
{
    r[ 0] = b[ 0] * a[ 0] + b[ 4] * a[ 1] + b[ 8] * a[ 2] + b[12] * a[ 3];
    r[ 1] = b[ 1] * a[ 0] + b[ 5] * a[ 1] + b[ 9] * a[ 2] + b[13] * a[ 3];
    r[ 2] = b[ 2] * a[ 0] + b[ 6] * a[ 1] + b[10] * a[ 2] + b[14] * a[ 3];
    r[ 3] = b[ 3] * a[ 0] + b[ 7] * a[ 1] + b[11] * a[ 2] + b[15] * a[ 3];
    r[ 4] = b[ 0] * a[ 4] + b[ 4] * a[ 5] + b[ 8] * a[ 6] + b[12] * a[ 7];
    r[ 5] = b[ 1] * a[ 4] + b[ 5] * a[ 5] + b[ 9] * a[ 6] + b[13] * a[ 7];
    r[ 6] = b[ 2] * a[ 4] + b[ 6] * a[ 5] + b[10] * a[ 6] + b[14] * a[ 7];
    r[ 7] = b[ 3] * a[ 4] + b[ 7] * a[ 5] + b[11] * a[ 6] + b[15] * a[ 7];
    r[ 8] = b[ 0] * a[ 8] + b[ 4] * a[ 9] + b[ 8] * a[10] + b[12] * a[11];
    r[ 9] = b[ 1] * a[ 8] + b[ 5] * a[ 9] + b[ 9] * a[10] + b[13] * a[11];
    r[10] = b[ 2] * a[ 8] + b[ 6] * a[ 9] + b[10] * a[10] + b[14] * a[11];
    r[11] = b[ 3] * a[ 8] + b[ 7] * a[ 9] + b[11] * a[10] + b[15] * a[11];
    r[12] = b[ 0] * a[12] + b[ 4] * a[13] + b[ 8] * a[14] + b[12] * a[15];
    r[13] = b[ 1] * a[12] + b[ 5] * a[13] + b[ 9] * a[14] + b[13] * a[15];
    r[14] = b[ 2] * a[12] + b[ 6] * a[13] + b[10] * a[14] + b[14] * a[15];
    r[15] = b[ 3] * a[12] + b[ 7] * a[13] + b[11] * a[14] + b[15] * a[15];
}
/* calculate 4 x 4 matrix inverse */
void _m3d_inv(M3D_FLOAT *m)
{
    M3D_FLOAT r[16];
    M3D_FLOAT det =
          m[ 0]*m[ 5]*m[10]*m[15] - m[ 0]*m[ 5]*m[11]*m[14] + m[ 0]*m[ 6]*m[11]*m[13] - m[ 0]*m[ 6]*m[ 9]*m[15]
        + m[ 0]*m[ 7]*m[ 9]*m[14] - m[ 0]*m[ 7]*m[10]*m[13] - m[ 1]*m[ 6]*m[11]*m[12] + m[ 1]*m[ 6]*m[ 8]*m[15]
        - m[ 1]*m[ 7]*m[ 8]*m[14] + m[ 1]*m[ 7]*m[10]*m[12] - m[ 1]*m[ 4]*m[10]*m[15] + m[ 1]*m[ 4]*m[11]*m[14]
        + m[ 2]*m[ 7]*m[ 8]*m[13] - m[ 2]*m[ 7]*m[ 9]*m[12] + m[ 2]*m[ 4]*m[ 9]*m[15] - m[ 2]*m[ 4]*m[11]*m[13]
        + m[ 2]*m[ 5]*m[11]*m[12] - m[ 2]*m[ 5]*m[ 8]*m[15] - m[ 3]*m[ 4]*m[ 9]*m[14] + m[ 3]*m[ 4]*m[10]*m[13]
        - m[ 3]*m[ 5]*m[10]*m[12] + m[ 3]*m[ 5]*m[ 8]*m[14] - m[ 3]*m[ 6]*m[ 8]*m[13] + m[ 3]*m[ 6]*m[ 9]*m[12];
    if(det == (M3D_FLOAT)0.0 || det == (M3D_FLOAT)-0.0) det = (M3D_FLOAT)1.0; else det = (M3D_FLOAT)1.0 / det;
    r[ 0] = det *(m[ 5]*(m[10]*m[15] - m[11]*m[14]) + m[ 6]*(m[11]*m[13] - m[ 9]*m[15]) + m[ 7]*(m[ 9]*m[14] - m[10]*m[13]));
    r[ 1] = -det*(m[ 1]*(m[10]*m[15] - m[11]*m[14]) + m[ 2]*(m[11]*m[13] - m[ 9]*m[15]) + m[ 3]*(m[ 9]*m[14] - m[10]*m[13]));
    r[ 2] = det *(m[ 1]*(m[ 6]*m[15] - m[ 7]*m[14]) + m[ 2]*(m[ 7]*m[13] - m[ 5]*m[15]) + m[ 3]*(m[ 5]*m[14] - m[ 6]*m[13]));
    r[ 3] = -det*(m[ 1]*(m[ 6]*m[11] - m[ 7]*m[10]) + m[ 2]*(m[ 7]*m[ 9] - m[ 5]*m[11]) + m[ 3]*(m[ 5]*m[10] - m[ 6]*m[ 9]));
    r[ 4] = -det*(m[ 4]*(m[10]*m[15] - m[11]*m[14]) + m[ 6]*(m[11]*m[12] - m[ 8]*m[15]) + m[ 7]*(m[ 8]*m[14] - m[10]*m[12]));
    r[ 5] = det *(m[ 0]*(m[10]*m[15] - m[11]*m[14]) + m[ 2]*(m[11]*m[12] - m[ 8]*m[15]) + m[ 3]*(m[ 8]*m[14] - m[10]*m[12]));
    r[ 6] = -det*(m[ 0]*(m[ 6]*m[15] - m[ 7]*m[14]) + m[ 2]*(m[ 7]*m[12] - m[ 4]*m[15]) + m[ 3]*(m[ 4]*m[14] - m[ 6]*m[12]));
    r[ 7] = det *(m[ 0]*(m[ 6]*m[11] - m[ 7]*m[10]) + m[ 2]*(m[ 7]*m[ 8] - m[ 4]*m[11]) + m[ 3]*(m[ 4]*m[10] - m[ 6]*m[ 8]));
    r[ 8] = det *(m[ 4]*(m[ 9]*m[15] - m[11]*m[13]) + m[ 5]*(m[11]*m[12] - m[ 8]*m[15]) + m[ 7]*(m[ 8]*m[13] - m[ 9]*m[12]));
    r[ 9] = -det*(m[ 0]*(m[ 9]*m[15] - m[11]*m[13]) + m[ 1]*(m[11]*m[12] - m[ 8]*m[15]) + m[ 3]*(m[ 8]*m[13] - m[ 9]*m[12]));
    r[10] = det *(m[ 0]*(m[ 5]*m[15] - m[ 7]*m[13]) + m[ 1]*(m[ 7]*m[12] - m[ 4]*m[15]) + m[ 3]*(m[ 4]*m[13] - m[ 5]*m[12]));
    r[11] = -det*(m[ 0]*(m[ 5]*m[11] - m[ 7]*m[ 9]) + m[ 1]*(m[ 7]*m[ 8] - m[ 4]*m[11]) + m[ 3]*(m[ 4]*m[ 9] - m[ 5]*m[ 8]));
    r[12] = -det*(m[ 4]*(m[ 9]*m[14] - m[10]*m[13]) + m[ 5]*(m[10]*m[12] - m[ 8]*m[14]) + m[ 6]*(m[ 8]*m[13] - m[ 9]*m[12]));
    r[13] = det *(m[ 0]*(m[ 9]*m[14] - m[10]*m[13]) + m[ 1]*(m[10]*m[12] - m[ 8]*m[14]) + m[ 2]*(m[ 8]*m[13] - m[ 9]*m[12]));
    r[14] = -det*(m[ 0]*(m[ 5]*m[14] - m[ 6]*m[13]) + m[ 1]*(m[ 6]*m[12] - m[ 4]*m[14]) + m[ 2]*(m[ 4]*m[13] - m[ 5]*m[12]));
    r[15] = det *(m[ 0]*(m[ 5]*m[10] - m[ 6]*m[ 9]) + m[ 1]*(m[ 6]*m[ 8] - m[ 4]*m[10]) + m[ 2]*(m[ 4]*m[ 9] - m[ 5]*m[ 8]));
    memcpy(m, &r, sizeof(r));
}
/* compose a coloumn major 4 x 4 matrix from vec3 position and vec4 orientation/rotation quaternion */
void _m3d_mat(M3D_FLOAT *r, m3dv_t *p, m3dv_t *q)
{
    if(q->x == (M3D_FLOAT)0.0 && q->y == (M3D_FLOAT)0.0 && q->z >=(M3D_FLOAT) 0.7071065 && q->z <= (M3D_FLOAT)0.7071075 &&
        q->w == (M3D_FLOAT)0.0) {
        r[ 1] = r[ 2] = r[ 4] = r[ 6] = r[ 8] = r[ 9] = (M3D_FLOAT)0.0;
        r[ 0] = r[ 5] = r[10] = (M3D_FLOAT)-1.0;
    } else {
        r[ 0] = 1 - 2 * (q->y * q->y + q->z * q->z); if(r[ 0]>-M3D_EPSILON && r[ 0]<M3D_EPSILON) r[ 0]=(M3D_FLOAT)0.0;
        r[ 1] = 2 * (q->x * q->y - q->z * q->w);     if(r[ 1]>-M3D_EPSILON && r[ 1]<M3D_EPSILON) r[ 1]=(M3D_FLOAT)0.0;
        r[ 2] = 2 * (q->x * q->z + q->y * q->w);     if(r[ 2]>-M3D_EPSILON && r[ 2]<M3D_EPSILON) r[ 2]=(M3D_FLOAT)0.0;
        r[ 4] = 2 * (q->x * q->y + q->z * q->w);     if(r[ 4]>-M3D_EPSILON && r[ 4]<M3D_EPSILON) r[ 4]=(M3D_FLOAT)0.0;
        r[ 5] = 1 - 2 * (q->x * q->x + q->z * q->z); if(r[ 5]>-M3D_EPSILON && r[ 5]<M3D_EPSILON) r[ 5]=(M3D_FLOAT)0.0;
        r[ 6] = 2 * (q->y * q->z - q->x * q->w);     if(r[ 6]>-M3D_EPSILON && r[ 6]<M3D_EPSILON) r[ 6]=(M3D_FLOAT)0.0;
        r[ 8] = 2 * (q->x * q->z - q->y * q->w);     if(r[ 8]>-M3D_EPSILON && r[ 8]<M3D_EPSILON) r[ 8]=(M3D_FLOAT)0.0;
        r[ 9] = 2 * (q->y * q->z + q->x * q->w);     if(r[ 9]>-M3D_EPSILON && r[ 9]<M3D_EPSILON) r[ 9]=(M3D_FLOAT)0.0;
        r[10] = 1 - 2 * (q->x * q->x + q->y * q->y); if(r[10]>-M3D_EPSILON && r[10]<M3D_EPSILON) r[10]=(M3D_FLOAT)0.0;
    }
    r[ 3] = p->x; r[ 7] = p->y; r[11] = p->z;
    r[12] = 0; r[13] = 0; r[14] = 0; r[15] = 1;
}
#endif
#if !defined(M3D_NOANIMATION) || !defined(M3D_NONORMALS)
/* portable fast inverse square root calculation. returns 1/sqrt(x) */
static M3D_FLOAT _m3d_rsq(M3D_FLOAT x)
{
#ifdef M3D_DOUBLE
    return ((M3D_FLOAT)15.0/(M3D_FLOAT)8.0) + ((M3D_FLOAT)-5.0/(M3D_FLOAT)4.0)*x + ((M3D_FLOAT)3.0/(M3D_FLOAT)8.0)*x*x;
#else
    /* John Carmack's */
    float x2 = x * 0.5f;
    uint32_t *i = (uint32_t*)&x;
    *i = (0x5f3759df - (*i >> 1));
    return x * (1.5f - (x2 * x * x));
#endif
}
#endif

/**
 * Function to decode a Model 3D into in-memory format
 */
m3d_t *m3d_load(unsigned char *data, m3dread_t readfilecb, m3dfree_t freecb, m3d_t *mtllib)
{
    unsigned char *end, *chunk, *buff, weights[8];
    unsigned int i, j, k, l, n, am, len = 0, reclen, offs;
#ifndef M3D_NOVOXELS
    int32_t min_x, min_y, min_z, max_x, max_y, max_z, sx, sy, sz, x, y, z;
    M3D_INDEX edge[8], enorm;
#endif
    char *name, *lang;
    float f;
    m3d_t *model;
    M3D_INDEX mi;
#ifdef M3D_VERTEXMAX
    M3D_INDEX pi;
#endif
    M3D_FLOAT w;
    m3dcd_t *cd;
    m3dtx_t *tx;
    m3dh_t *h;
    m3dm_t *m;
    m3da_t *a;
    m3di_t *t;
#ifndef M3D_NONORMALS
    char neednorm = 0;
    m3dv_t *norm = NULL, *v0, *v1, *v2, va, vb;
#endif
#ifndef M3D_NOANIMATION
    M3D_FLOAT r[16];
#endif
#if !defined(M3D_NOWEIGHTS) || !defined(M3D_NOANIMATION)
    m3db_t *b;
#endif
#ifndef M3D_NOWEIGHTS
    m3ds_t *sk;
#endif
#ifdef M3D_ASCII
    m3ds_t s;
    M3D_INDEX bi[M3D_BONEMAXLEVEL+1], level;
    const char *ol;
    char *ptr, *pe, *fn;
#endif
#ifdef M3D_PROFILING
    struct timeval tv0, tv1, tvd;
    gettimeofday(&tv0, NULL);
#endif

    if(!data || (!M3D_CHUNKMAGIC(data, '3','D','M','O')
#ifdef M3D_ASCII
        && !M3D_CHUNKMAGIC(data, '3','d','m','o')
#endif
        )) return NULL;
    model = (m3d_t*)M3D_MALLOC(sizeof(m3d_t));
    if(!model) {
        M3D_LOG("Out of memory");
        return NULL;
    }
    memset(model, 0, sizeof(m3d_t));

    if(mtllib) {
        model->nummaterial = mtllib->nummaterial;
        model->material = mtllib->material;
        model->numtexture = mtllib->numtexture;
        model->texture = mtllib->texture;
        model->flags |= M3D_FLG_MTLLIB;
    }
#ifdef M3D_ASCII
    /* ASCII variant? */
    if(M3D_CHUNKMAGIC(data, '3','d','m','o')) {
        model->errcode = M3D_ERR_BADFILE;
        model->flags |= M3D_FLG_FREESTR;
        model->raw = (m3dhdr_t*)data;
        ptr = (char*)data;
        ol = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");
        /* parse header. Don't use sscanf, that's incredibly slow */
        ptr = _m3d_findarg(ptr);
        if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
        pe = _m3d_findnl(ptr);
        model->scale = (float)strtod(ptr, NULL); ptr = pe;
        if(model->scale <= (M3D_FLOAT)0.0) model->scale = (M3D_FLOAT)1.0;
        model->name = _m3d_safestr(ptr, 2); ptr = _m3d_findnl(ptr);
        if(!*ptr) goto asciiend;
        model->license = _m3d_safestr(ptr, 2); ptr = _m3d_findnl(ptr);
        if(!*ptr) goto asciiend;
        model->author = _m3d_safestr(ptr, 2); ptr = _m3d_findnl(ptr);
        if(!*ptr) goto asciiend;
        if(*ptr != '\r' && *ptr != '\n')
            model->desc = _m3d_safestr(ptr, 3);
        while(*ptr) {
            while(*ptr && *ptr!='\n') ptr++;
            ptr++; if(*ptr=='\r') ptr++;
            if(*ptr == '\n') break;
        }

        /* the main chunk reader loop */
        while(*ptr) {
            while(*ptr && (*ptr == '\r' || *ptr == '\n')) ptr++;
            if(!*ptr || (ptr[0]=='E' && ptr[1]=='n' && ptr[2]=='d')) break;
            /* make sure there's at least one data row */
            pe = ptr; ptr = _m3d_findnl(ptr);
            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
            /* Preview chunk */
            if(!memcmp(pe, "Preview", 7)) {
                if(readfilecb) {
                    pe = _m3d_safestr(ptr, 0);
                    if(!pe || !*pe) goto asciiend;
                    model->preview.data = (*readfilecb)(pe, &model->preview.length);
                    M3D_FREE(pe);
                }
                while(*ptr && *ptr != '\r' && *ptr != '\n')
                    ptr = _m3d_findnl(ptr);
            } else
            /* texture map chunk */
            if(!memcmp(pe, "Textmap", 7)) {
                if(model->tmap) { M3D_LOG("More texture map chunks, should be unique"); goto asciiend; }
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    i = model->numtmap++;
                    model->tmap = (m3dti_t*)M3D_REALLOC(model->tmap, model->numtmap * sizeof(m3dti_t));
                    if(!model->tmap) goto memerr;
                    ptr = _m3d_getfloat(ptr, &model->tmap[i].u);
                    if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                    _m3d_getfloat(ptr, &model->tmap[i].v);
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* vertex chunk */
            if(!memcmp(pe, "Vertex", 6)) {
                if(model->vertex) { M3D_LOG("More vertex chunks, should be unique"); goto asciiend; }
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    i = model->numvertex++;
                    model->vertex = (m3dv_t*)M3D_REALLOC(model->vertex, model->numvertex * sizeof(m3dv_t));
                    if(!model->vertex) goto memerr;
                    memset(&model->vertex[i], 0, sizeof(m3dv_t));
                    model->vertex[i].skinid = M3D_UNDEF;
                    model->vertex[i].color = 0;
                    model->vertex[i].w = (M3D_FLOAT)1.0;
                    ptr = _m3d_getfloat(ptr, &model->vertex[i].x);
                    if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                    ptr = _m3d_getfloat(ptr, &model->vertex[i].y);
                    if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                    ptr = _m3d_getfloat(ptr, &model->vertex[i].z);
                    if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                    ptr = _m3d_getfloat(ptr, &model->vertex[i].w);
                    if(!*ptr) goto asciiend;
                    if(*ptr == '#') {
                        ptr = _m3d_gethex(ptr, &model->vertex[i].color);
                        if(!*ptr) goto asciiend;
                    }
                    /* parse skin */
                    memset(&s, 0, sizeof(m3ds_t));
                    for(j = 0, w = (M3D_FLOAT)0.0; j < M3D_NUMBONE && *ptr && *ptr != '\r' && *ptr != '\n'; j++) {
                        ptr = _m3d_findarg(ptr);
                        if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                        ptr = _m3d_getint(ptr, &k);
                        s.boneid[j] = (M3D_INDEX)k;
                        if(*ptr == ':') {
                            ptr++;
                            ptr = _m3d_getfloat(ptr, &s.weight[j]);
                            w += s.weight[j];
                        } else if(!j)
                            s.weight[j] = (M3D_FLOAT)1.0;
                        if(!*ptr) goto asciiend;
                    }
                    if(s.boneid[0] != M3D_UNDEF && s.weight[0] > (M3D_FLOAT)0.0) {
                        if(w != (M3D_FLOAT)1.0 && w != (M3D_FLOAT)0.0)
                            for(j = 0; j < M3D_NUMBONE && s.weight[j] > (M3D_FLOAT)0.0; j++)
                                s.weight[j] /= w;
                        k = M3D_NOTDEFINED;
                        if(model->skin) {
                            for(j = 0; j < model->numskin; j++)
                                if(!memcmp(&model->skin[j], &s, sizeof(m3ds_t))) { k = j; break; }
                        }
                        if(k == M3D_NOTDEFINED) {
                            k = model->numskin++;
                            model->skin = (m3ds_t*)M3D_REALLOC(model->skin, model->numskin * sizeof(m3ds_t));
                            if(!model->skin) goto memerr;
                            memcpy(&model->skin[k], &s, sizeof(m3ds_t));
                        }
                        model->vertex[i].skinid = (M3D_INDEX)k;
                    }
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* Skeleton, bone hierarchy */
            if(!memcmp(pe, "Bones", 5)) {
                if(model->bone) { M3D_LOG("More bones chunks, should be unique"); goto asciiend; }
                bi[0] = M3D_UNDEF;
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    i = model->numbone++;
                    model->bone = (m3db_t*)M3D_REALLOC(model->bone, model->numbone * sizeof(m3db_t));
                    if(!model->bone) goto memerr;
                    for(level = 0; *ptr == '/'; ptr++, level++);
                    if(level > M3D_BONEMAXLEVEL || !*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                    bi[level+1] = i;
                    model->bone[i].numweight = 0;
                    model->bone[i].weight = NULL;
                    model->bone[i].parent = bi[level];
                    ptr = _m3d_getint(ptr, &k);
                    ptr = _m3d_findarg(ptr);
                    if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                    model->bone[i].pos = (M3D_INDEX)k;
                    ptr = _m3d_getint(ptr, &k);
                    ptr = _m3d_findarg(ptr);
                    if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                    model->bone[i].ori = (M3D_INDEX)k;
                    model->vertex[k].skinid = M3D_INDEXMAX;
                    pe = _m3d_safestr(ptr, 0);
                    if(!pe || !*pe) goto asciiend;
                    model->bone[i].name = pe;
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* material chunk */
            if(!memcmp(pe, "Material", 8)) {
                pe = _m3d_findarg(pe);
                if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                pe = _m3d_safestr(pe, 0);
                if(!pe || !*pe) goto asciiend;
                for(i = 0; i < model->nummaterial; i++)
                    if(!strcmp(pe, model->material[i].name)) {
                        M3D_LOG("Multiple definitions for material");
                        M3D_LOG(pe);
                        M3D_FREE(pe);
                        pe = NULL;
                        while(*ptr && *ptr != '\r' && *ptr != '\n') ptr = _m3d_findnl(ptr);
                        break;
                    }
                if(!pe) continue;
                i = model->nummaterial++;
                if(model->flags & M3D_FLG_MTLLIB) {
                    m = model->material;
                    model->material = (m3dm_t*)M3D_MALLOC(model->nummaterial * sizeof(m3dm_t));
                    if(!model->material) goto memerr;
                    memcpy(model->material, m, (model->nummaterial - 1) * sizeof(m3dm_t));
                    if(model->texture) {
                        tx = model->texture;
                        model->texture = (m3dtx_t*)M3D_MALLOC(model->numtexture * sizeof(m3dtx_t));
                        if(!model->texture) goto memerr;
                        memcpy(model->texture, tx, model->numtexture * sizeof(m3dm_t));
                    }
                    model->flags &= ~M3D_FLG_MTLLIB;
                } else {
                    model->material = (m3dm_t*)M3D_REALLOC(model->material, model->nummaterial * sizeof(m3dm_t));
                    if(!model->material) goto memerr;
                }
                m = &model->material[i];
                m->name = pe;
                m->numprop = 0;
                m->prop = NULL;
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    k = n = 256;
                    if(*ptr == 'm' && *(ptr+1) == 'a' && *(ptr+2) == 'p' && *(ptr+3) == '_') {
                        k = m3dpf_map;
                        ptr += 4;
                    }
                    for(j = 0; j < sizeof(m3d_propertytypes)/sizeof(m3d_propertytypes[0]); j++)
                        if(!memcmp(ptr, m3d_propertytypes[j].key, strlen(m3d_propertytypes[j].key))) {
                            n = m3d_propertytypes[j].id;
                            if(k != m3dpf_map) k = m3d_propertytypes[j].format;
                            break;
                        }
                    if(n != 256 && k != 256) {
                        ptr = _m3d_findarg(ptr);
                        if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                        j = m->numprop++;
                        m->prop = (m3dp_t*)M3D_REALLOC(m->prop, m->numprop * sizeof(m3dp_t));
                        if(!m->prop) goto memerr;
                        m->prop[j].type = n + (k == m3dpf_map && n < 128 ? 128 : 0);
                        switch(k) {
                            case m3dpf_color: ptr = _m3d_gethex(ptr, &m->prop[j].value.color); break;
                            case m3dpf_uint8:
                            case m3dpf_uint16:
                            case m3dpf_uint32: ptr = _m3d_getint(ptr, &m->prop[j].value.num); break;
                            case m3dpf_float:  ptr = _m3d_getfloat(ptr, &m->prop[j].value.fnum); break;
                            case m3dpf_map:
                                pe = _m3d_safestr(ptr, 0);
                                if(!pe || !*pe) goto asciiend;
                                m->prop[j].value.textureid = _m3d_gettx(model, readfilecb, freecb, pe);
                                if(model->errcode == M3D_ERR_ALLOC) { M3D_FREE(pe); goto memerr; }
                                /* this error code only returned if readfilecb was specified */
                                if(m->prop[j].value.textureid == M3D_UNDEF) {
                                    M3D_LOG("Texture not found");
                                    M3D_LOG(pe);
                                    m->numprop--;
                                }
                                M3D_FREE(pe);
                            break;
                        }
                    } else {
                        M3D_LOG("Unknown material property in");
                        M3D_LOG(m->name);
                        model->errcode = M3D_ERR_UNKPROP;
                    }
                    ptr = _m3d_findnl(ptr);
                }
                if(!m->numprop) model->nummaterial--;
            } else
            /* procedural */
            if(!memcmp(pe, "Procedural", 10)) {
                pe = _m3d_safestr(ptr, 0);
                _m3d_getpr(model, readfilecb, freecb, pe);
                M3D_FREE(pe);
                while(*ptr && *ptr != '\r' && *ptr != '\n') ptr = _m3d_findnl(ptr);
            } else
            /* mesh */
            if(!memcmp(pe, "Mesh", 4)) {
                mi = M3D_UNDEF;
#ifdef M3D_VERTEXMAX
                pi = M3D_UNDEF;
#endif
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    if(*ptr == 'u') {
                        ptr = _m3d_findarg(ptr);
                        if(!*ptr) goto asciiend;
                        mi = M3D_UNDEF;
                        if(*ptr != '\r' && *ptr != '\n') {
                            pe = _m3d_safestr(ptr, 0);
                            if(!pe || !*pe) goto asciiend;
                            for(j = 0; j < model->nummaterial; j++)
                                if(!strcmp(pe, model->material[j].name)) { mi = (M3D_INDEX)j; break; }
                            if(mi == M3D_UNDEF && !(model->flags & M3D_FLG_MTLLIB)) {
                                mi = model->nummaterial++;
                                model->material = (m3dm_t*)M3D_REALLOC(model->material, model->nummaterial * sizeof(m3dm_t));
                                if(!model->material) goto memerr;
                                model->material[mi].name = pe;
                                model->material[mi].numprop = 1;
                                model->material[mi].prop = NULL;
                            } else
                                M3D_FREE(pe);
                        }
                    } else
                    if(*ptr == 'p') {
                        ptr = _m3d_findarg(ptr);
                        if(!*ptr) goto asciiend;
#ifdef M3D_VERTEXMAX
                        pi = M3D_UNDEF;
                        if(*ptr != '\r' && *ptr != '\n') {
                            pe = _m3d_safestr(ptr, 0);
                            if(!pe || !*pe) goto asciiend;
                            for(j = 0; j < model->numparam; j++)
                                if(!strcmp(pe, model->param[j].name)) { pi = (M3D_INDEX)j; break; }
                            if(pi == M3D_UNDEF) {
                                pi = model->numparam++;
                                model->param = (m3dvi_t*)M3D_REALLOC(model->param, model->numparam * sizeof(m3dvi_t));
                                if(!model->param) goto memerr;
                                model->param[pi].name = pe;
                                model->param[pi].count = 0;
                            } else
                                M3D_FREE(pe);
                        }
#endif
                    } else {
                        i = model->numface++;
                        model->face = (m3df_t*)M3D_REALLOC(model->face, model->numface * sizeof(m3df_t));
                        if(!model->face) goto memerr;
                        memset(&model->face[i], 255, sizeof(m3df_t)); /* set all index to -1 by default */
                        model->face[i].materialid = mi;
#ifdef M3D_VERTEXMAX
                        model->face[i].paramid = pi;
#endif
                        /* hardcoded triangles. */
                        for(j = 0; j < 3; j++) {
                            /* vertex */
                            ptr = _m3d_getint(ptr, &k);
                            model->face[i].vertex[j] = (M3D_INDEX)k;
                            if(!*ptr) goto asciiend;
                            if(*ptr == '/') {
                                ptr++;
                                if(*ptr != '/') {
                                    /* texcoord */
                                    ptr = _m3d_getint(ptr, &k);
                                    model->face[i].texcoord[j] = (M3D_INDEX)k;
                                    if(!*ptr) goto asciiend;
                                }
                                if(*ptr == '/') {
                                    ptr++;
                                    /* normal */
                                    ptr = _m3d_getint(ptr, &k);
                                    model->face[i].normal[j] = (M3D_INDEX)k;
                                    if(!*ptr) goto asciiend;
                                }
                                if(*ptr == '/') {
                                    ptr++;
                                    /* maximum */
                                    ptr = _m3d_getint(ptr, &k);
#ifdef M3D_VERTEXMAX
                                    model->face[i].vertmax[j] = (M3D_INDEX)k;
#endif
                                    if(!*ptr) goto asciiend;
                                }
                            }
#ifndef M3D_NONORMALS
                            if(model->face[i].normal[j] == M3D_UNDEF) neednorm = 1;
#endif
                            ptr = _m3d_findarg(ptr);
                        }
                    }
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* voxel types chunk */
            if(!memcmp(pe, "VoxTypes", 8) || !memcmp(pe, "Voxtypes", 8)) {
                if(model->voxtype) { M3D_LOG("More voxel types chunks, should be unique"); goto asciiend; }
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    i = model->numvoxtype++;
                    model->voxtype = (m3dvt_t*)M3D_REALLOC(model->voxtype, model->numvoxtype * sizeof(m3dvt_t));
                    if(!model->voxtype) goto memerr;
                    memset(&model->voxtype[i], 0, sizeof(m3dvt_t));
                    model->voxtype[i].materialid = M3D_UNDEF;
                    model->voxtype[i].skinid = M3D_UNDEF;
                    ptr = _m3d_gethex(ptr, &model->voxtype[i].color);
                    if(!*ptr) goto asciiend;
                    if(*ptr == '/') {
                        ptr = _m3d_gethex(ptr, &k);
                        model->voxtype[i].rotation = k;
                        if(!*ptr) goto asciiend;
                        if(*ptr == '/') {
                            ptr = _m3d_gethex(ptr, &k);
                            model->voxtype[i].voxshape = k;
                            if(!*ptr) goto asciiend;
                        }
                    }
                    while(*ptr == ' ' || *ptr == '\t') ptr++;
                    if(*ptr == '\r' || *ptr == '\n') { ptr = _m3d_findnl(ptr); continue; }
                    /* name */
                    if(*ptr != '-') {
                        pe = _m3d_safestr(ptr, 0);
                        if(!pe || !*pe) goto asciiend;
                        model->voxtype[i].name = pe;
                        for(j = 0; j < model->nummaterial; j++)
                            if(!strcmp(pe, model->material[j].name)) { model->voxtype[i].materialid = (M3D_INDEX)j; break; }
                    }
                    ptr = _m3d_findarg(ptr);
                    /* parse skin */
                    memset(&s, 0, sizeof(m3ds_t));
                    for(j = 0, w = (M3D_FLOAT)0.0; j < M3D_NUMBONE && *ptr && *ptr != '{' && *ptr != '\r' && *ptr != '\n'; j++) {
                        ptr = _m3d_getint(ptr, &k);
                        s.boneid[j] = (M3D_INDEX)k;
                        if(*ptr == ':') {
                            ptr++;
                            ptr = _m3d_getfloat(ptr, &s.weight[j]);
                            w += s.weight[j];
                        } else if(!j)
                            s.weight[j] = (M3D_FLOAT)1.0;
                        if(!*ptr) goto asciiend;
                        ptr = _m3d_findarg(ptr);
                    }
                    if(s.boneid[0] != M3D_UNDEF && s.weight[0] > (M3D_FLOAT)0.0) {
                        if(w != (M3D_FLOAT)1.0 && w != (M3D_FLOAT)0.0)
                            for(j = 0; j < M3D_NUMBONE && s.weight[j] > (M3D_FLOAT)0.0; j++)
                                s.weight[j] /= w;
                        k = M3D_NOTDEFINED;
                        if(model->skin) {
                            for(j = 0; j < model->numskin; j++)
                                if(!memcmp(&model->skin[j], &s, sizeof(m3ds_t))) { k = j; break; }
                        }
                        if(k == M3D_NOTDEFINED) {
                            k = model->numskin++;
                            model->skin = (m3ds_t*)M3D_REALLOC(model->skin, model->numskin * sizeof(m3ds_t));
                            if(!model->skin) goto memerr;
                            memcpy(&model->skin[k], &s, sizeof(m3ds_t));
                        }
                        model->voxtype[i].skinid = (M3D_INDEX)k;
                    }
                    /* parse item list */
                    if(*ptr == '{') {
                        while(*ptr == '{' || *ptr == ' ' || *ptr == '\t') ptr++;
                        while(*ptr && *ptr != '}' && *ptr != '\r' && *ptr != '\n') {
                            ptr = _m3d_getint(ptr, &k);
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '}' || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            pe = _m3d_safestr(ptr, 0);
                            if(!pe || !*pe) goto asciiend;
                            ptr = _m3d_findarg(ptr);
                            j = model->voxtype[i].numitem++;
                            model->voxtype[i].item = (m3dvi_t*)M3D_REALLOC(model->voxtype[i].item,
                                model->voxtype[i].numitem * sizeof(m3dvi_t));
                            if(!model->voxtype[i].item) goto memerr;
                            model->voxtype[i].item[j].count = k;
                            model->voxtype[i].item[j].name = pe;
                        }
                        if(*ptr != '}') goto asciiend;
                    }
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* voxel data */
            if(!memcmp(pe, "Voxel", 5)) {
                if(!model->voxtype) { M3D_LOG("No voxel type chunk before voxel data"); goto asciiend; }
                pe = _m3d_findarg(pe);
                if(!*pe) goto asciiend;
                if(*pe == '\r' || *pe == '\n') pe = NULL;
                else pe = _m3d_safestr(pe, 0);
                i = model->numvoxel++;
                model->voxel = (m3dvx_t*)M3D_REALLOC(model->voxel, model->numvoxel * sizeof(m3dvx_t));
                if(!model->voxel) goto memerr;
                memset(&model->voxel[i], 0, sizeof(m3dvx_t));
                model->voxel[i].name = pe;
                k = l = 0;
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    switch(*ptr) {
                        case 'u':
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            ptr = _m3d_getint(ptr, &n);
                            model->voxel[i].uncertain = ((n > 0 && n < 256 ? n : 0) * 255) / 100;
                            ptr = _m3d_findarg(ptr);
                            if(*ptr && *ptr != '\r' && *ptr != '\n') {
                                ptr = _m3d_getint(ptr, &n);
                                model->voxel[i].groupid = n > 0 && n < 256 ? n : 0;
                            }
                        break;
                        case 'p':
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            ptr = _m3d_getint(ptr, &n);
                            model->voxel[i].x = n;
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            ptr = _m3d_getint(ptr, &n);
                            model->voxel[i].y = n;
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            ptr = _m3d_getint(ptr, &n);
                            model->voxel[i].z = n;
                        break;
                        case 'd':
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            ptr = _m3d_getint(ptr, &n);
                            model->voxel[i].w = n;
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            ptr = _m3d_getint(ptr, &n);
                            model->voxel[i].h = n;
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                            ptr = _m3d_getint(ptr, &n);
                            model->voxel[i].d = n;
                        break;
                        case 'l':
                            if(model->voxel[i].data) { l++; k = 0; }
                            else {
                                if(!model->voxel[i].w || !model->voxel[i].h || !model->voxel[i].d) {
                                    M3D_LOG("No voxel dimension before layer data");
                                    goto asciiend;
                                }
                                model->voxel[i].data = (M3D_VOXEL*)M3D_MALLOC(
                                    model->voxel[i].w * model->voxel[i].h * model->voxel[i].d * sizeof(M3D_VOXEL));
                                if(!model->voxel[i].data) goto memerr;
                            }
                        break;
                        default:
                            if(!model->voxel[i].data || l >= model->voxel[i].h || k >= model->voxel[i].d) {
                                M3D_LOG("Missing voxel attributes or out of bound data");
                                goto asciiend;
                            }
                            for(n = l * model->voxel[i].w * model->voxel[i].d + k * model->voxel[i].w;
                                j < model->voxel[i].w && *ptr && *ptr != '\r' && *ptr != '\n'; j++) {
                                ptr = _m3d_getint(ptr, &am);
                                if(am >= model->numvoxtype) goto asciiend;
                                model->voxel[i].data[n + j] = am;
                            }
                            k++;
                        break;
                    }
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* mathematical shape */
            if(!memcmp(pe, "Shape", 5)) {
                pe = _m3d_findarg(pe);
                if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                pe = _m3d_safestr(pe, 0);
                if(!pe || !*pe) goto asciiend;
                i = model->numshape++;
                model->shape = (m3dh_t*)M3D_REALLOC(model->shape, model->numshape * sizeof(m3ds_t));
                if(!model->shape) goto memerr;
                h = &model->shape[i];
                h->name = pe;
                h->group = M3D_UNDEF;
                h->numcmd = 0;
                h->cmd = NULL;
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    if(!memcmp(ptr, "group", 5)) {
                        ptr = _m3d_findarg(ptr);
                        ptr = _m3d_getint(ptr, &h->group);
                        ptr = _m3d_findnl(ptr);
                        if(h->group != M3D_UNDEF && h->group >= model->numbone) {
                            M3D_LOG("Unknown bone id as shape group in shape");
                            M3D_LOG(pe);
                            h->group = M3D_UNDEF;
                            model->errcode = M3D_ERR_SHPE;
                        }
                        continue;
                    }
                    for(cd = NULL, k = 0; k < (unsigned int)(sizeof(m3d_commandtypes)/sizeof(m3d_commandtypes[0])); k++) {
                        j = (unsigned int)strlen(m3d_commandtypes[k].key);
                        if(!memcmp(ptr, m3d_commandtypes[k].key, j) && (ptr[j] == ' ' || ptr[j] == '\r' || ptr[j] == '\n'))
                            { cd = &m3d_commandtypes[k]; break; }
                    }
                    if(cd) {
                        j = h->numcmd++;
                        h->cmd = (m3dc_t*)M3D_REALLOC(h->cmd, h->numcmd * sizeof(m3dc_t));
                        if(!h->cmd) goto memerr;
                        h->cmd[j].type = k;
                        h->cmd[j].arg = (uint32_t*)M3D_MALLOC(cd->p * sizeof(uint32_t));
                        if(!h->cmd[j].arg) goto memerr;
                        memset(h->cmd[j].arg, 0, cd->p * sizeof(uint32_t));
                        for(k = n = 0, l = cd->p; k < l; k++) {
                            ptr = _m3d_findarg(ptr);
                            if(!*ptr) goto asciiend;
                            if(*ptr == '[') {
                                ptr = _m3d_findarg(ptr + 1);
                                if(!*ptr) goto asciiend;
                            }
                            if(*ptr == ']' || *ptr == '\r' || *ptr == '\n') break;
                            switch(cd->a[((k - n) % (cd->p - n)) + n]) {
                                case m3dcp_mi_t:
                                    mi = M3D_UNDEF;
                                    if(*ptr != '\r' && *ptr != '\n') {
                                        pe = _m3d_safestr(ptr, 0);
                                        if(!pe || !*pe) goto asciiend;
                                        for(n = 0; n < model->nummaterial; n++)
                                            if(!strcmp(pe, model->material[n].name)) { mi = (M3D_INDEX)n; break; }
                                        if(mi == M3D_UNDEF && !(model->flags & M3D_FLG_MTLLIB)) {
                                            mi = model->nummaterial++;
                                            model->material = (m3dm_t*)M3D_REALLOC(model->material,
                                                model->nummaterial * sizeof(m3dm_t));
                                            if(!model->material) goto memerr;
                                            model->material[mi].name = pe;
                                            model->material[mi].numprop = 1;
                                            model->material[mi].prop = NULL;
                                        } else
                                            M3D_FREE(pe);
                                    }
                                    h->cmd[j].arg[k] = mi;
                                break;
                                case m3dcp_vc_t:
#ifdef M3D_DOUBLE
                                    _m3d_getfloat(ptr, &w); f = w;
                                    memcpy(&h->cmd[j].arg[k], &f, 4);
#else
                                    _m3d_getfloat(ptr, (float*)&h->cmd[j].arg[k]);
#endif
                                break;
                                case m3dcp_va_t:
                                    ptr = _m3d_getint(ptr, &h->cmd[j].arg[k]);
                                    n = k + 1; l += (h->cmd[j].arg[k] - 1) * (cd->p - k - 1);
                                    h->cmd[j].arg = (uint32_t*)M3D_REALLOC(h->cmd[j].arg, l * sizeof(uint32_t));
                                    if(!h->cmd[j].arg) goto memerr;
                                    memset(&h->cmd[j].arg[k + 1], 0, (l - k - 1) * sizeof(uint32_t));
                                break;
                                case m3dcp_qi_t:
                                    ptr = _m3d_getint(ptr, &h->cmd[j].arg[k]);
                                    model->vertex[h->cmd[i].arg[k]].skinid = M3D_INDEXMAX;
                                break;
                                default:
                                    ptr = _m3d_getint(ptr, &h->cmd[j].arg[k]);
                                break;
                            }
                        }
                    } else {
                        M3D_LOG("Unknown shape command in");
                        M3D_LOG(h->name);
                        model->errcode = M3D_ERR_UNKCMD;
                    }
                    ptr = _m3d_findnl(ptr);
                }
                if(!h->numcmd) model->numshape--;
            } else
            /* annotation labels */
            if(!memcmp(pe, "Labels", 6)) {
                pe = _m3d_findarg(pe);
                if(!*pe) goto asciiend;
                if(*pe == '\r' || *pe == '\n') pe = NULL;
                else pe = _m3d_safestr(pe, 0);
                k = 0; fn = NULL;
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    if(*ptr == 'c') {
                        ptr = _m3d_findarg(ptr);
                        if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                        ptr = _m3d_gethex(ptr, &k);
                    } else
                    if(*ptr == 'l') {
                        ptr = _m3d_findarg(ptr);
                        if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                        fn = _m3d_safestr(ptr, 2);
                    } else {
                        i = model->numlabel++;
                        model->label = (m3dl_t*)M3D_REALLOC(model->label, model->numlabel * sizeof(m3dl_t));
                        if(!model->label) goto memerr;
                        model->label[i].name = pe;
                        model->label[i].lang = fn;
                        model->label[i].color = k;
                        ptr = _m3d_getint(ptr, &j);
                        model->label[i].vertexid = (M3D_INDEX)j;
                        ptr = _m3d_findarg(ptr);
                        if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                        model->label[i].text = _m3d_safestr(ptr, 2);
                    }
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* action */
            if(!memcmp(pe, "Action", 6)) {
                pe = _m3d_findarg(pe);
                if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                pe = _m3d_getint(pe, &k);
                pe = _m3d_findarg(pe);
                if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                pe = _m3d_safestr(pe, 0);
                if(!pe || !*pe) goto asciiend;
                i = model->numaction++;
                model->action = (m3da_t*)M3D_REALLOC(model->action, model->numaction * sizeof(m3da_t));
                if(!model->action) goto memerr;
                a = &model->action[i];
                a->name = pe;
                a->durationmsec = k;
                /* skip the first frame marker as there's always at least one frame */
                a->numframe = 1;
                a->frame = (m3dfr_t*)M3D_MALLOC(sizeof(m3dfr_t));
                if(!a->frame) goto memerr;
                a->frame[0].msec = 0;
                a->frame[0].numtransform = 0;
                a->frame[0].transform = NULL;
                i = 0;
                if(*ptr == 'f')
                    ptr = _m3d_findnl(ptr);
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    if(*ptr == 'f') {
                        i = a->numframe++;
                        a->frame = (m3dfr_t*)M3D_REALLOC(a->frame, a->numframe * sizeof(m3dfr_t));
                        if(!a->frame) goto memerr;
                        ptr = _m3d_findarg(ptr);
                        ptr = _m3d_getint(ptr, &a->frame[i].msec);
                        a->frame[i].numtransform = 0;
                        a->frame[i].transform = NULL;
                    } else {
                        j = a->frame[i].numtransform++;
                        a->frame[i].transform = (m3dtr_t*)M3D_REALLOC(a->frame[i].transform,
                            a->frame[i].numtransform * sizeof(m3dtr_t));
                        if(!a->frame[i].transform) goto memerr;
                        ptr = _m3d_getint(ptr, &k);
                        ptr = _m3d_findarg(ptr);
                        if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                        a->frame[i].transform[j].boneid = (M3D_INDEX)k;
                        ptr = _m3d_getint(ptr, &k);
                        ptr = _m3d_findarg(ptr);
                        if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                        a->frame[i].transform[j].pos = (M3D_INDEX)k;
                        ptr = _m3d_getint(ptr, &k);
                        if(!*ptr || *ptr == '\r' || *ptr == '\n') goto asciiend;
                        a->frame[i].transform[j].ori = (M3D_INDEX)k;
                        model->vertex[k].skinid = M3D_INDEXMAX;
                    }
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* inlined assets chunk */
            if(!memcmp(pe, "Assets", 6)) {
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    if(readfilecb) {
                        pe = _m3d_safestr(ptr, 2);
                        if(!pe || !*pe) goto asciiend;
                        i = model->numinlined++;
                        model->inlined = (m3di_t*)M3D_REALLOC(model->inlined, model->numinlined * sizeof(m3di_t));
                        if(!model->inlined) goto memerr;
                        t = &model->inlined[i];
                        model->inlined[i].data = (*readfilecb)(pe, &model->inlined[i].length);
                        if(model->inlined[i].data) {
                            fn = strrchr(pe, '.');
                            if(fn && (fn[1] == 'p' || fn[1] == 'P') && (fn[2] == 'n' || fn[2] == 'N') &&
                                (fn[3] == 'g' || fn[3] == 'G')) *fn = 0;
                            fn = strrchr(pe, '/');
                            if(!fn) fn = strrchr(pe, '\\');
                            if(!fn) fn = pe; else fn++;
                            model->inlined[i].name = _m3d_safestr(fn, 0);
                        } else
                            model->numinlined--;
                        M3D_FREE(pe);
                    }
                    ptr = _m3d_findnl(ptr);
                }
            } else
            /* extra chunks */
            if(!memcmp(pe, "Extra", 5)) {
                pe = _m3d_findarg(pe);
                if(!*pe || *pe == '\r' || *pe == '\n') goto asciiend;
                buff = (unsigned char*)_m3d_findnl(ptr);
                k = ((uint32_t)((uintptr_t)buff - (uintptr_t)ptr) / 3) + 1;
                i = model->numextra++;
                model->extra = (m3dchunk_t**)M3D_REALLOC(model->extra, model->numextra * sizeof(m3dchunk_t*));
                if(!model->extra) goto memerr;
                model->extra[i] = (m3dchunk_t*)M3D_MALLOC(k + sizeof(m3dchunk_t));
                if(!model->extra[i]) goto memerr;
                memcpy(&model->extra[i]->magic, pe, 4);
                model->extra[i]->length = sizeof(m3dchunk_t);
                pe = (char*)model->extra[i] + sizeof(m3dchunk_t);
                while(*ptr && *ptr != '\r' && *ptr != '\n') {
                    ptr = _m3d_gethex(ptr, &k);
                    *pe++ = (uint8_t)k;
                    model->extra[i]->length++;
                }
            } else
                goto asciiend;
        }
        model->errcode = M3D_SUCCESS;
asciiend:
        setlocale(LC_NUMERIC, ol);
        goto postprocess;
    }
#endif
    /* Binary variant */
    len = ((m3dhdr_t*)data)->length - 8;
    data += 8;
    if(M3D_CHUNKMAGIC(data, 'P','R','V','W')) {
        /* optional preview chunk */
        model->preview.length = ((m3dchunk_t*)data)->length;
        model->preview.data = data + sizeof(m3dchunk_t);
        data += model->preview.length;
        len -= model->preview.length;
    }
    if(!M3D_CHUNKMAGIC(data, 'H','E','A','D')) {
        buff = (unsigned char *)stbi_zlib_decode_malloc_guesssize_headerflag((const char*)data, len, 4096, (int*)&len, 1);
        if(!buff || !len || !M3D_CHUNKMAGIC(buff, 'H','E','A','D')) {
            if(buff) M3D_FREE(buff);
            M3D_FREE(model);
            return NULL;
        }
        buff = (unsigned char*)M3D_REALLOC(buff, len);
        model->flags |= M3D_FLG_FREERAW; /* mark that we have to free the raw buffer */
        data = buff;
#ifdef M3D_PROFILING
        gettimeofday(&tv1, NULL);
        tvd.tv_sec = tv1.tv_sec - tv0.tv_sec;
        tvd.tv_usec = tv1.tv_usec - tv0.tv_usec;
        if(tvd.tv_usec < 0) { tvd.tv_sec--; tvd.tv_usec += 1000000L; }
        printf("  Deflate model   %ld.%06ld sec\n", tvd.tv_sec, tvd.tv_usec);
        memcpy(&tv0, &tv1, sizeof(struct timeval));
#endif
    }
    model->raw = (m3dhdr_t*)data;
    end = data + len;

    /* parse header */
    data += sizeof(m3dhdr_t);
    M3D_LOG((char*)data);
    model->name = (char*)data;
    for(; data < end && *data; data++) {}; data++;
    model->license = (char*)data;
    for(; data < end && *data; data++) {}; data++;
    model->author = (char*)data;
    for(; data < end && *data; data++) {}; data++;
    model->desc = (char*)data;
    chunk = (unsigned char*)model->raw + model->raw->length;
    model->scale = (M3D_FLOAT)model->raw->scale;
    if(model->scale <= (M3D_FLOAT)0.0) model->scale = (M3D_FLOAT)1.0;
    model->vc_s = 1 << ((model->raw->types >> 0) & 3);  /* vertex coordinate size */
    model->vi_s = 1 << ((model->raw->types >> 2) & 3);  /* vertex index size */
    model->si_s = 1 << ((model->raw->types >> 4) & 3);  /* string offset size */
    model->ci_s = 1 << ((model->raw->types >> 6) & 3);  /* color index size */
    model->ti_s = 1 << ((model->raw->types >> 8) & 3);  /* tmap index size */
    model->bi_s = 1 << ((model->raw->types >>10) & 3);  /* bone index size */
    model->nb_s = 1 << ((model->raw->types >>12) & 3);  /* number of bones per vertex */
    model->sk_s = 1 << ((model->raw->types >>14) & 3);  /* skin index size */
    model->fc_s = 1 << ((model->raw->types >>16) & 3);  /* frame counter size */
    model->hi_s = 1 << ((model->raw->types >>18) & 3);  /* shape index size */
    model->fi_s = 1 << ((model->raw->types >>20) & 3);  /* face index size */
    model->vd_s = 1 << ((model->raw->types >>22) & 3);  /* voxel dimension size */
    model->vp_s = 1 << ((model->raw->types >>24) & 3);  /* voxel pixel size */
    if(model->ci_s == 8) model->ci_s = 0;               /* optional indices */
    if(model->ti_s == 8) model->ti_s = 0;
    if(model->bi_s == 8) model->bi_s = 0;
    if(model->sk_s == 8) model->sk_s = 0;
    if(model->fc_s == 8) model->fc_s = 0;
    if(model->hi_s == 8) model->hi_s = 0;
    if(model->fi_s == 8) model->fi_s = 0;

    /* variable limit checks */
    if(sizeof(M3D_FLOAT) == 4 && model->vc_s > 4) {
        M3D_LOG("Double precision coordinates not supported, truncating to float...");
        model->errcode = M3D_ERR_TRUNC;
    }
    if((sizeof(M3D_INDEX) == 2 && (model->vi_s > 2 || model->si_s > 2 || model->ci_s > 2 || model->ti_s > 2 ||
        model->bi_s > 2 || model->sk_s > 2 || model->fc_s > 2 || model->hi_s > 2 || model->fi_s > 2)) ||
       (sizeof(M3D_VOXEL) < (size_t)model->vp_s && model->vp_s != 8)) {
        M3D_LOG("32 bit indices not supported, unable to load model");
        M3D_FREE(model);
        return NULL;
    }
    if(model->vi_s > 4 || model->si_s > 4 || model->vp_s == 4) {
        M3D_LOG("Invalid index size, unable to load model");
        M3D_FREE(model);
        return NULL;
    }
    if(!M3D_CHUNKMAGIC(end - 4, 'O','M','D','3')) {
        M3D_LOG("Missing end chunk");
        M3D_FREE(model);
        return NULL;
    }
    if(model->nb_s > M3D_NUMBONE) {
        M3D_LOG("Model has more bones per vertex than what importer was configured to support");
        model->errcode = M3D_ERR_TRUNC;
    }

    /* look for inlined assets in advance, material and procedural chunks may need them */
    buff = chunk;
    while(buff < end && !M3D_CHUNKMAGIC(buff, 'O','M','D','3')) {
        data = buff;
        len = ((m3dchunk_t*)data)->length;
        buff += len;
        if(len < sizeof(m3dchunk_t) || buff >= end) {
            M3D_LOG("Invalid chunk size");
            break;
        }
        len -= sizeof(m3dchunk_t) + model->si_s;

        /* inlined assets */
        if(M3D_CHUNKMAGIC(data, 'A','S','E','T') && len > 0) {
            M3D_LOG("Inlined asset");
            i = model->numinlined++;
            model->inlined = (m3di_t*)M3D_REALLOC(model->inlined, model->numinlined * sizeof(m3di_t));
            if(!model->inlined) {
memerr:         M3D_LOG("Out of memory");
                model->errcode = M3D_ERR_ALLOC;
                return model;
            }
            data += sizeof(m3dchunk_t);
            t = &model->inlined[i];
            M3D_GETSTR(t->name);
            M3D_LOG(t->name);
            t->data = (uint8_t*)data;
            t->length = len;
        }
    }

    /* parse chunks */
    while(chunk < end && !M3D_CHUNKMAGIC(chunk, 'O','M','D','3')) {
        data = chunk;
        len = ((m3dchunk_t*)chunk)->length;
        chunk += len;
        if(len < sizeof(m3dchunk_t) || chunk >= end) {
            M3D_LOG("Invalid chunk size");
            break;
        }
        len -= sizeof(m3dchunk_t);

        /* color map */
        if(M3D_CHUNKMAGIC(data, 'C','M','A','P')) {
            M3D_LOG("Color map");
            if(model->cmap) { M3D_LOG("More color map chunks, should be unique"); model->errcode = M3D_ERR_CMAP; continue; }
            if(!model->ci_s) { M3D_LOG("Color map chunk, shouldn't be any"); model->errcode = M3D_ERR_CMAP; continue; }
            model->numcmap = len / sizeof(uint32_t);
            model->cmap = (uint32_t*)(data + sizeof(m3dchunk_t));
        } else
        /* texture map */
        if(M3D_CHUNKMAGIC(data, 'T','M','A','P')) {
            M3D_LOG("Texture map");
            if(model->tmap) { M3D_LOG("More texture map chunks, should be unique"); model->errcode = M3D_ERR_TMAP; continue; }
            if(!model->ti_s) { M3D_LOG("Texture map chunk, shouldn't be any"); model->errcode = M3D_ERR_TMAP; continue; }
            reclen = model->vc_s + model->vc_s;
            model->numtmap = len / reclen;
            model->tmap = (m3dti_t*)M3D_MALLOC(model->numtmap * sizeof(m3dti_t));
            if(!model->tmap) goto memerr;
            for(i = 0, data += sizeof(m3dchunk_t); data < chunk; i++) {
                switch(model->vc_s) {
                    case 1:
                        model->tmap[i].u = (M3D_FLOAT)((uint8_t)data[0]) / (M3D_FLOAT)255.0;
                        model->tmap[i].v = (M3D_FLOAT)((uint8_t)data[1]) / (M3D_FLOAT)255.0;
                    break;
                    case 2:
                        model->tmap[i].u = (M3D_FLOAT)(*((uint16_t*)(data+0))) / (M3D_FLOAT)65535.0;
                        model->tmap[i].v = (M3D_FLOAT)(*((uint16_t*)(data+2))) / (M3D_FLOAT)65535.0;
                    break;
                    case 4:
                        model->tmap[i].u = (M3D_FLOAT)(*((float*)(data+0)));
                        model->tmap[i].v = (M3D_FLOAT)(*((float*)(data+4)));
                    break;
                    case 8:
                        model->tmap[i].u = (M3D_FLOAT)(*((double*)(data+0)));
                        model->tmap[i].v = (M3D_FLOAT)(*((double*)(data+8)));
                    break;
                }
                data += reclen;
            }
        } else
        /* vertex list */
        if(M3D_CHUNKMAGIC(data, 'V','R','T','S')) {
            M3D_LOG("Vertex list");
            if(model->vertex) { M3D_LOG("More vertex chunks, should be unique"); model->errcode = M3D_ERR_VRTS; continue; }
            if(model->ci_s && model->ci_s < 4 && !model->cmap) model->errcode = M3D_ERR_CMAP;
            reclen = model->ci_s + model->sk_s + 4 * model->vc_s;
            model->numvertex = len / reclen;
            model->vertex = (m3dv_t*)M3D_MALLOC(model->numvertex * sizeof(m3dv_t));
            if(!model->vertex) goto memerr;
            memset(model->vertex, 0, model->numvertex * sizeof(m3dv_t));
            for(i = 0, data += sizeof(m3dchunk_t); data < chunk && i < model->numvertex; i++) {
                switch(model->vc_s) {
                    case 1:
                        model->vertex[i].x = (M3D_FLOAT)((int8_t)data[0]) / (M3D_FLOAT)127.0;
                        model->vertex[i].y = (M3D_FLOAT)((int8_t)data[1]) / (M3D_FLOAT)127.0;
                        model->vertex[i].z = (M3D_FLOAT)((int8_t)data[2]) / (M3D_FLOAT)127.0;
                        model->vertex[i].w = (M3D_FLOAT)((int8_t)data[3]) / (M3D_FLOAT)127.0;
                        data += 4;
                    break;
                    case 2:
                        model->vertex[i].x = (M3D_FLOAT)(*((int16_t*)(data+0))) / (M3D_FLOAT)32767.0;
                        model->vertex[i].y = (M3D_FLOAT)(*((int16_t*)(data+2))) / (M3D_FLOAT)32767.0;
                        model->vertex[i].z = (M3D_FLOAT)(*((int16_t*)(data+4))) / (M3D_FLOAT)32767.0;
                        model->vertex[i].w = (M3D_FLOAT)(*((int16_t*)(data+6))) / (M3D_FLOAT)32767.0;
                        data += 8;
                    break;
                    case 4:
                        model->vertex[i].x = (M3D_FLOAT)(*((float*)(data+0)));
                        model->vertex[i].y = (M3D_FLOAT)(*((float*)(data+4)));
                        model->vertex[i].z = (M3D_FLOAT)(*((float*)(data+8)));
                        model->vertex[i].w = (M3D_FLOAT)(*((float*)(data+12)));
                        data += 16;
                    break;
                    case 8:
                        model->vertex[i].x = (M3D_FLOAT)(*((double*)(data+0)));
                        model->vertex[i].y = (M3D_FLOAT)(*((double*)(data+8)));
                        model->vertex[i].z = (M3D_FLOAT)(*((double*)(data+16)));
                        model->vertex[i].w = (M3D_FLOAT)(*((double*)(data+24)));
                        data += 32;
                    break;
                }
                switch(model->ci_s) {
                    case 1: model->vertex[i].color = model->cmap ? model->cmap[data[0]] : 0; data++; break;
                    case 2: model->vertex[i].color = model->cmap ? model->cmap[*((uint16_t*)data)] : 0; data += 2; break;
                    case 4: model->vertex[i].color = *((uint32_t*)data); data += 4; break;
                    /* case 8: break; */
                }
                model->vertex[i].skinid = M3D_UNDEF;
                data = _m3d_getidx(data, model->sk_s, &model->vertex[i].skinid);
            }
        } else
        /* skeleton: bone hierarchy and skin */
        if(M3D_CHUNKMAGIC(data, 'B','O','N','E')) {
            M3D_LOG("Skeleton");
            if(model->bone) { M3D_LOG("More bone chunks, should be unique"); model->errcode = M3D_ERR_BONE; continue; }
            if(!model->bi_s) { M3D_LOG("Bone chunk, shouldn't be any"); model->errcode=M3D_ERR_BONE; continue; }
            if(!model->vertex) { M3D_LOG("No vertex chunk before bones"); model->errcode = M3D_ERR_VRTS; break; }
            data += sizeof(m3dchunk_t);
            model->numbone = 0;
            data = _m3d_getidx(data, model->bi_s, &model->numbone);
            if(model->numbone) {
                model->bone = (m3db_t*)M3D_MALLOC(model->numbone * sizeof(m3db_t));
                if(!model->bone) goto memerr;
            }
            model->numskin = 0;
            data = _m3d_getidx(data, model->sk_s, &model->numskin);
            /* read bone hierarchy */
            for(i = 0; data < chunk && i < model->numbone; i++) {
                data = _m3d_getidx(data, model->bi_s, &model->bone[i].parent);
                M3D_GETSTR(model->bone[i].name);
                data = _m3d_getidx(data, model->vi_s, &model->bone[i].pos);
                data = _m3d_getidx(data, model->vi_s, &model->bone[i].ori);
                model->bone[i].numweight = 0;
                model->bone[i].weight = NULL;
            }
            /* read skin definitions */
            if(model->numskin) {
                model->skin = (m3ds_t*)M3D_MALLOC(model->numskin * sizeof(m3ds_t));
                if(!model->skin) goto memerr;
                for(i = 0; data < chunk && i < model->numskin; i++) {
                    for(j = 0; j < M3D_NUMBONE; j++) {
                        model->skin[i].boneid[j] = M3D_UNDEF;
                        model->skin[i].weight[j] = (M3D_FLOAT)0.0;
                    }
                    memset(&weights, 0, sizeof(weights));
                    if(model->nb_s == 1) weights[0] = 255;
                    else {
                        memcpy(&weights, data, model->nb_s);
                        data += model->nb_s;
                    }
                    for(j = 0, w = (M3D_FLOAT)0.0; j < (unsigned int)model->nb_s; j++) {
                        if(weights[j]) {
                            if(j >= M3D_NUMBONE)
                                data += model->bi_s;
                            else {
                                model->skin[i].weight[j] = (M3D_FLOAT)(weights[j]) / (M3D_FLOAT)255.0;
                                w += model->skin[i].weight[j];
                                data = _m3d_getidx(data, model->bi_s, &model->skin[i].boneid[j]);
                            }
                        }
                    }
                    /* this can occur if model has more bones than what the importer is configured to handle */
                    if(w != (M3D_FLOAT)1.0 && w != (M3D_FLOAT)0.0) {
                        for(j = 0; j < M3D_NUMBONE; j++)
                            model->skin[i].weight[j] /= w;
                    }
                }
            }
        } else
        /* material */
        if(M3D_CHUNKMAGIC(data, 'M','T','R','L')) {
            data += sizeof(m3dchunk_t);
            M3D_GETSTR(name);
            M3D_LOG("Material");
            M3D_LOG(name);
            if(model->ci_s < 4 && !model->numcmap) model->errcode = M3D_ERR_CMAP;
            for(i = 0; i < model->nummaterial; i++)
                if(!strcmp(name, model->material[i].name)) {
                    model->errcode = M3D_ERR_MTRL;
                    M3D_LOG("Multiple definitions for material");
                    M3D_LOG(name);
                    name = NULL;
                    break;
                }
            if(name) {
                i = model->nummaterial++;
                if(model->flags & M3D_FLG_MTLLIB) {
                    m = model->material;
                    model->material = (m3dm_t*)M3D_MALLOC(model->nummaterial * sizeof(m3dm_t));
                    if(!model->material) goto memerr;
                    memcpy(model->material, m, (model->nummaterial - 1) * sizeof(m3dm_t));
                    if(model->texture) {
                        tx = model->texture;
                        model->texture = (m3dtx_t*)M3D_MALLOC(model->numtexture * sizeof(m3dtx_t));
                        if(!model->texture) goto memerr;
                        memcpy(model->texture, tx, model->numtexture * sizeof(m3dm_t));
                    }
                    model->flags &= ~M3D_FLG_MTLLIB;
                } else {
                    model->material = (m3dm_t*)M3D_REALLOC(model->material, model->nummaterial * sizeof(m3dm_t));
                    if(!model->material) goto memerr;
                }
                m = &model->material[i];
                m->numprop = 0;
                m->name = name;
                m->prop = (m3dp_t*)M3D_MALLOC((len / 2) * sizeof(m3dp_t));
                if(!m->prop) goto memerr;
                while(data < chunk) {
                    i = m->numprop++;
                    m->prop[i].type = *data++;
                    m->prop[i].value.num = 0;
                    if(m->prop[i].type >= 128)
                        k = m3dpf_map;
                    else {
                        for(k = 256, j = 0; j < sizeof(m3d_propertytypes)/sizeof(m3d_propertytypes[0]); j++)
                            if(m->prop[i].type == m3d_propertytypes[j].id) { k = m3d_propertytypes[j].format; break; }
                    }
                    switch(k) {
                        case m3dpf_color:
                            switch(model->ci_s) {
                                case 1: m->prop[i].value.color = model->cmap ? model->cmap[data[0]] : 0; data++; break;
                                case 2: m->prop[i].value.color = model->cmap ? model->cmap[*((uint16_t*)data)] : 0; data += 2; break;
                                case 4: m->prop[i].value.color = *((uint32_t*)data); data += 4; break;
                            }
                        break;

                        case m3dpf_uint8: m->prop[i].value.num = *data++; break;
                        case m3dpf_uint16:m->prop[i].value.num = *((uint16_t*)data); data += 2; break;
                        case m3dpf_uint32:m->prop[i].value.num = *((uint32_t*)data); data += 4; break;
                        case m3dpf_float: m->prop[i].value.fnum = *((float*)data); data += 4; break;

                        case m3dpf_map:
                            M3D_GETSTR(name);
                            m->prop[i].value.textureid = _m3d_gettx(model, readfilecb, freecb, name);
                            if(model->errcode == M3D_ERR_ALLOC) goto memerr;
                            /* this error code only returned if readfilecb was specified */
                            if(m->prop[i].value.textureid == M3D_UNDEF) {
                                M3D_LOG("Texture not found");
                                M3D_LOG(m->name);
                                m->numprop--;
                            }
                        break;

                        default:
                            M3D_LOG("Unknown material property in");
                            M3D_LOG(m->name);
                            model->errcode = M3D_ERR_UNKPROP;
                            data = chunk;
                        break;
                    }
                }
                m->prop = (m3dp_t*)M3D_REALLOC(m->prop, m->numprop * sizeof(m3dp_t));
                if(!m->prop) goto memerr;
            }
        } else
        /* face */
        if(M3D_CHUNKMAGIC(data, 'P','R','O','C')) {
            /* procedural surface */
            M3D_GETSTR(name);
            M3D_LOG("Procedural surface");
            M3D_LOG(name);
            _m3d_getpr(model, readfilecb, freecb, name);
        } else
        if(M3D_CHUNKMAGIC(data, 'M','E','S','H')) {
            M3D_LOG("Mesh data");
            if(!model->vertex) { M3D_LOG("No vertex chunk before mesh"); model->errcode = M3D_ERR_VRTS; }
            /* mesh */
            data += sizeof(m3dchunk_t);
            mi = M3D_UNDEF;
#ifdef M3D_VERTEXMAX
            pi = M3D_UNDEF;
#endif
            am = model->numface;
            while(data < chunk) {
                k = *data++;
                n = k >> 4;
                k &= 15;
                if(!n) {
                    if(!k) {
                        /* use material */
                        mi = M3D_UNDEF;
                        M3D_GETSTR(name);
                        if(name) {
                            for(j = 0; j < model->nummaterial; j++)
                                if(!strcmp(name, model->material[j].name)) {
                                    mi = (M3D_INDEX)j;
                                    break;
                                }
                            if(mi == M3D_UNDEF) model->errcode = M3D_ERR_MTRL;
                        }
                    } else {
                        /* use parameter */
                        M3D_GETSTR(name);
#ifdef M3D_VERTEXMAX
                        pi = M3D_UNDEF;
                        if(name) {
                            for(j = 0; j < model->numparam; j++)
                                if(!strcmp(name, model->param[j].name)) {
                                    pi = (M3D_INDEX)j;
                                    break;
                                }
                            if(pi == M3D_UNDEF) {
                                pi = model->numparam++;
                                model->param = (m3dvi_t*)M3D_REALLOC(model->param, model->numparam * sizeof(m3dvi_t));
                                if(!model->param) goto memerr;
                                model->param[pi].name = name;
                                model->param[pi].count = 0;
                            }
                        }
#endif
                    }
                    continue;
                }
                if(n != 3) { M3D_LOG("Only triangle mesh supported for now"); model->errcode = M3D_ERR_UNKMESH; return model; }
                i = model->numface++;
                if(model->numface > am) {
                    am = model->numface + 4095;
                    model->face = (m3df_t*)M3D_REALLOC(model->face, am * sizeof(m3df_t));
                    if(!model->face) goto memerr;
                }
                memset(&model->face[i], 255, sizeof(m3df_t)); /* set all index to -1 by default */
                model->face[i].materialid = mi;
#ifdef M3D_VERTEXMAX
                model->face[i].paramid = pi;
#endif
                for(j = 0; data < chunk && j < n; j++) {
                    /* vertex */
                    data = _m3d_getidx(data, model->vi_s, &model->face[i].vertex[j]);
                    /* texcoord */
                    if(k & 1)
                        data = _m3d_getidx(data, model->ti_s, &model->face[i].texcoord[j]);
                    /* normal */
                    if(k & 2)
                        data = _m3d_getidx(data, model->vi_s, &model->face[i].normal[j]);
#ifndef M3D_NONORMALS
                    if(model->face[i].normal[j] == M3D_UNDEF) neednorm = 1;
#endif
                    /* maximum */
                    if(k & 4)
#ifdef M3D_VERTEXMAX
                        data = _m3d_getidx(data, model->vi_s, &model->face[i].vertmax[j]);
#else
                        data += model->vi_s;
#endif
                }
                if(j != n) { M3D_LOG("Invalid mesh"); model->numface = 0; model->errcode = M3D_ERR_UNKMESH; return model; }
            }
            model->face = (m3df_t*)M3D_REALLOC(model->face, model->numface * sizeof(m3df_t));
        } else
        if(M3D_CHUNKMAGIC(data, 'V','O','X','T')) {
            /* voxel types */
            M3D_LOG("Voxel types list");
            if(model->voxtype) { M3D_LOG("More voxel type chunks, should be unique"); model->errcode = M3D_ERR_VOXT; continue; }
            if(model->ci_s && model->ci_s < 4 && !model->cmap) model->errcode = M3D_ERR_CMAP;
            reclen = model->ci_s + model->si_s + 3 + model->sk_s;
            k = len / reclen;
            model->voxtype = (m3dvt_t*)M3D_MALLOC(k * sizeof(m3dvt_t));
            if(!model->voxtype) goto memerr;
            memset(model->voxtype, 0, k * sizeof(m3dvt_t));
            model->numvoxtype = 0;
            for(i = 0, data += sizeof(m3dchunk_t); data < chunk && i < k; i++) {
                switch(model->ci_s) {
                    case 1: model->voxtype[i].color = model->cmap ? model->cmap[data[0]] : 0; data++; break;
                    case 2: model->voxtype[i].color = model->cmap ? model->cmap[*((uint16_t*)data)] : 0; data += 2; break;
                    case 4: model->voxtype[i].color = *((uint32_t*)data); data += 4; break;
                    /* case 8: break; */
                }
                M3D_GETSTR(name);
                model->voxtype[i].materialid = M3D_UNDEF;
                if(name) {
                    model->voxtype[i].name = name;
/*
                    for(j = 0; j < model->nummaterial; j++)
                        if(!strcmp(name, model->material[j].name)) {
                            model->voxtype[i].materialid = (M3D_INDEX)j;
                            break;
                        }
*/
                }
                j = *data++;
                model->voxtype[i].rotation = j & 0xBF;
                model->voxtype[i].voxshape = ((j & 0x40) << 2) | *data++;
                model->voxtype[i].numitem = *data++;
                model->voxtype[i].skinid = M3D_UNDEF;
                data = _m3d_getidx(data, model->sk_s, &model->voxtype[i].skinid);
                if(model->voxtype[i].numitem) {
                    model->voxtype[i].item = (m3dvi_t*)M3D_MALLOC(model->voxtype[i].numitem * sizeof(m3dvi_t));
                    if(!model->voxtype[i].item) goto memerr;
                    memset(model->voxtype[i].item, 0, model->voxtype[i].numitem * sizeof(m3dvi_t));
                    for(j = 0; j < model->voxtype[i].numitem; j++) {
                        model->voxtype[i].item[j].count = *data++;
                        model->voxtype[i].item[j].count |= (*data++) << 8;
                        M3D_GETSTR(model->voxtype[i].item[j].name);
                    }
                }
            }
            model->numvoxtype = i;
            if(k != model->numvoxtype) {
                model->voxtype = (m3dvt_t*)M3D_REALLOC(model->voxtype, model->numvoxtype * sizeof(m3dvt_t));
                if(!model->voxtype) goto memerr;
            }
        } else
        if(M3D_CHUNKMAGIC(data, 'V','O','X','D')) {
            /* voxel data */
            data += sizeof(m3dchunk_t);
            M3D_GETSTR(name);
            M3D_LOG("Voxel Data Layer");
            M3D_LOG(name);
            if(model->vd_s > 4 || model->vp_s > 2) { M3D_LOG("No voxel index size"); model->errcode = M3D_ERR_UNKVOX; continue; }
            if(!model->voxtype) { M3D_LOG("No voxel type chunk before voxel data"); model->errcode = M3D_ERR_VOXT; }
            i = model->numvoxel++;
            model->voxel = (m3dvx_t*)M3D_REALLOC(model->voxel, model->numvoxel * sizeof(m3dvx_t));
            if(!model->voxel) goto memerr;
            memset(&model->voxel[i], 0, sizeof(m3dvx_t));
            model->voxel[i].name = name;
            switch(model->vd_s) {
                case 1:
                    model->voxel[i].x = (int32_t)((int8_t)data[0]);
                    model->voxel[i].y = (int32_t)((int8_t)data[1]);
                    model->voxel[i].z = (int32_t)((int8_t)data[2]);
                    model->voxel[i].w = (uint32_t)(data[3]);
                    model->voxel[i].h = (uint32_t)(data[4]);
                    model->voxel[i].d = (uint32_t)(data[5]);
                    data += 6;
                break;
                case 2:
                    model->voxel[i].x = (int32_t)(*((int16_t*)(data+0)));
                    model->voxel[i].y = (int32_t)(*((int16_t*)(data+2)));
                    model->voxel[i].z = (int32_t)(*((int16_t*)(data+4)));
                    model->voxel[i].w = (uint32_t)(*((uint16_t*)(data+6)));
                    model->voxel[i].h = (uint32_t)(*((uint16_t*)(data+8)));
                    model->voxel[i].d = (uint32_t)(*((uint16_t*)(data+10)));
                    data += 12;
                break;
                case 4:
                    model->voxel[i].x = *((int32_t*)(data+0));
                    model->voxel[i].y = *((int32_t*)(data+4));
                    model->voxel[i].z = *((int32_t*)(data+8));
                    model->voxel[i].w = *((uint32_t*)(data+12));
                    model->voxel[i].h = *((uint32_t*)(data+16));
                    model->voxel[i].d = *((uint32_t*)(data+20));
                    data += 24;
                break;
            }
            model->voxel[i].uncertain = *data++;
            model->voxel[i].groupid = *data++;
            k = model->voxel[i].w * model->voxel[i].h * model->voxel[i].d;
            model->voxel[i].data = (M3D_VOXEL*)M3D_MALLOC(k * sizeof(M3D_VOXEL));
            if(!model->voxel[i].data) goto memerr;
            memset(model->voxel[i].data, 0xff, k * sizeof(M3D_VOXEL));
            for(j = 0; data < chunk && j < k;) {
                l = ((*data++) & 0x7F) + 1;
                if(data[-1] & 0x80) {
                    data = _m3d_getidx(data, model->vp_s, &mi);
                    while(l-- && j < k) model->voxel[i].data[j++] = (M3D_VOXEL)mi;
                } else
                    while(l-- && j < k) {
                        data = _m3d_getidx(data, model->vp_s, &mi);
                        model->voxel[i].data[j++] = (M3D_VOXEL)mi;
                    }
            }
        } else
        if(M3D_CHUNKMAGIC(data, 'S','H','P','E')) {
            /* mathematical shape */
            data += sizeof(m3dchunk_t);
            M3D_GETSTR(name);
            M3D_LOG("Mathematical Shape");
            M3D_LOG(name);
            i = model->numshape++;
            model->shape = (m3dh_t*)M3D_REALLOC(model->shape, model->numshape * sizeof(m3dh_t));
            if(!model->shape) goto memerr;
            h = &model->shape[i];
            h->numcmd = 0;
            h->cmd = NULL;
            h->name = name;
            h->group = M3D_UNDEF;
            data = _m3d_getidx(data, model->bi_s, &h->group);
            if(h->group != M3D_UNDEF && h->group >= model->numbone) {
                M3D_LOG("Unknown bone id as shape group in shape");
                M3D_LOG(name);
                h->group = M3D_UNDEF;
                model->errcode = M3D_ERR_SHPE;
            }
            while(data < chunk) {
                i = h->numcmd++;
                h->cmd = (m3dc_t*)M3D_REALLOC(h->cmd, h->numcmd * sizeof(m3dc_t));
                if(!h->cmd) goto memerr;
                h->cmd[i].type = *data++;
                if(h->cmd[i].type & 0x80) {
                    h->cmd[i].type &= 0x7F;
                    h->cmd[i].type |= (*data++ << 7);
                }
                if(h->cmd[i].type >= (unsigned int)(sizeof(m3d_commandtypes)/sizeof(m3d_commandtypes[0]))) {
                    M3D_LOG("Unknown shape command in");
                    M3D_LOG(h->name);
                    model->errcode = M3D_ERR_UNKCMD;
                    break;
                }
                cd = &m3d_commandtypes[h->cmd[i].type];
                h->cmd[i].arg = (uint32_t*)M3D_MALLOC(cd->p * sizeof(uint32_t));
                if(!h->cmd[i].arg) goto memerr;
                memset(h->cmd[i].arg, 0, cd->p * sizeof(uint32_t));
                for(k = n = 0, l = cd->p; k < l; k++)
                    switch(cd->a[((k - n) % (cd->p - n)) + n]) {
                        case m3dcp_mi_t:
                            h->cmd[i].arg[k] = M3D_NOTDEFINED;
                            M3D_GETSTR(name);
                            if(name) {
                                for(n = 0; n < model->nummaterial; n++)
                                    if(!strcmp(name, model->material[n].name)) {
                                        h->cmd[i].arg[k] = n;
                                        break;
                                    }
                                if(h->cmd[i].arg[k] == M3D_NOTDEFINED) model->errcode = M3D_ERR_MTRL;
                            }
                        break;
                        case m3dcp_vc_t:
                            f = 0.0f;
                            switch(model->vc_s) {
                                case 1: f = (float)((int8_t)data[0]) / 127; break;
                                case 2: f = (float)(*((int16_t*)(data+0))) / 32767; break;
                                case 4: f = (float)(*((float*)(data+0))); break;
                                case 8: f = (float)(*((double*)(data+0))); break;
                            }
                            memcpy(&h->cmd[i].arg[k], &f, 4);
                            data += model->vc_s;
                        break;
                        case m3dcp_hi_t: data = _m3d_getidx(data, model->hi_s, &h->cmd[i].arg[k]); break;
                        case m3dcp_fi_t: data = _m3d_getidx(data, model->fi_s, &h->cmd[i].arg[k]); break;
                        case m3dcp_ti_t: data = _m3d_getidx(data, model->ti_s, &h->cmd[i].arg[k]); break;
                        case m3dcp_qi_t:
                        case m3dcp_vi_t: data = _m3d_getidx(data, model->vi_s, &h->cmd[i].arg[k]); break;
                        case m3dcp_i1_t: data = _m3d_getidx(data, 1, &h->cmd[i].arg[k]); break;
                        case m3dcp_i2_t: data = _m3d_getidx(data, 2, &h->cmd[i].arg[k]); break;
                        case m3dcp_i4_t: data = _m3d_getidx(data, 4, &h->cmd[i].arg[k]); break;
                        case m3dcp_va_t: data = _m3d_getidx(data, 4, &h->cmd[i].arg[k]);
                            n = k + 1; l += (h->cmd[i].arg[k] - 1) * (cd->p - k - 1);
                            h->cmd[i].arg = (uint32_t*)M3D_REALLOC(h->cmd[i].arg, l * sizeof(uint32_t));
                            if(!h->cmd[i].arg) goto memerr;
                            memset(&h->cmd[i].arg[k + 1], 0, (l - k - 1) * sizeof(uint32_t));
                        break;
                    }
            }
        } else
        /* annotation label list */
        if(M3D_CHUNKMAGIC(data, 'L','B','L','S')) {
            data += sizeof(m3dchunk_t);
            M3D_GETSTR(name);
            M3D_GETSTR(lang);
            M3D_LOG("Label list");
            if(name) { M3D_LOG(name); }
            if(lang) { M3D_LOG(lang); }
            if(model->ci_s && model->ci_s < 4 && !model->cmap) model->errcode = M3D_ERR_CMAP;
            k = 0;
            switch(model->ci_s) {
                case 1: k = model->cmap ? model->cmap[data[0]] : 0; data++; break;
                case 2: k = model->cmap ? model->cmap[*((uint16_t*)data)] : 0; data += 2; break;
                case 4: k = *((uint32_t*)data); data += 4; break;
                /* case 8: break; */
            }
            reclen = model->vi_s + model->si_s;
            i = model->numlabel; model->numlabel += len / reclen;
            model->label = (m3dl_t*)M3D_REALLOC(model->label, model->numlabel * sizeof(m3dl_t));
            if(!model->label) goto memerr;
            memset(&model->label[i], 0, (model->numlabel - i) * sizeof(m3dl_t));
            for(; data < chunk && i < model->numlabel; i++) {
                model->label[i].name = name;
                model->label[i].lang = lang;
                model->label[i].color = k;
                data = _m3d_getidx(data, model->vi_s, &model->label[i].vertexid);
                M3D_GETSTR(model->label[i].text);
            }
        } else
        /* action */
        if(M3D_CHUNKMAGIC(data, 'A','C','T','N')) {
            M3D_LOG("Action");
            i = model->numaction++;
            model->action = (m3da_t*)M3D_REALLOC(model->action, model->numaction * sizeof(m3da_t));
            if(!model->action) goto memerr;
            a = &model->action[i];
            data += sizeof(m3dchunk_t);
            M3D_GETSTR(a->name);
            M3D_LOG(a->name);
            a->numframe = *((uint16_t*)data); data += 2;
            if(a->numframe < 1) {
                model->numaction--;
            } else {
                a->durationmsec = *((uint32_t*)data); data += 4;
                a->frame = (m3dfr_t*)M3D_MALLOC(a->numframe * sizeof(m3dfr_t));
                if(!a->frame) goto memerr;
                for(i = 0; data < chunk && i < a->numframe; i++) {
                    a->frame[i].msec = *((uint32_t*)data); data += 4;
                    a->frame[i].numtransform = 0; a->frame[i].transform = NULL;
                    data = _m3d_getidx(data, model->fc_s, &a->frame[i].numtransform);
                    if(a->frame[i].numtransform > 0) {
                        a->frame[i].transform = (m3dtr_t*)M3D_MALLOC(a->frame[i].numtransform * sizeof(m3dtr_t));
                        for(j = 0; j < a->frame[i].numtransform; j++) {
                            data = _m3d_getidx(data, model->bi_s, &a->frame[i].transform[j].boneid);
                            data = _m3d_getidx(data, model->vi_s, &a->frame[i].transform[j].pos);
                            data = _m3d_getidx(data, model->vi_s, &a->frame[i].transform[j].ori);
                        }
                    }
                }
            }
        } else {
            i = model->numextra++;
            model->extra = (m3dchunk_t**)M3D_REALLOC(model->extra, model->numextra * sizeof(m3dchunk_t*));
            if(!model->extra) goto memerr;
            model->extra[i] = (m3dchunk_t*)data;
        }
    }
    /* calculate normals, normalize skin weights, create bone/vertex cross-references and calculate transform matrices */
#ifdef M3D_ASCII
postprocess:
#endif
    if(model) {
        M3D_LOG("Post-process");
#ifdef M3D_PROFILING
        gettimeofday(&tv1, NULL);
        tvd.tv_sec = tv1.tv_sec - tv0.tv_sec;
        tvd.tv_usec = tv1.tv_usec - tv0.tv_usec;
        if(tvd.tv_usec < 0) { tvd.tv_sec--; tvd.tv_usec += 1000000L; }
        printf("  Parsing chunks  %ld.%06ld sec\n", tvd.tv_sec, tvd.tv_usec);
#endif
#ifndef M3D_NOVOXELS
        if(model->numvoxel && model->voxel) {
            M3D_LOG("Converting voxels into vertices and mesh");
            /* add normals */
            enorm = model->numvertex; model->numvertex += 6;
            model->vertex = (m3dv_t*)M3D_REALLOC(model->vertex, model->numvertex * sizeof(m3dv_t));
            if(!model->vertex) goto memerr;
            memset(&model->vertex[enorm], 0, 6 * sizeof(m3dv_t));
            for(l = 0; l < 6; l++)
                model->vertex[enorm+l].skinid = M3D_UNDEF;
            model->vertex[enorm+0].y = (M3D_FLOAT)-1.0;
            model->vertex[enorm+1].z = (M3D_FLOAT)-1.0;
            model->vertex[enorm+2].x = (M3D_FLOAT)-1.0;
            model->vertex[enorm+3].y = (M3D_FLOAT)1.0;
            model->vertex[enorm+4].z = (M3D_FLOAT)1.0;
            model->vertex[enorm+5].x = (M3D_FLOAT)1.0;
            /* this is a fast, not so memory efficient version, only basic face culling used */
            min_x = min_y = min_z = 2147483647L;
            max_x = max_y = max_z = -2147483647L;
            for(i = 0; i < model->numvoxel; i++) {
                if(model->voxel[i].x + (int32_t)model->voxel[i].w > max_x) max_x = model->voxel[i].x + (int32_t)model->voxel[i].w;
                if(model->voxel[i].x < min_x) min_x = model->voxel[i].x;
                if(model->voxel[i].y + (int32_t)model->voxel[i].h > max_y) max_y = model->voxel[i].y + (int32_t)model->voxel[i].h;
                if(model->voxel[i].y < min_y) min_y = model->voxel[i].y;
                if(model->voxel[i].z + (int32_t)model->voxel[i].d > max_z) max_z = model->voxel[i].z + (int32_t)model->voxel[i].d;
                if(model->voxel[i].z < min_z) min_z = model->voxel[i].z;
            }
            i = (-min_x > max_x ? -min_x : max_x);
            j = (-min_y > max_y ? -min_y : max_y);
            k = (-min_z > max_z ? -min_z : max_z);
            if(j > i) i = j;
            if(k > i) i = k;
            if(i <= 1) i = 1;
            w = (M3D_FLOAT)1.0 / (M3D_FLOAT)i;
            if(i >= 254) model->vc_s = 2;
            if(i >= 65534) model->vc_s = 4;
            for(i = 0; i < model->numvoxel; i++) {
                sx = model->voxel[i].w; sz = model->voxel[i].d; sy = model->voxel[i].h;
                for(y = 0, j = 0; y < sy; y++)
                    for(z = 0; z < sz; z++)
                        for(x = 0; x < sx; x++, j++)
                            if(model->voxel[i].data[j] < model->numvoxtype) {
                                k = 0;
                                /*  16__32     ____
                                 *  /|  /|    /|2 /|
                                 *64_128 |   /_8_/ 32
                                 * | 1_|_2   |4|_|_|
                                 * |/  |/    |/ 1|/
                                 * 4___8     |16_|    */
                                k = n = am = 0;
                                if(!y || model->voxel[i].data[j - sx*sz] >= model->numvoxtype) { n++; am |= 1; k |= 1|2|4|8; }
                                if(!z || model->voxel[i].data[j - sx] >= model->numvoxtype) { n++; am |= 2; k |= 1|2|16|32; }
                                if(!x || model->voxel[i].data[j - 1] >= model->numvoxtype) { n++; am |= 4; k |= 1|4|16|64; }
                                if(y == sy-1 || model->voxel[i].data[j + sx*sz] >= model->numvoxtype) { n++; am |= 8; k |= 16|32|64|128; }
                                if(z == sz-1 || model->voxel[i].data[j + sx] >= model->numvoxtype) { n++; am |= 16; k |= 4|8|64|128; }
                                if(x == sx-1 || model->voxel[i].data[j + 1] >= model->numvoxtype) { n++; am |= 32; k |= 2|8|32|128; }
                                if(k) {
                                    memset(edge, 255, sizeof(edge));
                                    for(l = 0, len = 1, reclen = model->numvertex; l < 8; l++, len <<= 1)
                                        if(k & len) edge[l] = model->numvertex++;
                                    model->vertex = (m3dv_t*)M3D_REALLOC(model->vertex, model->numvertex * sizeof(m3dv_t));
                                    if(!model->vertex) goto memerr;
                                    memset(&model->vertex[reclen], 0, (model->numvertex-reclen) * sizeof(m3dv_t));
                                    for(l = reclen; l < model->numvertex; l++) {
                                        model->vertex[l].skinid = model->voxtype[model->voxel[i].data[j]].skinid;
                                        model->vertex[l].color = model->voxtype[model->voxel[i].data[j]].color;
                                    }
                                    l = reclen;
                                    if(k & 1) {
                                        model->vertex[l].x = (model->voxel[i].x + x) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z) * w;
                                        l++;
                                    }
                                    if(k & 2) {
                                        model->vertex[l].x = (model->voxel[i].x + x + 1) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z) * w;
                                        l++;
                                    }
                                    if(k & 4) {
                                        model->vertex[l].x = (model->voxel[i].x + x) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z + 1) * w;
                                        l++;
                                    }
                                    if(k & 8) {
                                        model->vertex[l].x = (model->voxel[i].x + x + 1) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z + 1) * w;
                                        l++;
                                    }
                                    if(k & 16) {
                                        model->vertex[l].x = (model->voxel[i].x + x) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y + 1) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z) * w;
                                        l++;
                                    }
                                    if(k & 32) {
                                        model->vertex[l].x = (model->voxel[i].x + x + 1) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y + 1) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z) * w;
                                        l++;
                                    }
                                    if(k & 64) {
                                        model->vertex[l].x = (model->voxel[i].x + x) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y + 1) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z + 1) * w;
                                        l++;
                                    }
                                    if(k & 128) {
                                        model->vertex[l].x = (model->voxel[i].x + x + 1) * w;
                                        model->vertex[l].y = (model->voxel[i].y + y + 1) * w;
                                        model->vertex[l].z = (model->voxel[i].z + z + 1) * w;
                                        l++;
                                    }
                                    n <<= 1;
                                    l = model->numface; model->numface += n;
                                    model->face = (m3df_t*)M3D_REALLOC(model->face, model->numface * sizeof(m3df_t));
                                    if(!model->face) goto memerr;
                                    memset(&model->face[l], 255, n * sizeof(m3df_t));
                                    for(reclen = l; reclen < model->numface; reclen++)
                                        model->face[reclen].materialid = model->voxtype[model->voxel[i].data[j]].materialid;
                                    if(am & 1) {            /* bottom */
                                        model->face[l].vertex[0] = edge[0];   model->face[l].vertex[1] = edge[1];   model->face[l].vertex[2] = edge[2];
                                        model->face[l+1].vertex[0] = edge[2]; model->face[l+1].vertex[1] = edge[1]; model->face[l+1].vertex[2] = edge[3];
                                        model->face[l].normal[0] = model->face[l].normal[1] = model->face[l].normal[2] =
                                        model->face[l+1].normal[0] = model->face[l+1].normal[1] = model->face[l+1].normal[2] = enorm;
                                        l += 2;
                                    }
                                    if(am & 2) {            /* north */
                                        model->face[l].vertex[0] = edge[0];   model->face[l].vertex[1] = edge[4];   model->face[l].vertex[2] = edge[1];
                                        model->face[l+1].vertex[0] = edge[1]; model->face[l+1].vertex[1] = edge[4]; model->face[l+1].vertex[2] = edge[5];
                                        model->face[l].normal[0] = model->face[l].normal[1] = model->face[l].normal[2] =
                                        model->face[l+1].normal[0] = model->face[l+1].normal[1] = model->face[l+1].normal[2] = enorm+1;
                                        l += 2;
                                    }
                                    if(am & 4) {            /* west */
                                        model->face[l].vertex[0] = edge[0];   model->face[l].vertex[1] = edge[2];   model->face[l].vertex[2] = edge[4];
                                        model->face[l+1].vertex[0] = edge[2]; model->face[l+1].vertex[1] = edge[6]; model->face[l+1].vertex[2] = edge[4];
                                        model->face[l].normal[0] = model->face[l].normal[1] = model->face[l].normal[2] =
                                        model->face[l+1].normal[0] = model->face[l+1].normal[1] = model->face[l+1].normal[2] = enorm+2;
                                        l += 2;
                                    }
                                    if(am & 8) {            /* top */
                                        model->face[l].vertex[0] = edge[4];   model->face[l].vertex[1] = edge[6];   model->face[l].vertex[2] = edge[5];
                                        model->face[l+1].vertex[0] = edge[5]; model->face[l+1].vertex[1] = edge[6]; model->face[l+1].vertex[2] = edge[7];
                                        model->face[l].normal[0] = model->face[l].normal[1] = model->face[l].normal[2] =
                                        model->face[l+1].normal[0] = model->face[l+1].normal[1] = model->face[l+1].normal[2] = enorm+3;
                                        l += 2;
                                    }
                                    if(am & 16) {           /* south */
                                        model->face[l].vertex[0] = edge[2];   model->face[l].vertex[1] = edge[7];   model->face[l].vertex[2] = edge[6];
                                        model->face[l+1].vertex[0] = edge[7]; model->face[l+1].vertex[1] = edge[2]; model->face[l+1].vertex[2] = edge[3];
                                        model->face[l].normal[0] = model->face[l].normal[1] = model->face[l].normal[2] =
                                        model->face[l+1].normal[0] = model->face[l+1].normal[1] = model->face[l+1].normal[2] = enorm+4;
                                        l += 2;
                                    }
                                    if(am & 32) {           /* east */
                                        model->face[l].vertex[0] = edge[1];   model->face[l].vertex[1] = edge[5];   model->face[l].vertex[2] = edge[7];
                                        model->face[l+1].vertex[0] = edge[1]; model->face[l+1].vertex[1] = edge[7]; model->face[l+1].vertex[2] = edge[3];
                                        model->face[l].normal[0] = model->face[l].normal[1] = model->face[l].normal[2] =
                                        model->face[l+1].normal[0] = model->face[l+1].normal[1] = model->face[l+1].normal[2] = enorm+5;
                                        l += 2;
                                    }
                                }
                            }
            }
        }
#endif
#ifndef M3D_NONORMALS
        if(model->numface && model->face && neednorm) {
            /* if they are missing, calculate triangle normals into a temporary buffer */
            norm = (m3dv_t*)M3D_MALLOC(model->numface * sizeof(m3dv_t));
            if(!norm) goto memerr;
            for(i = 0, n = model->numvertex; i < model->numface; i++)
                if(model->face[i].normal[0] == M3D_UNDEF) {
                    v0 = &model->vertex[model->face[i].vertex[0]];
                    v1 = &model->vertex[model->face[i].vertex[1]];
                    v2 = &model->vertex[model->face[i].vertex[2]];
                    va.x = v1->x - v0->x; va.y = v1->y - v0->y; va.z = v1->z - v0->z;
                    vb.x = v2->x - v0->x; vb.y = v2->y - v0->y; vb.z = v2->z - v0->z;
                    v0 = &norm[i];
                    v0->x = (va.y * vb.z) - (va.z * vb.y);
                    v0->y = (va.z * vb.x) - (va.x * vb.z);
                    v0->z = (va.x * vb.y) - (va.y * vb.x);
                    w = _m3d_rsq((v0->x * v0->x) + (v0->y * v0->y) + (v0->z * v0->z));
                    v0->x *= w; v0->y *= w; v0->z *= w;
                    model->face[i].normal[0] = model->face[i].vertex[0] + n;
                    model->face[i].normal[1] = model->face[i].vertex[1] + n;
                    model->face[i].normal[2] = model->face[i].vertex[2] + n;
                }
            /* this is the fast way, we don't care if a normal is repeated in model->vertex */
            M3D_LOG("Generating normals");
            model->flags |= M3D_FLG_GENNORM;
            model->numvertex <<= 1;
            model->vertex = (m3dv_t*)M3D_REALLOC(model->vertex, model->numvertex * sizeof(m3dv_t));
            if(!model->vertex) goto memerr;
            memset(&model->vertex[n], 0, n * sizeof(m3dv_t));
            for(i = 0; i < model->numface; i++)
                for(j = 0; j < 3; j++) {
                    v0 = &model->vertex[model->face[i].vertex[j] + n];
                    v0->x += norm[i].x;
                    v0->y += norm[i].y;
                    v0->z += norm[i].z;
                }
            /* for each vertex, take the average of the temporary normals and use that */
            for(i = 0, v0 = &model->vertex[n]; i < n; i++, v0++) {
                w = _m3d_rsq((v0->x * v0->x) + (v0->y * v0->y) + (v0->z * v0->z));
                v0->x *= w; v0->y *= w; v0->z *= w;
                v0->skinid = M3D_UNDEF;
            }
            M3D_FREE(norm);
        }
#endif
        if(model->numbone && model->bone && model->numskin && model->skin && model->numvertex && model->vertex) {
#ifndef M3D_NOWEIGHTS
            M3D_LOG("Generating weight cross-reference");
            for(i = 0; i < model->numvertex; i++) {
                if(model->vertex[i].skinid < model->numskin) {
                    sk = &model->skin[model->vertex[i].skinid];
                    w = (M3D_FLOAT)0.0;
                    for(j = 0; j < M3D_NUMBONE && sk->boneid[j] != M3D_UNDEF && sk->weight[j] > (M3D_FLOAT)0.0; j++)
                        w += sk->weight[j];
                    for(j = 0; j < M3D_NUMBONE && sk->boneid[j] != M3D_UNDEF && sk->weight[j] > (M3D_FLOAT)0.0; j++) {
                        sk->weight[j] /= w;
                        b = &model->bone[sk->boneid[j]];
                        k = b->numweight++;
                        b->weight = (m3dw_t*)M3D_REALLOC(b->weight, b->numweight * sizeof(m3da_t));
                        if(!b->weight) goto memerr;
                        b->weight[k].vertexid = i;
                        b->weight[k].weight = sk->weight[j];
                    }
                }
            }
#endif
#ifndef M3D_NOANIMATION
            M3D_LOG("Calculating bone transformation matrices");
            for(i = 0; i < model->numbone; i++) {
                b = &model->bone[i];
                if(model->bone[i].parent == M3D_UNDEF) {
                    _m3d_mat((M3D_FLOAT*)&b->mat4, &model->vertex[b->pos], &model->vertex[b->ori]);
                } else {
                    _m3d_mat((M3D_FLOAT*)&r, &model->vertex[b->pos], &model->vertex[b->ori]);
                    _m3d_mul((M3D_FLOAT*)&b->mat4, (M3D_FLOAT*)&model->bone[b->parent].mat4, (M3D_FLOAT*)&r);
                }
            }
            for(i = 0; i < model->numbone; i++)
                _m3d_inv((M3D_FLOAT*)&model->bone[i].mat4);
#endif
        }
#ifdef M3D_PROFILING
        gettimeofday(&tv0, NULL);
        tvd.tv_sec = tv0.tv_sec - tv1.tv_sec;
        tvd.tv_usec = tv0.tv_usec - tv1.tv_usec;
        if(tvd.tv_usec < 0) { tvd.tv_sec--; tvd.tv_usec += 1000000L; }
        printf("  Post-process    %ld.%06ld sec\n", tvd.tv_sec, tvd.tv_usec);
#endif
    }
    return model;
}

/**
 * Calculates skeletons for animation frames, returns a working copy (should be freed after use)
 */
m3dtr_t *m3d_frame(m3d_t *model, M3D_INDEX actionid, M3D_INDEX frameid, m3dtr_t *skeleton)
{
    unsigned int i;
    M3D_INDEX s = frameid;
    m3dfr_t *fr;

    if(!model || !model->numbone || !model->bone || (actionid != M3D_UNDEF && (!model->action ||
        actionid >= model->numaction || frameid >= model->action[actionid].numframe))) {
            model->errcode = M3D_ERR_UNKFRAME;
            return skeleton;
    }
    model->errcode = M3D_SUCCESS;
    if(!skeleton) {
        skeleton = (m3dtr_t*)M3D_MALLOC(model->numbone * sizeof(m3dtr_t));
        if(!skeleton) {
            model->errcode = M3D_ERR_ALLOC;
            return NULL;
        }
        goto gen;
    }
    if(actionid == M3D_UNDEF || !frameid) {
gen:    s = 0;
        for(i = 0; i < model->numbone; i++) {
            skeleton[i].boneid = i;
            skeleton[i].pos = model->bone[i].pos;
            skeleton[i].ori = model->bone[i].ori;
        }
    }
    if(actionid < model->numaction && (frameid || !model->action[actionid].frame[0].msec)) {
        for(; s <= frameid; s++) {
            fr = &model->action[actionid].frame[s];
            for(i = 0; i < fr->numtransform; i++) {
                skeleton[fr->transform[i].boneid].pos = fr->transform[i].pos;
                skeleton[fr->transform[i].boneid].ori = fr->transform[i].ori;
            }
        }
    }
    return skeleton;
}

#ifndef M3D_NOANIMATION
/**
 * Returns interpolated animation-pose, a working copy (should be freed after use)
 */
m3db_t *m3d_pose(m3d_t *model, M3D_INDEX actionid, uint32_t msec)
{
    unsigned int i, j, l;
    M3D_FLOAT r[16], t, c, d, s;
    m3db_t *ret;
    m3dv_t *v, *p, *f;
    m3dtr_t *tmp;
    m3dfr_t *fr;

    if(!model || !model->numbone || !model->bone) {
        model->errcode = M3D_ERR_UNKFRAME;
        return NULL;
    }
    ret = (m3db_t*)M3D_MALLOC(model->numbone * sizeof(m3db_t));
    if(!ret) {
        model->errcode = M3D_ERR_ALLOC;
        return NULL;
    }
    memcpy(ret, model->bone, model->numbone * sizeof(m3db_t));
    for(i = 0; i < model->numbone; i++)
        _m3d_inv((M3D_FLOAT*)&ret[i].mat4);
    if(!model->action || actionid >= model->numaction) {
        model->errcode = M3D_ERR_UNKFRAME;
        return ret;
    }
    msec %= model->action[actionid].durationmsec;
    model->errcode = M3D_SUCCESS;
    fr = &model->action[actionid].frame[0];
    for(j = l = 0; j < model->action[actionid].numframe && model->action[actionid].frame[j].msec <= msec; j++) {
        fr = &model->action[actionid].frame[j];
        l = fr->msec;
        for(i = 0; i < fr->numtransform; i++) {
            ret[fr->transform[i].boneid].pos = fr->transform[i].pos;
            ret[fr->transform[i].boneid].ori = fr->transform[i].ori;
        }
    }
    if(l != msec) {
        model->vertex = (m3dv_t*)M3D_REALLOC(model->vertex, (model->numvertex + 2 * model->numbone) * sizeof(m3dv_t));
        if(!model->vertex) {
            free(ret);
            model->errcode = M3D_ERR_ALLOC;
            return NULL;
        }
        tmp = (m3dtr_t*)M3D_MALLOC(model->numbone * sizeof(m3dtr_t));
        if(tmp) {
            for(i = 0; i < model->numbone; i++) {
                tmp[i].pos = ret[i].pos;
                tmp[i].ori = ret[i].ori;
            }
            fr = &model->action[actionid].frame[j % model->action[actionid].numframe];
            t = l >= fr->msec ? (M3D_FLOAT)1.0 : (M3D_FLOAT)(msec - l) / (M3D_FLOAT)(fr->msec - l);
            for(i = 0; i < fr->numtransform; i++) {
                tmp[fr->transform[i].boneid].pos = fr->transform[i].pos;
                tmp[fr->transform[i].boneid].ori = fr->transform[i].ori;
            }
            for(i = 0, j = model->numvertex; i < model->numbone; i++) {
                /* interpolation of position */
                if(ret[i].pos != tmp[i].pos) {
                    p = &model->vertex[ret[i].pos];
                    f = &model->vertex[tmp[i].pos];
                    v = &model->vertex[j];
                    v->x = p->x + t * (f->x - p->x);
                    v->y = p->y + t * (f->y - p->y);
                    v->z = p->z + t * (f->z - p->z);
                    ret[i].pos = j++;
                }
                /* interpolation of orientation */
                if(ret[i].ori != tmp[i].ori) {
                    p = &model->vertex[ret[i].ori];
                    f = &model->vertex[tmp[i].ori];
                    v = &model->vertex[j];
                    d = p->w * f->w + p->x * f->x + p->y * f->y + p->z * f->z;
                    if(d < 0) { d = -d; s = (M3D_FLOAT)-1.0; } else s = (M3D_FLOAT)1.0;
#if 0
                    /* don't use SLERP, requires two more variables, libm linkage and it is slow (but nice) */
                    a = (M3D_FLOAT)1.0 - t; b = t;
                    if(d < (M3D_FLOAT)0.999999) { c = acosf(d); b = 1 / sinf(c); a = sinf(a * c) * b; b *= sinf(t * c) * s; }
                    v->x = p->x * a + f->x * b;
                    v->y = p->y * a + f->y * b;
                    v->z = p->z * a + f->z * b;
                    v->w = p->w * a + f->w * b;
#else
                    /* approximated NLERP, original approximation by Arseny Kapoulkine, heavily optimized by me */
                    c = t - (M3D_FLOAT)0.5; t += t * c * (t - (M3D_FLOAT)1.0) * (((M3D_FLOAT)1.0904 + d * ((M3D_FLOAT)-3.2452 +
                        d * ((M3D_FLOAT)3.55645 - d * (M3D_FLOAT)1.43519))) * c * c + ((M3D_FLOAT)0.848013 + d *
                        ((M3D_FLOAT)-1.06021 + d * (M3D_FLOAT)0.215638)));
                    v->x = p->x + t * (s * f->x - p->x);
                    v->y = p->y + t * (s * f->y - p->y);
                    v->z = p->z + t * (s * f->z - p->z);
                    v->w = p->w + t * (s * f->w - p->w);
                    d = _m3d_rsq(v->w * v->w + v->x * v->x + v->y * v->y + v->z * v->z);
                    v->x *= d; v->y *= d; v->z *= d; v->w *= d;
#endif
                    ret[i].ori = j++;
                }
            }
            M3D_FREE(tmp);
        }
    }
    for(i = 0; i < model->numbone; i++) {
        if(ret[i].parent == M3D_UNDEF) {
            _m3d_mat((M3D_FLOAT*)&ret[i].mat4, &model->vertex[ret[i].pos], &model->vertex[ret[i].ori]);
        } else {
            _m3d_mat((M3D_FLOAT*)&r, &model->vertex[ret[i].pos], &model->vertex[ret[i].ori]);
            _m3d_mul((M3D_FLOAT*)&ret[i].mat4, (M3D_FLOAT*)&ret[ret[i].parent].mat4, (M3D_FLOAT*)&r);
        }
    }
    return ret;
}

#endif /* M3D_NOANIMATION */

#endif /* M3D_IMPLEMENTATION */

#if !defined(M3D_NODUP) && (!defined(M3D_NOIMPORTER) || defined(M3D_EXPORTER))
/**
 * Free the in-memory model
 */
void m3d_free(m3d_t *model)
{
    unsigned int i, j;

    if(!model) return;
#ifdef M3D_ASCII
    /* if model imported from ASCII, we have to free all strings as well */
    if(model->flags & M3D_FLG_FREESTR) {
        if(model->name) M3D_FREE(model->name);
        if(model->license) M3D_FREE(model->license);
        if(model->author) M3D_FREE(model->author);
        if(model->desc) M3D_FREE(model->desc);
        if(model->bone)
            for(i = 0; i < model->numbone; i++)
                if(model->bone[i].name)
                    M3D_FREE(model->bone[i].name);
        if(model->shape)
            for(i = 0; i < model->numshape; i++)
                if(model->shape[i].name)
                    M3D_FREE(model->shape[i].name);
        if(model->numvoxtype)
            for(i = 0; i < model->numvoxtype; i++) {
                if(model->voxtype[i].name)
                    M3D_FREE(model->voxtype[i].name);
                for(j = 0; j < model->voxtype[i].numitem; j++)
                    if(model->voxtype[i].item[j].name)
                        M3D_FREE(model->voxtype[i].item[j].name);
            }
        if(model->numvoxel)
            for(i = 0; i < model->numvoxel; i++)
                if(model->voxel[i].name)
                    M3D_FREE(model->voxel[i].name);
        if(model->material)
            for(i = 0; i < model->nummaterial; i++)
                if(model->material[i].name)
                    M3D_FREE(model->material[i].name);
        if(model->action)
            for(i = 0; i < model->numaction; i++)
                if(model->action[i].name)
                    M3D_FREE(model->action[i].name);
        if(model->texture)
            for(i = 0; i < model->numtexture; i++)
                if(model->texture[i].name)
                    M3D_FREE(model->texture[i].name);
        if(model->inlined)
            for(i = 0; i < model->numinlined; i++) {
                if(model->inlined[i].name)
                    M3D_FREE(model->inlined[i].name);
                if(model->inlined[i].data)
                    M3D_FREE(model->inlined[i].data);
            }
        if(model->extra)
            for(i = 0; i < model->numextra; i++)
                if(model->extra[i])
                    M3D_FREE(model->extra[i]);
        if(model->label)
            for(i = 0; i < model->numlabel; i++) {
                if(model->label[i].name) {
                    for(j = i + 1; j < model->numlabel; j++)
                        if(model->label[j].name == model->label[i].name)
                            model->label[j].name = NULL;
                    M3D_FREE(model->label[i].name);
                }
                if(model->label[i].lang) {
                    for(j = i + 1; j < model->numlabel; j++)
                        if(model->label[j].lang == model->label[i].lang)
                            model->label[j].lang = NULL;
                    M3D_FREE(model->label[i].lang);
                }
                if(model->label[i].text)
                    M3D_FREE(model->label[i].text);
            }
        if(model->preview.data)
            M3D_FREE(model->preview.data);
    }
#endif
    if(model->flags & M3D_FLG_FREERAW) M3D_FREE(model->raw);

    if(model->tmap) M3D_FREE(model->tmap);
    if(model->bone) {
        for(i = 0; i < model->numbone; i++)
            if(model->bone[i].weight)
                M3D_FREE(model->bone[i].weight);
        M3D_FREE(model->bone);
    }
    if(model->skin) M3D_FREE(model->skin);
    if(model->vertex) M3D_FREE(model->vertex);
    if(model->face) M3D_FREE(model->face);
    if(model->voxtype) {
        for(i = 0; i < model->numvoxtype; i++)
            if(model->voxtype[i].item)
                M3D_FREE(model->voxtype[i].item);
        M3D_FREE(model->voxtype);
    }
    if(model->voxel) {
        for(i = 0; i < model->numvoxel; i++)
            if(model->voxel[i].data)
                M3D_FREE(model->voxel[i].data);
        M3D_FREE(model->voxel);
    }
    if(model->shape) {
        for(i = 0; i < model->numshape; i++) {
            if(model->shape[i].cmd) {
                for(j = 0; j < model->shape[i].numcmd; j++)
                    if(model->shape[i].cmd[j].arg) M3D_FREE(model->shape[i].cmd[j].arg);
                M3D_FREE(model->shape[i].cmd);
            }
        }
        M3D_FREE(model->shape);
    }
    if(model->material && !(model->flags & M3D_FLG_MTLLIB)) {
        for(i = 0; i < model->nummaterial; i++)
            if(model->material[i].prop) M3D_FREE(model->material[i].prop);
        M3D_FREE(model->material);
    }
    if(model->texture) {
        for(i = 0; i < model->numtexture; i++)
            if(model->texture[i].d) M3D_FREE(model->texture[i].d);
        M3D_FREE(model->texture);
    }
    if(model->action) {
        for(i = 0; i < model->numaction; i++) {
            if(model->action[i].frame) {
                for(j = 0; j < model->action[i].numframe; j++)
                    if(model->action[i].frame[j].transform) M3D_FREE(model->action[i].frame[j].transform);
                M3D_FREE(model->action[i].frame);
            }
        }
        M3D_FREE(model->action);
    }
    if(model->label) M3D_FREE(model->label);
    if(model->inlined) M3D_FREE(model->inlined);
    if(model->extra) M3D_FREE(model->extra);
    free(model);
}
#endif

#ifdef M3D_EXPORTER
typedef struct {
    char *str;
    uint32_t offs;
} m3dstr_t;

typedef struct {
    m3dti_t data;
    M3D_INDEX oldidx;
    M3D_INDEX newidx;
} m3dtisave_t;

typedef struct {
    m3dv_t data;
    M3D_INDEX oldidx;
    M3D_INDEX newidx;
    unsigned char norm;
} m3dvsave_t;

typedef struct {
    m3ds_t data;
    M3D_INDEX oldidx;
    M3D_INDEX newidx;
} m3dssave_t;

typedef struct {
    m3df_t data;
    int group;
    uint8_t opacity;
} m3dfsave_t;

/* create unique list of strings */
static m3dstr_t *_m3d_addstr(m3dstr_t *str, uint32_t *numstr, char *s)
{
    uint32_t i;
    if(!s || !*s) return str;
    if(str) {
        for(i = 0; i < *numstr; i++)
            if(str[i].str == s || !strcmp(str[i].str, s)) return str;
    }
    str = (m3dstr_t*)M3D_REALLOC(str, ((*numstr) + 1) * sizeof(m3dstr_t));
    str[*numstr].str = s;
    str[*numstr].offs = 0;
    (*numstr)++;
    return str;
}

/* add strings to header */
m3dhdr_t *_m3d_addhdr(m3dhdr_t *h, m3dstr_t *s)
{
    int i;
    char *safe = _m3d_safestr(s->str, 0);
    i = (int)strlen(safe);
    h = (m3dhdr_t*)M3D_REALLOC(h, h->length + i+1);
    if(!h) { M3D_FREE(safe); return NULL; }
    memcpy((uint8_t*)h + h->length, safe, i+1);
    s->offs = h->length - 16;
    h->length += i+1;
    M3D_FREE(safe);
    return h;
}

/* return offset of string */
static uint32_t _m3d_stridx(m3dstr_t *str, uint32_t numstr, char *s)
{
    uint32_t i;
    char *safe;
    if(!s || !*s) return 0;
    if(str) {
        safe = _m3d_safestr(s, 0);
        if(!safe) return 0;
        if(!*safe) {
            free(safe);
            return 0;
        }
        for(i = 0; i < numstr; i++)
            if(!strcmp(str[i].str, s)) {
                free(safe);
                return str[i].offs;
            }
        free(safe);
    }
    return 0;
}

/* compare to faces by their material */
static int _m3d_facecmp(const void *a, const void *b) {
    const m3dfsave_t *A = (const m3dfsave_t*)a, *B = (const m3dfsave_t*)b;
    return A->group != B->group ? A->group - B->group : (A->opacity != B->opacity ? (int)B->opacity - (int)A->opacity :
        (int)A->data.materialid - (int)B->data.materialid);
}
/* compare face groups */
static int _m3d_grpcmp(const void *a, const void *b) { return *((uint32_t*)a) - *((uint32_t*)b); }
/* compare UVs */
static int _m3d_ticmp(const void *a, const void *b) { return memcmp(a, b, sizeof(m3dti_t)); }
/* compare skin groups */
static int _m3d_skincmp(const void *a, const void *b) { return memcmp(a, b, sizeof(m3ds_t)); }
/* compare vertices */
static int _m3d_vrtxcmp(const void *a, const void *b) {
    int c = memcmp(a, b, 3 * sizeof(M3D_FLOAT));
    if(c) return c;
    c = ((m3dvsave_t*)a)->norm - ((m3dvsave_t*)b)->norm;
    if(c) return c;
    return memcmp(a, b, sizeof(m3dv_t));
}
/* compare labels */
static _inline int _m3d_strcmp(char *a, char *b)
{
    if(a == NULL && b != NULL) return -1;
    if(a != NULL && b == NULL) return 1;
    if(a == NULL && b == NULL) return 0;
    return strcmp(a, b);
}
static int _m3d_lblcmp(const void *a, const void *b) {
    const m3dl_t *A = (const m3dl_t*)a, *B = (const m3dl_t*)b;
    int c = _m3d_strcmp(A->lang, B->lang);
    if(!c) c = _m3d_strcmp(A->name, B->name);
    if(!c) c = _m3d_strcmp(A->text, B->text);
    return c;
}
/* compare two colors by HSV value */
_inline static int _m3d_cmapcmp(const void *a, const void *b)
{
    uint8_t *A = (uint8_t*)a,  *B = (uint8_t*)b;
    _register int m, vA, vB;
    /* get HSV value for A */
    m = A[2] < A[1]? A[2] : A[1]; if(A[0] < m) m = A[0];
    vA = A[2] > A[1]? A[2] : A[1]; if(A[0] > vA) vA = A[0];
    /* get HSV value for B */
    m = B[2] < B[1]? B[2] : B[1]; if(B[0] < m) m = B[0];
    vB = B[2] > B[1]? B[2] : B[1]; if(B[0] > vB) vB = B[0];
    return vA - vB;
}

/* create sorted list of colors */
static uint32_t *_m3d_addcmap(uint32_t *cmap, uint32_t *numcmap, uint32_t color)
{
    uint32_t i;
    if(cmap) {
        for(i = 0; i < *numcmap; i++)
            if(cmap[i] == color) return cmap;
    }
    cmap = (uint32_t*)M3D_REALLOC(cmap, ((*numcmap) + 1) * sizeof(uint32_t));
    for(i = 0; i < *numcmap && _m3d_cmapcmp(&color, &cmap[i]) > 0; i++);
    if(i < *numcmap) memmove(&cmap[i+1], &cmap[i], ((*numcmap) - i)*sizeof(uint32_t));
    cmap[i] = color;
    (*numcmap)++;
    return cmap;
}

/* look up a color and return its index */
static uint32_t _m3d_cmapidx(uint32_t *cmap, uint32_t numcmap, uint32_t color)
{
    uint32_t i;
    if(numcmap >= 65536)
        return color;
    for(i = 0; i < numcmap; i++)
        if(cmap[i] == color) return i;
    return 0;
}

/* add index to output */
static unsigned char *_m3d_addidx(unsigned char *out, char type, uint32_t idx) {
    switch(type) {
        case 1: *out++ = (uint8_t)(idx); break;
        case 2: *((uint16_t*)out) = (uint16_t)(idx); out += 2; break;
        case 4: *((uint32_t*)out) = (uint32_t)(idx); out += 4; break;
        /* case 0: case 8: break; */
    }
    return out;
}

/* round a vertex position */
static void _m3d_round(int quality, m3dv_t *src, m3dv_t *dst)
{
    _register int t;
    /* copy additional attributes */
    if(src != dst) memcpy(dst, src, sizeof(m3dv_t));
    /* round according to quality */
    switch(quality) {
        case M3D_EXP_INT8:
            t = (int)(src->x * 127 + (src->x >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->x = (M3D_FLOAT)t / (M3D_FLOAT)127.0;
            t = (int)(src->y * 127 + (src->y >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->y = (M3D_FLOAT)t / (M3D_FLOAT)127.0;
            t = (int)(src->z * 127 + (src->z >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->z = (M3D_FLOAT)t / (M3D_FLOAT)127.0;
            t = (int)(src->w * 127 + (src->w >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->w = (M3D_FLOAT)t / (M3D_FLOAT)127.0;
        break;
        case M3D_EXP_INT16:
            t = (int)(src->x * 32767 + (src->x >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->x = (M3D_FLOAT)t / (M3D_FLOAT)32767.0;
            t = (int)(src->y * 32767 + (src->y >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->y = (M3D_FLOAT)t / (M3D_FLOAT)32767.0;
            t = (int)(src->z * 32767 + (src->z >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->z = (M3D_FLOAT)t / (M3D_FLOAT)32767.0;
            t = (int)(src->w * 32767 + (src->w >= 0 ? (M3D_FLOAT)0.5 : (M3D_FLOAT)-0.5)); dst->w = (M3D_FLOAT)t / (M3D_FLOAT)32767.0;
        break;
    }
    if(dst->x == (M3D_FLOAT)-0.0) dst->x = (M3D_FLOAT)0.0;
    if(dst->y == (M3D_FLOAT)-0.0) dst->y = (M3D_FLOAT)0.0;
    if(dst->z == (M3D_FLOAT)-0.0) dst->z = (M3D_FLOAT)0.0;
    if(dst->w == (M3D_FLOAT)-0.0) dst->w = (M3D_FLOAT)0.0;
}

#ifdef M3D_ASCII
/* add a bone to ascii output */
static char *_m3d_prtbone(char *ptr, m3db_t *bone, M3D_INDEX numbone, M3D_INDEX parent, uint32_t level, M3D_INDEX *vrtxidx)
{
    uint32_t i, j;
    char *sn;

    if(level > M3D_BONEMAXLEVEL || !bone) return ptr;
    for(i = 0; i < numbone; i++) {
        if(bone[i].parent == parent) {
            for(j = 0; j < level; j++) *ptr++ = '/';
            sn = _m3d_safestr(bone[i].name, 0);
            ptr += sprintf(ptr, "%d %d %s\r\n", vrtxidx[bone[i].pos], vrtxidx[bone[i].ori], sn);
            M3D_FREE(sn);
            ptr = _m3d_prtbone(ptr, bone, numbone, i, level + 1, vrtxidx);
        }
    }
    return ptr;
}
#endif

/**
 * Function to encode an in-memory model into on storage Model 3D format
 */
unsigned char *m3d_save(m3d_t *model, int quality, int flags, unsigned int *size)
{
#ifdef M3D_ASCII
    const char *ol;
    char *ptr;
#endif
    char vc_s, vi_s, si_s, ci_s, ti_s, bi_s, nb_s, sk_s, fc_s, hi_s, fi_s, vd_s, vp_s;
    char *sn = NULL, *sl = NULL, *sa = NULL, *sd = NULL;
    unsigned char *out = NULL, *z = NULL, weights[M3D_NUMBONE < 8 ? 8 : M3D_NUMBONE], *norm = NULL;
    unsigned int i, j, k, l, n, o, len, chunklen, *length;
    int maxvox = 0, minvox = 0;
    M3D_FLOAT scale = (M3D_FLOAT)0.0, min_x, max_x, min_y, max_y, min_z, max_z;
    M3D_INDEX last, *vrtxidx = NULL, *mtrlidx = NULL, *tmapidx = NULL, *skinidx = NULL;
#ifdef M3D_VERTEXMAX
    M3D_INDEX lastp;
#endif
    uint32_t idx, numcmap = 0, *cmap = NULL, numvrtx = 0, maxvrtx = 0, numtmap = 0, maxtmap = 0, numproc = 0;
    uint32_t numskin = 0, maxskin = 0, numstr = 0, maxt = 0, maxbone = 0, numgrp = 0, maxgrp = 0, *grpidx = NULL;
    uint8_t *opa;
    m3dcd_t *cd;
    m3dc_t *cmd;
    m3dstr_t *str = NULL;
    m3dvsave_t *vrtx = NULL, vertex;
    m3dtisave_t *tmap = NULL, tcoord;
    m3dssave_t *skin = NULL, sk;
    m3dfsave_t *face = NULL;
    m3dhdr_t *h = NULL;
    m3dm_t *m;
    m3da_t *a;

    if(!model) {
        if(size) *size = 0;
        return NULL;
    }
    model->errcode = M3D_SUCCESS;
#ifdef M3D_ASCII
    if(flags & M3D_EXP_ASCII) quality = M3D_EXP_DOUBLE;
#endif
    vrtxidx = (M3D_INDEX*)M3D_MALLOC(model->numvertex * sizeof(M3D_INDEX));
    if(!vrtxidx) goto memerr;
    memset(vrtxidx, 255, model->numvertex * sizeof(M3D_INDEX));
    if(model->numvertex && !(flags & M3D_EXP_NONORMAL)){
        norm = (unsigned char*)M3D_MALLOC(model->numvertex * sizeof(unsigned char));
        if(!norm) goto memerr;
        memset(norm, 0, model->numvertex * sizeof(unsigned char));
    }
    if(model->nummaterial && !(flags & M3D_EXP_NOMATERIAL)) {
        mtrlidx = (M3D_INDEX*)M3D_MALLOC(model->nummaterial * sizeof(M3D_INDEX));
        if(!mtrlidx) goto memerr;
        memset(mtrlidx, 255, model->nummaterial * sizeof(M3D_INDEX));
        opa = (uint8_t*)M3D_MALLOC(model->nummaterial * 2 * sizeof(M3D_INDEX));
        if(!opa) goto memerr;
        memset(opa, 255, model->nummaterial * 2 * sizeof(M3D_INDEX));
    }
    if(model->numtmap && !(flags & M3D_EXP_NOTXTCRD)) {
        tmapidx = (M3D_INDEX*)M3D_MALLOC(model->numtmap * sizeof(M3D_INDEX));
        if(!tmapidx) goto memerr;
        memset(tmapidx, 255, model->numtmap * sizeof(M3D_INDEX));
    }
    /** collect array elements that are actually referenced **/
    if(!(flags & M3D_EXP_NOFACE)) {
        /* face */
        if(model->numface && model->face) {
            M3D_LOG("Processing mesh face");
            face = (m3dfsave_t*)M3D_MALLOC(model->numface * sizeof(m3dfsave_t));
            if(!face) goto memerr;
            for(i = 0; i < model->numface; i++) {
                memcpy(&face[i].data, &model->face[i], sizeof(m3df_t));
                face[i].group = 0;
                face[i].opacity = 255;
                if(!(flags & M3D_EXP_NOMATERIAL) && model->face[i].materialid < model->nummaterial) {
                    if(model->material[model->face[i].materialid].numprop) {
                        mtrlidx[model->face[i].materialid] = 0;
                        if(opa[model->face[i].materialid * 2]) {
                            m = &model->material[model->face[i].materialid];
                            for(j = 0; j < m->numprop; j++)
                                if(m->prop[j].type == m3dp_Kd) {
                                    opa[model->face[i].materialid * 2 + 1] = ((uint8_t*)&m->prop[j].value.color)[3];
                                    break;
                                }
                            for(j = 0; j < m->numprop; j++)
                                if(m->prop[j].type == m3dp_d) {
                                    opa[model->face[i].materialid * 2 + 1] = (uint8_t)(m->prop[j].value.fnum * 255);
                                    break;
                                }
                            opa[model->face[i].materialid * 2] = 0;
                        }
                        face[i].opacity = opa[model->face[i].materialid * 2 + 1];
                    } else
                        face[i].data.materialid = M3D_UNDEF;
                }
                for(j = 0; j < 3; j++) {
                    k = model->face[i].vertex[j];
                    if(k < model->numvertex)
                        vrtxidx[k] = 0;
                    if(!(flags & M3D_EXP_NOCMAP)) {
                        cmap = _m3d_addcmap(cmap, &numcmap, model->vertex[k].color);
                        if(!cmap) goto memerr;
                    }
                    k = model->face[i].normal[j];
                    if(k < model->numvertex && !(flags & M3D_EXP_NONORMAL)) {
                        vrtxidx[k] = 0;
                        norm[k] = 1;
                    }
                    k = model->face[i].texcoord[j];
                    if(k < model->numtmap && !(flags & M3D_EXP_NOTXTCRD))
                        tmapidx[k] = 0;
#ifdef M3D_VERTEXMAX
                    k = model->face[i].vertmax[j];
                    if(k < model->numvertex && !(flags & M3D_EXP_NOVRTMAX))
                        vrtxidx[k] = 0;
#endif
                }
                /* convert from CW to CCW */
                if(flags & M3D_EXP_IDOSUCK) {
                    j = face[i].data.vertex[1];
                    face[i].data.vertex[1] = face[i].data.vertex[2];
                    face[i].data.vertex[2] = j;
                    j = face[i].data.normal[1];
                    face[i].data.normal[1] = face[i].data.normal[2];
                    face[i].data.normal[2] = j;
                    j = face[i].data.texcoord[1];
                    face[i].data.texcoord[1] = face[i].data.texcoord[2];
                    face[i].data.texcoord[2] = j;
#ifdef M3D_VERTEXMAX
                    j = face[i].data.vertmax[1];
                    face[i].data.vertmax[1] = face[i].data.vertmax[2];
                    face[i].data.vertmax[2] = j;
#endif
                }
            }
        }
        if((model->numvoxtype && model->voxtype) || (model->numvoxel && model->voxel)) {
            M3D_LOG("Processing voxel face");
            for(i = 0; i < model->numvoxtype; i++) {
                str = _m3d_addstr(str, &numstr, model->voxtype[i].name);
                if(model->voxtype[i].name && !str) goto memerr;
                if(!(flags & M3D_EXP_NOCMAP)) {
                    cmap = _m3d_addcmap(cmap, &numcmap, model->voxtype[i].color);
                    if(!cmap) goto memerr;
                }
                for(j = 0; j < model->voxtype[i].numitem; j++) {
                    str = _m3d_addstr(str, &numstr, model->voxtype[i].item[j].name);
                    if(model->voxtype[i].item[j].name && !str) goto memerr;
                }
            }
            for(i = 0; i < model->numvoxel; i++) {
                str = _m3d_addstr(str, &numstr, model->voxel[i].name);
                if(model->voxel[i].name && !str) goto memerr;
                if(model->voxel[i].x < minvox) minvox = model->voxel[i].x;
                if(model->voxel[i].x + (int)model->voxel[i].w > maxvox) maxvox = model->voxel[i].x + model->voxel[i].w;
                if(model->voxel[i].y < minvox) minvox = model->voxel[i].y;
                if(model->voxel[i].y + (int)model->voxel[i].h > maxvox) maxvox = model->voxel[i].y + model->voxel[i].h;
                if(model->voxel[i].z < minvox) minvox = model->voxel[i].z;
                if(model->voxel[i].z + (int)model->voxel[i].d > maxvox) maxvox = model->voxel[i].z + model->voxel[i].d;
            }
        }
        if(model->numshape && model->shape) {
            M3D_LOG("Processing shape face");
            for(i = 0; i < model->numshape; i++) {
                if(!model->shape[i].numcmd) continue;
                str = _m3d_addstr(str, &numstr, model->shape[i].name);
                if(model->shape[i].name && !str) goto memerr;
                for(j = 0; j < model->shape[i].numcmd; j++) {
                    cmd = &model->shape[i].cmd[j];
                    if(cmd->type >= (unsigned int)(sizeof(m3d_commandtypes)/sizeof(m3d_commandtypes[0])) || !cmd->arg)
                        continue;
                    if(cmd->type == m3dc_mesh) {
                        if(numgrp + 2 < maxgrp) {
                            maxgrp += 1024;
                            grpidx = (uint32_t*)realloc(grpidx, maxgrp * sizeof(uint32_t));
                            if(!grpidx) goto memerr;
                            if(!numgrp) {
                                grpidx[0] = 0;
                                grpidx[1] = model->numface;
                                numgrp += 2;
                            }
                        }
                        grpidx[numgrp + 0] = cmd->arg[0];
                        grpidx[numgrp + 1] = cmd->arg[0] + cmd->arg[1];
                        numgrp += 2;
                    }
                    cd = &m3d_commandtypes[cmd->type];
                    for(k = n = 0, l = cd->p; k < l; k++)
                        switch(cd->a[((k - n) % (cd->p - n)) + n]) {
                            case m3dcp_mi_t:
                                if(!(flags & M3D_EXP_NOMATERIAL) && cmd->arg[k] < model->nummaterial)
                                    mtrlidx[cmd->arg[k]] = 0;
                            break;
                            case m3dcp_ti_t:
                                if(!(flags & M3D_EXP_NOTXTCRD) && cmd->arg[k] < model->numtmap)
                                    tmapidx[cmd->arg[k]] = 0;
                            break;
                            case m3dcp_qi_t:
                            case m3dcp_vi_t:
                                if(cmd->arg[k] < model->numvertex)
                                    vrtxidx[cmd->arg[k]] = 0;
                            break;
                            case m3dcp_va_t:
                                n = k + 1; l += (cmd->arg[k] - 1) * (cd->p - k - 1);
                            break;
                        }
                }
            }
        }
        if(model->numface && face) {
            if(numgrp && grpidx) {
                qsort(grpidx, numgrp, sizeof(uint32_t), _m3d_grpcmp);
                for(i = j = 0; i < model->numface && j < numgrp; i++) {
                    while(j < numgrp && grpidx[j] < i) j++;
                    face[i].group = j;
                }
            }
            qsort(face, model->numface, sizeof(m3dfsave_t), _m3d_facecmp);
        }
        if(grpidx) { M3D_FREE(grpidx); grpidx = NULL; }
        if(model->numlabel && model->label) {
            M3D_LOG("Processing annotation labels");
            for(i = 0; i < model->numlabel; i++) {
                str = _m3d_addstr(str, &numstr, model->label[i].name);
                str = _m3d_addstr(str, &numstr, model->label[i].lang);
                str = _m3d_addstr(str, &numstr, model->label[i].text);
                if(!(flags & M3D_EXP_NOCMAP)) {
                    cmap = _m3d_addcmap(cmap, &numcmap, model->label[i].color);
                    if(!cmap) goto memerr;
                }
                if(model->label[i].vertexid < model->numvertex)
                    vrtxidx[model->label[i].vertexid] = 0;
            }
            qsort(model->label, model->numlabel, sizeof(m3dl_t), _m3d_lblcmp);
        }
    } else if(!(flags & M3D_EXP_NOMATERIAL)) {
        /* without a face, simply add all materials, because it can be an mtllib */
        for(i = 0; i < model->nummaterial; i++)
            mtrlidx[i] = i;
    }
    /* bind-pose skeleton */
    if(model->numbone && model->bone && !(flags & M3D_EXP_NOBONE)) {
        M3D_LOG("Processing bones");
        for(i = 0; i < model->numbone; i++) {
            str = _m3d_addstr(str, &numstr, model->bone[i].name);
            if(!str) goto memerr;
            k = model->bone[i].pos;
            if(k < model->numvertex)
                vrtxidx[k] = 0;
            k = model->bone[i].ori;
            if(k < model->numvertex)
                vrtxidx[k] = 0;
        }
    }
    /* actions, animated skeleton poses */
    if(model->numaction && model->action && !(flags & M3D_EXP_NOACTION)) {
        M3D_LOG("Processing action list");
        for(j = 0; j < model->numaction; j++) {
            a = &model->action[j];
            str = _m3d_addstr(str, &numstr, a->name);
            if(!str) goto memerr;
            if(a->numframe > 65535) a->numframe = 65535;
            for(i = 0; i < a->numframe; i++) {
                for(l = 0; l < a->frame[i].numtransform; l++) {
                    k = a->frame[i].transform[l].pos;
                    if(k < model->numvertex)
                        vrtxidx[k] = 0;
                    k = a->frame[i].transform[l].ori;
                    if(k < model->numvertex)
                        vrtxidx[k] = 0;
                }
                if(l > maxt) maxt = l;
            }
        }
    }
    /* add colors to color map and texture names to string table */
    if(!(flags & M3D_EXP_NOMATERIAL)) {
        M3D_LOG("Processing materials");
        for(i = k = 0; i < model->nummaterial; i++) {
            if(mtrlidx[i] == M3D_UNDEF || !model->material[i].numprop) continue;
            mtrlidx[i] = k++;
            m = &model->material[i];
            str = _m3d_addstr(str, &numstr, m->name);
            if(!str) goto memerr;
            if(m->prop)
                for(j = 0; j < m->numprop; j++) {
                    if(!(flags & M3D_EXP_NOCMAP) && m->prop[j].type < 128) {
                        for(l = 0; l < sizeof(m3d_propertytypes)/sizeof(m3d_propertytypes[0]); l++) {
                            if(m->prop[j].type == m3d_propertytypes[l].id && m3d_propertytypes[l].format == m3dpf_color) {
                                ((uint8_t*)&m->prop[j].value.color)[3] = opa[i * 2 + 1];
                                cmap = _m3d_addcmap(cmap, &numcmap, m->prop[j].value.color);
                                if(!cmap) goto memerr;
                                break;
                            }
                        }
                    }
                    if(m->prop[j].type >= 128 && m->prop[j].value.textureid < model->numtexture &&
                        model->texture[m->prop[j].value.textureid].name) {
                        str = _m3d_addstr(str, &numstr, model->texture[m->prop[j].value.textureid].name);
                        if(!str) goto memerr;
                    }
                }
        }
    }
    /* if there's only one black color, don't store it */
    if(numcmap == 1 && cmap && !cmap[0]) numcmap = 0;

    /** compress lists **/
    if(model->numtmap && !(flags & M3D_EXP_NOTXTCRD)) {
        M3D_LOG("Compressing tmap");
        tmap = (m3dtisave_t*)M3D_MALLOC(model->numtmap * sizeof(m3dtisave_t));
        if(!tmap) goto memerr;
        for(i = 0; i < model->numtmap; i++) {
            if(tmapidx[i] == M3D_UNDEF) continue;
            switch(quality) {
                case M3D_EXP_INT8:
                    l = (unsigned int)(model->tmap[i].u * 255); tcoord.data.u = (M3D_FLOAT)l / (M3D_FLOAT)255.0;
                    l = (unsigned int)(model->tmap[i].v * 255); tcoord.data.v = (M3D_FLOAT)l / (M3D_FLOAT)255.0;
                break;
                case M3D_EXP_INT16:
                    l = (unsigned int)(model->tmap[i].u * 65535); tcoord.data.u = (M3D_FLOAT)l / (M3D_FLOAT)65535.0;
                    l = (unsigned int)(model->tmap[i].v * 65535); tcoord.data.v = (M3D_FLOAT)l / (M3D_FLOAT)65535.0;
                break;
                default:
                    tcoord.data.u = model->tmap[i].u;
                    tcoord.data.v = model->tmap[i].v;
                break;
            }
            if(flags & M3D_EXP_FLIPTXTCRD)
                tcoord.data.v = (M3D_FLOAT)1.0 - tcoord.data.v;
            tcoord.oldidx = i;
            memcpy(&tmap[numtmap++], &tcoord, sizeof(m3dtisave_t));
        }
        if(numtmap) {
            qsort(tmap, numtmap, sizeof(m3dtisave_t), _m3d_ticmp);
            memcpy(&tcoord.data, &tmap[0], sizeof(m3dti_t));
            for(i = 0; i < numtmap; i++) {
                if(memcmp(&tcoord.data, &tmap[i].data, sizeof(m3dti_t))) {
                    memcpy(&tcoord.data, &tmap[i].data, sizeof(m3dti_t));
                    maxtmap++;
                }
                tmap[i].newidx = maxtmap;
                tmapidx[tmap[i].oldidx] = maxtmap;
            }
            maxtmap++;
        }
    }
    if(model->numskin && model->skin && !(flags & M3D_EXP_NOBONE)) {
        M3D_LOG("Compressing skin");
        skinidx = (M3D_INDEX*)M3D_MALLOC(model->numskin * sizeof(M3D_INDEX));
        if(!skinidx) goto memerr;
        skin = (m3dssave_t*)M3D_MALLOC(model->numskin * sizeof(m3dssave_t));
        if(!skin) goto memerr;
        memset(skinidx, 255, model->numskin * sizeof(M3D_INDEX));
        for(i = 0; i < model->numvertex; i++) {
            if(vrtxidx[i] != M3D_UNDEF && model->vertex[i].skinid < model->numskin)
                skinidx[model->vertex[i].skinid] = 0;
        }
        for(i = 0; i < model->numskin; i++) {
            if(skinidx[i] == M3D_UNDEF) continue;
            memset(&sk, 0, sizeof(m3dssave_t));
            for(j = 0, min_x = (M3D_FLOAT)0.0; j < M3D_NUMBONE && model->skin[i].boneid[j] != M3D_UNDEF &&
                model->skin[i].weight[j] > (M3D_FLOAT)0.0; j++) {
                    sk.data.boneid[j] = model->skin[i].boneid[j];
                    sk.data.weight[j] = model->skin[i].weight[j];
                    min_x += sk.data.weight[j];
            }
            if(j > maxbone) maxbone = j;
            if(min_x != (M3D_FLOAT)1.0 && min_x != (M3D_FLOAT)0.0)
                for(j = 0; j < M3D_NUMBONE && sk.data.weight[j] > (M3D_FLOAT)0.0; j++)
                    sk.data.weight[j] /= min_x;
            sk.oldidx = i;
            memcpy(&skin[numskin++], &sk, sizeof(m3dssave_t));
        }
        if(numskin) {
            qsort(skin, numskin, sizeof(m3dssave_t), _m3d_skincmp);
            memcpy(&sk.data, &skin[0].data, sizeof(m3ds_t));
            for(i = 0; i < numskin; i++) {
                if(memcmp(&sk.data, &skin[i].data, sizeof(m3ds_t))) {
                    memcpy(&sk.data, &skin[i].data, sizeof(m3ds_t));
                    maxskin++;
                }
                skin[i].newidx = maxskin;
                skinidx[skin[i].oldidx] = maxskin;
            }
            maxskin++;
        }
    }

    M3D_LOG("Compressing vertex list");
    min_x = min_y = min_z = (M3D_FLOAT)1e10;
    max_x = max_y = max_z = (M3D_FLOAT)-1e10;
    if(vrtxidx) {
        vrtx = (m3dvsave_t*)M3D_MALLOC(model->numvertex * sizeof(m3dvsave_t));
        if(!vrtx) goto memerr;
        for(i = numvrtx = 0; i < model->numvertex; i++) {
            if(vrtxidx[i] == M3D_UNDEF) continue;
            _m3d_round(quality, &model->vertex[i], &vertex.data);
            vertex.norm = norm ? norm[i] : 0;
            if(vertex.data.skinid != M3D_INDEXMAX && !vertex.norm) {
                vertex.data.skinid = vertex.data.skinid != M3D_UNDEF && skinidx ? skinidx[vertex.data.skinid] : M3D_UNDEF;
                if(vertex.data.x > max_x) max_x = vertex.data.x;
                if(vertex.data.x < min_x) min_x = vertex.data.x;
                if(vertex.data.y > max_y) max_y = vertex.data.y;
                if(vertex.data.y < min_y) min_y = vertex.data.y;
                if(vertex.data.z > max_z) max_z = vertex.data.z;
                if(vertex.data.z < min_z) min_z = vertex.data.z;
            }
#ifdef M3D_VERTEXTYPE
            vertex.data.type = 0;
#endif
            vertex.oldidx = i;
            memcpy(&vrtx[numvrtx++], &vertex, sizeof(m3dvsave_t));
        }
        if(numvrtx) {
            qsort(vrtx, numvrtx, sizeof(m3dvsave_t), _m3d_vrtxcmp);
            memcpy(&vertex.data, &vrtx[0].data, sizeof(m3dv_t));
            for(i = 0; i < numvrtx; i++) {
                if(memcmp(&vertex.data, &vrtx[i].data, vrtx[i].norm ? 3 * sizeof(M3D_FLOAT) : sizeof(m3dv_t))) {
                    memcpy(&vertex.data, &vrtx[i].data, sizeof(m3dv_t));
                    maxvrtx++;
                }
                vrtx[i].newidx = maxvrtx;
                vrtxidx[vrtx[i].oldidx] = maxvrtx;
            }
            maxvrtx++;
        }
    }
    if(norm) { M3D_FREE(norm); norm = NULL; }

    /* normalize to bounding cube */
    if(numvrtx && !(flags & M3D_EXP_NORECALC)) {
        M3D_LOG("Normalizing coordinates");
        if(min_x < (M3D_FLOAT)0.0) min_x = -min_x;
        if(max_x < (M3D_FLOAT)0.0) max_x = -max_x;
        if(min_y < (M3D_FLOAT)0.0) min_y = -min_y;
        if(max_y < (M3D_FLOAT)0.0) max_y = -max_y;
        if(min_z < (M3D_FLOAT)0.0) min_z = -min_z;
        if(max_z < (M3D_FLOAT)0.0) max_z = -max_z;
        scale = min_x;
        if(max_x > scale) scale = max_x;
        if(min_y > scale) scale = min_y;
        if(max_y > scale) scale = max_y;
        if(min_z > scale) scale = min_z;
        if(max_z > scale) scale = max_z;
        if(scale <= (M3D_FLOAT)0.0) scale = (M3D_FLOAT)1.0;
        if(scale != (M3D_FLOAT)1.0) {
            for(i = 0; i < numvrtx; i++) {
                if(vrtx[i].data.skinid == M3D_INDEXMAX) continue;
                vrtx[i].data.x /= scale;
                vrtx[i].data.y /= scale;
                vrtx[i].data.z /= scale;
            }
        }
    }
    if(model->scale > (M3D_FLOAT)0.0) scale = model->scale;
    if(scale <= (M3D_FLOAT)0.0) scale = (M3D_FLOAT)1.0;

    /* meta info */
    sn = _m3d_safestr(model->name && *model->name ? model->name : (char*)"(noname)", 2);
    sl = _m3d_safestr(model->license ? model->license : (char*)"MIT", 2);
    sa = _m3d_safestr(model->author ? model->author : getenv("LOGNAME"), 2);
    if(!sn || !sl || !sa) {
memerr: if(vrtxidx) M3D_FREE(vrtxidx);
        if(mtrlidx) M3D_FREE(mtrlidx);
        if(tmapidx) M3D_FREE(tmapidx);
        if(skinidx) M3D_FREE(skinidx);
        if(grpidx) M3D_FREE(grpidx);
        if(norm) M3D_FREE(norm);
        if(face) M3D_FREE(face);
        if(cmap) M3D_FREE(cmap);
        if(tmap) M3D_FREE(tmap);
        if(skin) M3D_FREE(skin);
        if(str) M3D_FREE(str);
        if(vrtx) M3D_FREE(vrtx);
        if(sn) M3D_FREE(sn);
        if(sl) M3D_FREE(sl);
        if(sa) M3D_FREE(sa);
        if(sd) M3D_FREE(sd);
        if(out) M3D_FREE(out);
        if(h) M3D_FREE(h);
        M3D_LOG("Out of memory");
        model->errcode = M3D_ERR_ALLOC;
        return NULL;
    }

    M3D_LOG("Serializing model");
#ifdef M3D_ASCII
    if(flags & M3D_EXP_ASCII) {
        /* use CRLF to make model creators on Win happy... */
        sd = _m3d_safestr(model->desc, 1);
        if(!sd) goto memerr;
        ol = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");
        /* header */
        len = 64 + (unsigned int)(strlen(sn) + strlen(sl) + strlen(sa) + strlen(sd));
        out = (unsigned char*)M3D_MALLOC(len);
        if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
        ptr = (char*)out;
        ptr += sprintf(ptr, "3dmodel %g\r\n%s\r\n%s\r\n%s\r\n%s\r\n\r\n", scale,
            sn, sl, sa, sd);
        M3D_FREE(sl); M3D_FREE(sa); M3D_FREE(sd);
        sl = sa = sd = NULL;
        /* preview chunk */
        if(model->preview.data && model->preview.length) {
            sl = _m3d_safestr(sn, 0);
            if(sl) {
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)20 + strlen(sl));
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Preview\r\n%s.png\r\n\r\n", sl);
                M3D_FREE(sl); sl = NULL;
            }
        }
        M3D_FREE(sn);  sn = NULL;
        /* texture map */
        if(numtmap && tmap && !(flags & M3D_EXP_NOTXTCRD) && !(flags & M3D_EXP_NOFACE)) {
            ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)(maxtmap * 32) + (uintptr_t)12);
            out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
            if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
            ptr += sprintf(ptr, "Textmap\r\n");
            last = M3D_UNDEF;
            for(i = 0; i < numtmap; i++) {
                if(tmap[i].newidx == last) continue;
                last = tmap[i].newidx;
                ptr += sprintf(ptr, "%g %g\r\n", tmap[i].data.u, tmap[i].data.v);
            }
            ptr += sprintf(ptr, "\r\n");
        }
        /* vertex chunk */
        if(numvrtx && vrtx && !(flags & M3D_EXP_NOFACE)) {
            ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)(maxvrtx * 128) + (uintptr_t)10);
            out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
            if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
            ptr += sprintf(ptr, "Vertex\r\n");
            last = M3D_UNDEF;
            for(i = 0; i < numvrtx; i++) {
                if(vrtx[i].newidx == last) continue;
                last = vrtx[i].newidx;
                ptr += sprintf(ptr, "%g %g %g %g", vrtx[i].data.x, vrtx[i].data.y, vrtx[i].data.z, vrtx[i].data.w);
                if(!(flags & M3D_EXP_NOCMAP) && vrtx[i].data.color)
                    ptr += sprintf(ptr, " #%08x", vrtx[i].data.color);
                if(!(flags & M3D_EXP_NOBONE) && model->numbone && maxskin && vrtx[i].data.skinid < M3D_INDEXMAX) {
                    if(skin[vrtx[i].data.skinid].data.weight[0] == (M3D_FLOAT)1.0)
                        ptr += sprintf(ptr, " %d", skin[vrtx[i].data.skinid].data.boneid[0]);
                    else
                        for(j = 0; j < M3D_NUMBONE && skin[vrtx[i].data.skinid].data.boneid[j] != M3D_UNDEF &&
                            skin[vrtx[i].data.skinid].data.weight[j] > (M3D_FLOAT)0.0; j++)
                            ptr += sprintf(ptr, " %d:%g", skin[vrtx[i].data.skinid].data.boneid[j],
                                skin[vrtx[i].data.skinid].data.weight[j]);
                }
                ptr += sprintf(ptr, "\r\n");
            }
            ptr += sprintf(ptr, "\r\n");
        }
        /* bones chunk */
        if(model->numbone && model->bone && !(flags & M3D_EXP_NOBONE)) {
            ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)9);
            for(i = 0; i < model->numbone; i++) {
                len += (unsigned int)strlen(model->bone[i].name) + 128;
            }
            out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
            if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
            ptr += sprintf(ptr, "Bones\r\n");
            ptr = _m3d_prtbone(ptr, model->bone, model->numbone, M3D_UNDEF, 0, vrtxidx);
            ptr += sprintf(ptr, "\r\n");
        }
        /* materials */
        if(model->nummaterial && !(flags & M3D_EXP_NOMATERIAL)) {
            for(j = 0; j < model->nummaterial; j++) {
                if(mtrlidx[j] == M3D_UNDEF || !model->material[j].numprop || !model->material[j].prop) continue;
                m = &model->material[j];
                sn = _m3d_safestr(m->name, 0);
                if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)strlen(sn) + (uintptr_t)12);
                for(i = 0; i < m->numprop; i++) {
                    if(m->prop[i].type < 128)
                        len += 32;
                    else if(m->prop[i].value.textureid < model->numtexture && model->texture[m->prop[i].value.textureid].name)
                        len += (unsigned int)strlen(model->texture[m->prop[i].value.textureid].name) + 16;
                }
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Material %s\r\n", sn);
                M3D_FREE(sn); sn = NULL;
                for(i = 0; i < m->numprop; i++) {
                    k = 256;
                    if(m->prop[i].type >= 128) {
                        for(l = 0; l < sizeof(m3d_propertytypes)/sizeof(m3d_propertytypes[0]); l++)
                            if(m->prop[i].type == m3d_propertytypes[l].id) {
                                sn = m3d_propertytypes[l].key;
                                break;
                            }
                        if(!sn)
                            for(l = 0; l < sizeof(m3d_propertytypes)/sizeof(m3d_propertytypes[0]); l++)
                                if(m->prop[i].type - 128 == m3d_propertytypes[l].id) {
                                    sn = m3d_propertytypes[l].key;
                                    break;
                                }
                        k = sn ? m3dpf_map : 256;
                    } else {
                        for(l = 0; l < sizeof(m3d_propertytypes)/sizeof(m3d_propertytypes[0]); l++)
                            if(m->prop[i].type == m3d_propertytypes[l].id) {
                                sn = m3d_propertytypes[l].key;
                                k = m3d_propertytypes[l].format;
                                break;
                            }
                    }
                    switch(k) {
                        case m3dpf_color: ptr += sprintf(ptr, "%s #%08x\r\n", sn, m->prop[i].value.color); break;
                        case m3dpf_uint8:
                        case m3dpf_uint16:
                        case m3dpf_uint32: ptr += sprintf(ptr, "%s %d\r\n", sn, m->prop[i].value.num); break;
                        case m3dpf_float:  ptr += sprintf(ptr, "%s %g\r\n", sn, m->prop[i].value.fnum); break;
                        case m3dpf_map:
                            if(m->prop[i].value.textureid < model->numtexture &&
                                model->texture[m->prop[i].value.textureid].name) {
                                sl = _m3d_safestr(model->texture[m->prop[i].value.textureid].name, 0);
                                if(!sl) { setlocale(LC_NUMERIC, ol); goto memerr; }
                                if(*sl)
                                    ptr += sprintf(ptr, "map_%s %s\r\n", sn, sl);
                                M3D_FREE(sn); M3D_FREE(sl); sl = NULL;
                            }
                        break;
                    }
                    sn = NULL;
                }
                ptr += sprintf(ptr, "\r\n");
            }
        }
        /* procedural face */
        if(model->numinlined && model->inlined && !(flags & M3D_EXP_NOFACE)) {
            /* all inlined assets which are not textures should be procedural surfaces */
            for(j = 0; j < model->numinlined; j++) {
                if(!model->inlined[j].name || !*model->inlined[j].name || !model->inlined[j].length || !model->inlined[j].data ||
                 (model->inlined[j].data[1] == 'P' && model->inlined[j].data[2] == 'N' && model->inlined[j].data[3] == 'G'))
                    continue;
                for(i = k = 0; i < model->numtexture; i++) {
                    if(!strcmp(model->inlined[j].name, model->texture[i].name)) { k = 1; break; }
                }
                if(k) continue;
                sn = _m3d_safestr(model->inlined[j].name, 0);
                if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)strlen(sn) + (uintptr_t)18);
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Procedural\r\n%s\r\n\r\n", sn);
                M3D_FREE(sn); sn = NULL;
            }
        }
        /* mesh face */
        if(model->numface && face && !(flags & M3D_EXP_NOFACE)) {
            ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)(model->numface * 128) + (uintptr_t)6);
            last = M3D_UNDEF;
#ifdef M3D_VERTEXMAX
            lastp = M3D_UNDEF;
#endif
            if(!(flags & M3D_EXP_NOMATERIAL))
                for(i = 0; i < model->numface; i++) {
                    j = face[i].data.materialid < model->nummaterial ? face[i].data.materialid : M3D_UNDEF;
                    if(j != last) {
                        last = j;
                        if(last < model->nummaterial)
                            len += (unsigned int)strlen(model->material[last].name);
                        len += 6;
                    }
#ifdef M3D_VERTEXMAX
                    j = face[i].data.paramid < model->numparam ? face[i].data.paramid : M3D_UNDEF;
                    if(j != lastp) {
                        lastp = j;
                        if(lastp < model->numparam)
                            len += (unsigned int)strlen(model->param[lastp].name);
                        len += 6;
                    }
#endif
                }
            out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
            if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
            ptr += sprintf(ptr, "Mesh\r\n");
            last = M3D_UNDEF;
#ifdef M3D_VERTEXMAX
            lastp = M3D_UNDEF;
#endif
            for(i = 0; i < model->numface; i++) {
                j = face[i].data.materialid < model->nummaterial ? face[i].data.materialid : M3D_UNDEF;
                if(!(flags & M3D_EXP_NOMATERIAL) && j != last) {
                    last = j;
                    if(last < model->nummaterial) {
                        sn = _m3d_safestr(model->material[last].name, 0);
                        if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                        ptr += sprintf(ptr, "use %s\r\n", sn);
                        M3D_FREE(sn); sn = NULL;
                    } else
                        ptr += sprintf(ptr, "use\r\n");
                }
#ifdef M3D_VERTEXMAX
                j = face[i].data.paramid < model->numparam ? face[i].data.paramid : M3D_UNDEF;
                if(!(flags & M3D_EXP_NOVRTMAX) && j != lastp) {
                    lastp = j;
                    if(lastp < model->numparam) {
                        sn = _m3d_safestr(model->param[lastp].name, 0);
                        if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                        ptr += sprintf(ptr, "par %s\r\n", sn);
                        M3D_FREE(sn); sn = NULL;
                    } else
                        ptr += sprintf(ptr, "par\r\n");
                }
#endif
                /* hardcoded triangles. Should be repeated as many times as the number of edges in polygon */
                for(j = 0; j < 3; j++) {
                    ptr += sprintf(ptr, "%s%d", j?" ":"", vrtxidx[face[i].data.vertex[j]]);
                    k = l = M3D_NOTDEFINED;
                    if(!(flags & M3D_EXP_NOTXTCRD) && (face[i].data.texcoord[j] != M3D_UNDEF) &&
                        (tmapidx[face[i].data.texcoord[j]] != M3D_UNDEF)) {
                            k = tmapidx[face[i].data.texcoord[j]];
                            ptr += sprintf(ptr, "/%d", k);
                    }
                    if(!(flags & M3D_EXP_NONORMAL) && (face[i].data.normal[j] != M3D_UNDEF)) {
                        l = vrtxidx[face[i].data.normal[j]];
                        ptr += sprintf(ptr, "%s/%d", k == M3D_NOTDEFINED? "/" : "", l);
                    }
#ifdef M3D_VERTEXMAX
                    if(!(flags & M3D_EXP_NOVRTMAX) && (face[i].data.vertmax[j] != M3D_UNDEF)) {
                        ptr += sprintf(ptr, "%s%s/%d", k == M3D_NOTDEFINED? "/" : "", l == M3D_NOTDEFINED? "/" : "",
                            vrtxidx[face[i].data.vertmax[j]]);
                    }
#endif
                }
                ptr += sprintf(ptr, "\r\n");
            }
            ptr += sprintf(ptr, "\r\n");
        }
        /* voxel face */
        if(model->numvoxtype && model->voxtype && !(flags & M3D_EXP_NOFACE)) {
            ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)(model->numvoxtype * 128) + (uintptr_t)10);
            for(i = 0; i < model->numvoxtype; i++) {
                if(model->voxtype[i].name) len += (unsigned int)strlen(model->voxtype[i].name);
                for(j = 0; j < model->voxtype[i].numitem; j++)
                    if(model->voxtype[i].item[j].name)
                        len += (unsigned int)strlen(model->voxtype[i].item[j].name) + 6;
            }
            out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
            if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
            ptr += sprintf(ptr, "VoxTypes\r\n");
            for(i = 0; i < model->numvoxtype; i++) {
                ptr += sprintf(ptr, "#%08x", model->voxtype[i].color);
                if(model->voxtype[i].rotation)
                    ptr += sprintf(ptr, "/%02x", model->voxtype[i].rotation);
                if(model->voxtype[i].voxshape)
                    ptr += sprintf(ptr, "%s/%03x", model->voxtype[i].rotation ? "" : "/", model->voxtype[i].voxshape);
                sn = _m3d_safestr(model->voxtype[i].name, 0);
                if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, " %s", sn && sn[0] ? sn : "-");
                M3D_FREE(sn); sn = NULL;
                if(!(flags & M3D_EXP_NOBONE) && model->numbone && maxskin && model->voxtype[i].skinid < M3D_INDEXMAX) {
                    if(skin[skinidx[model->voxtype[i].skinid]].data.weight[0] == (M3D_FLOAT)1.0)
                        ptr += sprintf(ptr, " %d", skin[skinidx[model->voxtype[i].skinid]].data.boneid[0]);
                    else
                        for(j = 0; j < M3D_NUMBONE && skin[skinidx[model->voxtype[i].skinid]].data.boneid[j] != M3D_UNDEF &&
                            skin[skinidx[model->voxtype[i].skinid]].data.weight[j] > (M3D_FLOAT)0.0; j++)
                            ptr += sprintf(ptr, " %d:%g", skin[skinidx[model->voxtype[i].skinid]].data.boneid[j],
                                skin[skinidx[model->voxtype[i].skinid]].data.weight[j]);
                }
                if(model->voxtype[i].numitem && model->voxtype[i].item) {
                    for(j = k = 0; j < model->voxtype[i].numitem; j++) {
                        if(!model->voxtype[i].item[j].count || !model->voxtype[i].item[j].name ||
                            !model->voxtype[i].item[j].name[0]) continue;
                        if(!k) { ptr += sprintf(ptr, " {"); k = 1; }
                        sn = _m3d_safestr(model->voxtype[i].item[j].name, 0);
                        if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                        ptr += sprintf(ptr, " %d %s", model->voxtype[i].item[j].count, sn);
                        M3D_FREE(sn); sn = NULL;
                    }
                    if(k) ptr += sprintf(ptr, " }");
                }
                while(ptr[-1] == '-' || ptr[-1] == ' ') ptr--;
                ptr += sprintf(ptr, "\r\n");
            }
            ptr += sprintf(ptr, "\r\n");
        }
        if(model->numvoxel && model->voxel && !(flags & M3D_EXP_NOFACE)) {
            for(i = 0; i < model->numvoxel; i++) {
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)128);
                if(model->voxel[i].name) len += (unsigned int)strlen(model->voxel[i].name);
                len += model->voxel[i].h * ((model->voxel[i].w * 6 + 2) * model->voxel[i].d + 9);
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Voxel");
                sn = _m3d_safestr(model->voxel[i].name, 0);
                if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                if(sn && sn[0])
                    ptr += sprintf(ptr, " %s", sn);
                M3D_FREE(sn); sn = NULL;
                ptr += sprintf(ptr, "\r\n");
                if(model->voxel[i].uncertain)
                    ptr += sprintf(ptr, "uncertain %d %d\r\n", (model->voxel[i].uncertain * 100) / 255, model->voxel[i].groupid);
                if(model->voxel[i].x || model->voxel[i].y || model->voxel[i].z)
                    ptr += sprintf(ptr, "pos %d %d %d\r\n", model->voxel[i].x, model->voxel[i].y, model->voxel[i].z);
                ptr += sprintf(ptr, "dim %d %d %d\r\n", model->voxel[i].w, model->voxel[i].h, model->voxel[i].d);
                for(j = n = 0; j < model->voxel[i].h; j++) {
                    ptr += sprintf(ptr, "layer\r\n");
                    for(k = 0; k < model->voxel[i].d; k++) {
                        for(l = 0; l < model->voxel[i].w; l++, n++) {
                            switch(model->voxel[i].data[n]) {
                                case M3D_VOXCLEAR: *ptr++ = '-'; break;
                                case M3D_VOXUNDEF: *ptr++ = '.'; break;
                                default: ptr += sprintf(ptr, "%d", model->voxel[i].data[n]); break;
                            }
                            *ptr++ = ' ';
                        }
                        ptr--;
                        ptr += sprintf(ptr, "\r\n");
                    }
                }
                ptr += sprintf(ptr, "\r\n");
            }
        }
        /* mathematical shapes face */
        if(model->numshape && model->numshape && !(flags & M3D_EXP_NOFACE)) {
            for(j = 0; j < model->numshape; j++) {
                sn = _m3d_safestr(model->shape[j].name, 0);
                if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)strlen(sn) + (uintptr_t)33);
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Shape %s\r\n", sn);
                M3D_FREE(sn); sn = NULL;
                if(model->shape[j].group != M3D_UNDEF && !(flags & M3D_EXP_NOBONE))
                    ptr += sprintf(ptr, "group %d\r\n", model->shape[j].group);
                for(i = 0; i < model->shape[j].numcmd; i++) {
                    cmd = &model->shape[j].cmd[i];
                    if(cmd->type >= (unsigned int)(sizeof(m3d_commandtypes)/sizeof(m3d_commandtypes[0])) || !cmd->arg)
                        continue;
                    cd = &m3d_commandtypes[cmd->type];
                    ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)strlen(cd->key) + (uintptr_t)3);
                    for(k = 0; k < cd->p; k++)
                        switch(cd->a[k]) {
                            case m3dcp_mi_t: if(cmd->arg[k] != M3D_NOTDEFINED) { len += (unsigned int)strlen(model->material[cmd->arg[k]].name) + 1; } break;
                            case m3dcp_va_t: len += cmd->arg[k] * (cd->p - k - 1) * 16; k = cd->p; break;
                            default: len += 16; break;
                        }
                    out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                    if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                    ptr += sprintf(ptr, "%s", cd->key);
                    for(k = n = 0, l = cd->p; k < l; k++) {
                        switch(cd->a[((k - n) % (cd->p - n)) + n]) {
                            case m3dcp_mi_t:
                                if(cmd->arg[k] != M3D_NOTDEFINED) {
                                    sn = _m3d_safestr(model->material[cmd->arg[k]].name, 0);
                                    if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                                    ptr += sprintf(ptr, " %s", sn);
                                    M3D_FREE(sn); sn = NULL;
                                }
                            break;
                            case m3dcp_vc_t: ptr += sprintf(ptr, " %g", *((float*)&cmd->arg[k])); break;
                            case m3dcp_va_t: ptr += sprintf(ptr, " %d[", cmd->arg[k]);
                                n = k + 1; l += (cmd->arg[k] - 1) * (cd->p - k - 1);
                            break;
                            default: ptr += sprintf(ptr, " %d", cmd->arg[k]); break;
                        }
                    }
                    ptr += sprintf(ptr, "%s\r\n", l > cd->p ? " ]" : "");
                }
                ptr += sprintf(ptr, "\r\n");
            }
        }
        /* annotation labels */
        if(model->numlabel && model->label && !(flags & M3D_EXP_NOFACE)) {
            for(i = 0, j = 3, length = NULL; i < model->numlabel; i++) {
                if(model->label[i].name) j += (unsigned int)strlen(model->label[i].name);
                if(model->label[i].lang) j += (unsigned int)strlen(model->label[i].lang);
                if(model->label[i].text) j += (unsigned int)strlen(model->label[i].text);
                j += 40;
            }
            ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)j);
            out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
            if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
            for(i = 0; i < model->numlabel; i++) {
                if(!i || _m3d_strcmp(sl, model->label[i].lang) || _m3d_strcmp(sn, model->label[i].name)) {
                    sl = model->label[i].lang;
                    sn = model->label[i].name;
                    sd = _m3d_safestr(sn, 0);
                    if(!sd) { setlocale(LC_NUMERIC, ol); sn = sl = NULL; goto memerr; }
                    if(i) ptr += sprintf(ptr, "\r\n");
                    ptr += sprintf(ptr, "Labels %s\r\n", sd);
                    M3D_FREE(sd); sd = NULL;
                    if(model->label[i].color)
                        ptr += sprintf(ptr, "color #0x%08x\r\n", model->label[i].color);
                    if(sl && *sl) {
                        sd = _m3d_safestr(sl, 0);
                        if(!sd) { setlocale(LC_NUMERIC, ol); sn = sl = NULL; goto memerr; }
                        ptr += sprintf(ptr, "lang %s\r\n", sd);
                        M3D_FREE(sd); sd = NULL;
                    }
                }
                sd = _m3d_safestr(model->label[i].text, 2);
                if(!sd) { setlocale(LC_NUMERIC, ol); sn = sl = NULL; goto memerr; }
                ptr += sprintf(ptr, "%d %s\r\n", model->label[i].vertexid, sd);
                M3D_FREE(sd); sd = NULL;
            }
            ptr += sprintf(ptr, "\r\n");
            sn = sl = NULL;
        }
        /* actions */
        if(model->numaction && model->action && !(flags & M3D_EXP_NOACTION)) {
            for(j = 0; j < model->numaction; j++) {
                a = &model->action[j];
                sn = _m3d_safestr(a->name, 0);
                if(!sn) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)strlen(sn) + (uintptr_t)48);
                for(i = 0; i < a->numframe; i++)
                    len += a->frame[i].numtransform * 128 + 8;
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Action %d %s\r\n", a->durationmsec, sn);
                M3D_FREE(sn); sn = NULL;
                for(i = 0; i < a->numframe; i++) {
                    ptr += sprintf(ptr, "frame %d\r\n", a->frame[i].msec);
                    for(k = 0; k < a->frame[i].numtransform; k++) {
                        ptr += sprintf(ptr, "%d %d %d\r\n", a->frame[i].transform[k].boneid,
                            vrtxidx[a->frame[i].transform[k].pos], vrtxidx[a->frame[i].transform[k].ori]);
                    }
                }
                ptr += sprintf(ptr, "\r\n");
            }
        }
        /* inlined assets */
        if(model->numinlined && model->inlined) {
            for(i = j = 0; i < model->numinlined; i++)
                if(model->inlined[i].name)
                    j += (unsigned int)strlen(model->inlined[i].name) + 6;
            if(j > 0) {
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)j + (uintptr_t)16);
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Assets\r\n");
                for(i = 0; i < model->numinlined; i++)
                    if(model->inlined[i].name)
                        ptr += sprintf(ptr, "%s%s\r\n", model->inlined[i].name, strrchr(model->inlined[i].name, '.') ? "" : ".png");
                ptr += sprintf(ptr, "\r\n");
            }
        }
        /* extra info */
        if(model->numextra && (flags & M3D_EXP_EXTRA)) {
            for(i = 0; i < model->numextra; i++) {
                if(model->extra[i]->length < 9) continue;
                ptr -= (uintptr_t)out; len = (unsigned int)((uintptr_t)ptr + (uintptr_t)17 + (uintptr_t)(model->extra[i]->length * 3));
                out = (unsigned char*)M3D_REALLOC(out, len); ptr += (uintptr_t)out;
                if(!out) { setlocale(LC_NUMERIC, ol); goto memerr; }
                ptr += sprintf(ptr, "Extra %c%c%c%c\r\n",
                    model->extra[i]->magic[0] > ' ' ? model->extra[i]->magic[0] : '_',
                    model->extra[i]->magic[1] > ' ' ? model->extra[i]->magic[1] : '_',
                    model->extra[i]->magic[2] > ' ' ? model->extra[i]->magic[2] : '_',
                    model->extra[i]->magic[3] > ' ' ? model->extra[i]->magic[3] : '_');
                for(j = 0; j < model->extra[i]->length; j++)
                    ptr += sprintf(ptr, "%02x ", *((unsigned char *)model->extra + sizeof(m3dchunk_t) + j));
                ptr--;
                ptr += sprintf(ptr, "\r\n\r\n");
            }
        }
        setlocale(LC_NUMERIC, ol);
        len = (unsigned int)((uintptr_t)ptr - (uintptr_t)out);
        out = (unsigned char*)M3D_REALLOC(out, len + 1);
        if(!out) goto memerr;
        out[len] = 0;
    } else
#endif
    {
        /* stricly only use LF (newline) in binary */
        sd = _m3d_safestr(model->desc, 3);
        if(!sd) goto memerr;
        /* header */
        h = (m3dhdr_t*)M3D_MALLOC(sizeof(m3dhdr_t) + strlen(sn) + strlen(sl) + strlen(sa) + strlen(sd) + 4);
        if(!h) goto memerr;
        memcpy((uint8_t*)h, "HEAD", 4);
        h->length = sizeof(m3dhdr_t);
        h->scale = scale;
        i = (unsigned int)strlen(sn); memcpy((uint8_t*)h + h->length, sn, i+1); h->length += i+1; M3D_FREE(sn);
        i = (unsigned int)strlen(sl); memcpy((uint8_t*)h + h->length, sl, i+1); h->length += i+1; M3D_FREE(sl);
        i = (unsigned int)strlen(sa); memcpy((uint8_t*)h + h->length, sa, i+1); h->length += i+1; M3D_FREE(sa);
        i = (unsigned int)strlen(sd); memcpy((uint8_t*)h + h->length, sd, i+1); h->length += i+1; M3D_FREE(sd);
        sn = sl = sa = sd = NULL;
        if(model->inlined)
            for(i = 0; i < model->numinlined; i++) {
                if(model->inlined[i].name && *model->inlined[i].name && model->inlined[i].length > 0) {
                    str = _m3d_addstr(str, &numstr, model->inlined[i].name);
                    if(!str) goto memerr;
                }
            }
        if(str)
            for(i = 0; i < numstr; i++) {
                h = _m3d_addhdr(h, &str[i]);
                if(!h) goto memerr;
            }
        vc_s = quality == M3D_EXP_INT8? 1 : (quality == M3D_EXP_INT16? 2 : (quality == M3D_EXP_DOUBLE? 8 : 4));
        vi_s = maxvrtx < 254 ? 1 : (maxvrtx < 65534 ? 2 : 4);
        si_s = h->length - 16 < 254 ? 1 : (h->length - 16 < 65534 ? 2 : 4);
        ci_s = !numcmap || !cmap ? 0 : (numcmap < 254 ? 1 : (numcmap < 65534 ? 2 : 4));
        ti_s = !maxtmap || !tmap ? 0 : (maxtmap < 254 ? 1 : (maxtmap < 65534 ? 2 : 4));
        bi_s = !model->numbone || !model->bone || (flags & M3D_EXP_NOBONE)? 0 : (model->numbone < 254 ? 1 :
            (model->numbone < 65534 ? 2 : 4));
        nb_s = maxbone < 2 ? 1 : (maxbone == 2 ? 2 : (maxbone <= 4 ? 4 : 8));
        sk_s = !bi_s || !maxskin || !skin ? 0 : (maxskin < 254 ? 1 : (maxskin < 65534 ? 2 : 4));
        fc_s = maxt < 254 ? 1 : (maxt < 65534 ? 2 : 4);
        hi_s = !model->numshape || !model->shape || (flags & M3D_EXP_NOFACE)? 0 : (model->numshape < 254 ? 1 :
            (model->numshape < 65534 ? 2 : 4));
        fi_s = !model->numface || !model->face || (flags & M3D_EXP_NOFACE)? 0 : (model->numface < 254 ? 1 :
            (model->numface < 65534 ? 2 : 4));
        vd_s = !model->numvoxel || !model->voxel || (flags & M3D_EXP_NOFACE)? 0 : (minvox >= -128 && maxvox <= 127 ? 1 :
            (minvox >= -32768 && maxvox <= 32767 ? 2 : 4));
        vp_s = !model->numvoxtype || !model->voxtype || (flags & M3D_EXP_NOFACE)? 0 : (model->numvoxtype < 254 ? 1 :
            (model->numvoxtype < 65534 ? 2 : 4));
        h->types =  (vc_s == 8 ? (3<<0) : (vc_s == 2 ? (1<<0) : (vc_s == 1 ? (0<<0) : (2<<0)))) |
                    (vi_s == 2 ? (1<<2) : (vi_s == 1 ? (0<<2) : (2<<2))) |
                    (si_s == 2 ? (1<<4) : (si_s == 1 ? (0<<4) : (2<<4))) |
                    (ci_s == 2 ? (1<<6) : (ci_s == 1 ? (0<<6) : (ci_s == 4 ? (2<<6) : (3<<6)))) |
                    (ti_s == 2 ? (1<<8) : (ti_s == 1 ? (0<<8) : (ti_s == 4 ? (2<<8) : (3<<8)))) |
                    (bi_s == 2 ? (1<<10): (bi_s == 1 ? (0<<10): (bi_s == 4 ? (2<<10) : (3<<10)))) |
                    (nb_s == 2 ? (1<<12): (nb_s == 1 ? (0<<12): (2<<12))) |
                    (sk_s == 2 ? (1<<14): (sk_s == 1 ? (0<<14): (sk_s == 4 ? (2<<14) : (3<<14)))) |
                    (fc_s == 2 ? (1<<16): (fc_s == 1 ? (0<<16): (2<<16))) |
                    (hi_s == 2 ? (1<<18): (hi_s == 1 ? (0<<18): (hi_s == 4 ? (2<<18) : (3<<18)))) |
                    (fi_s == 2 ? (1<<20): (fi_s == 1 ? (0<<20): (fi_s == 4 ? (2<<20) : (3<<20)))) |
                    (vd_s == 2 ? (1<<22): (vd_s == 1 ? (0<<22): (vd_s == 4 ? (2<<22) : (3<<22)))) |
                    (vp_s == 2 ? (1<<24): (vp_s == 1 ? (0<<24): (vp_s == 4 ? (2<<24) : (3<<24))));
        len = h->length;
        /* color map */
        if(numcmap && cmap && ci_s < 4 && !(flags & M3D_EXP_NOCMAP)) {
            chunklen = 8 + numcmap * sizeof(uint32_t);
            h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
            if(!h) goto memerr;
            memcpy((uint8_t*)h + len, "CMAP", 4);
            *((uint32_t*)((uint8_t*)h + len + 4)) = chunklen;
            memcpy((uint8_t*)h + len + 8, cmap, chunklen - 8);
            len += chunklen;
        } else numcmap = 0;
        /* texture map */
        if(numtmap && tmap && !(flags & M3D_EXP_NOTXTCRD) && !(flags & M3D_EXP_NOFACE)) {
            chunklen = 8 + maxtmap * vc_s * 2;
            h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
            if(!h) goto memerr;
            memcpy((uint8_t*)h + len, "TMAP", 4);
            length = (uint32_t*)((uint8_t*)h + len + 4);
            out = (uint8_t*)h + len + 8;
            last = M3D_UNDEF;
            for(i = 0; i < numtmap; i++) {
                if(tmap[i].newidx == last) continue;
                last = tmap[i].newidx;
                switch(vc_s) {
                    case 1: *out++ = (uint8_t)(tmap[i].data.u * 255); *out++ = (uint8_t)(tmap[i].data.v * 255); break;
                    case 2:
                        *((uint16_t*)out) = (uint16_t)(tmap[i].data.u * 65535); out += 2;
                        *((uint16_t*)out) = (uint16_t)(tmap[i].data.v * 65535); out += 2;
                    break;
                    case 4:  *((float*)out) = tmap[i].data.u; out += 4;  *((float*)out) = tmap[i].data.v; out += 4; break;
                    case 8: *((double*)out) = tmap[i].data.u; out += 8; *((double*)out) = tmap[i].data.v; out += 8; break;
                }
            }
            *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
            out = NULL;
            len += *length;
        }
        /* vertex */
        if(numvrtx && vrtx) {
            chunklen = 8 + maxvrtx * (ci_s + sk_s + 4 * vc_s);
            h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
            if(!h) goto memerr;
            memcpy((uint8_t*)h + len, "VRTS", 4);
            length = (uint32_t*)((uint8_t*)h + len + 4);
            out = (uint8_t*)h + len + 8;
            last = M3D_UNDEF;
            for(i = 0; i < numvrtx; i++) {
                if(vrtx[i].newidx == last) continue;
                last = vrtx[i].newidx;
                switch(vc_s) {
                    case 1:
                        *out++ = (int8_t)(vrtx[i].data.x * 127);
                        *out++ = (int8_t)(vrtx[i].data.y * 127);
                        *out++ = (int8_t)(vrtx[i].data.z * 127);
                        *out++ = (int8_t)(vrtx[i].data.w * 127);
                    break;
                    case 2:
                        *((int16_t*)out) = (int16_t)(vrtx[i].data.x * 32767); out += 2;
                        *((int16_t*)out) = (int16_t)(vrtx[i].data.y * 32767); out += 2;
                        *((int16_t*)out) = (int16_t)(vrtx[i].data.z * 32767); out += 2;
                        *((int16_t*)out) = (int16_t)(vrtx[i].data.w * 32767); out += 2;
                    break;
                    case 4:
                        *((float*)out) = vrtx[i].data.x; out += 4;
                        *((float*)out) = vrtx[i].data.y; out += 4;
                        *((float*)out) = vrtx[i].data.z; out += 4;
                        *((float*)out) = vrtx[i].data.w; out += 4;
                    break;
                    case 8:
                        *((double*)out) = vrtx[i].data.x; out += 8;
                        *((double*)out) = vrtx[i].data.y; out += 8;
                        *((double*)out) = vrtx[i].data.z; out += 8;
                        *((double*)out) = vrtx[i].data.w; out += 8;
                    break;
                }
                idx = _m3d_cmapidx(cmap, numcmap, vrtx[i].data.color);
                switch(ci_s) {
                    case 1: *out++ = (uint8_t)(idx); break;
                    case 2: *((uint16_t*)out) = (uint16_t)(idx); out += 2; break;
                    case 4: *((uint32_t*)out) = vrtx[i].data.color; out += 4; break;
                }
                out = _m3d_addidx(out, sk_s, vrtx[i].data.skinid);
            }
            *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
            out = NULL;
            len += *length;
        }
        /* bones chunk */
        if(model->numbone && model->bone && !(flags & M3D_EXP_NOBONE)) {
            i = 8 + bi_s + sk_s + model->numbone * (bi_s + si_s + 2*vi_s);
            chunklen = i + numskin * nb_s * (bi_s + 1);
            h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
            if(!h) goto memerr;
            memcpy((uint8_t*)h + len, "BONE", 4);
            length = (uint32_t*)((uint8_t*)h + len + 4);
            out = (uint8_t*)h + len + 8;
            out = _m3d_addidx(out, bi_s, model->numbone);
            out = _m3d_addidx(out, sk_s, maxskin);
            for(i = 0; i < model->numbone; i++) {
                out = _m3d_addidx(out, bi_s, model->bone[i].parent);
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->bone[i].name));
                out = _m3d_addidx(out, vi_s, vrtxidx[model->bone[i].pos]);
                out = _m3d_addidx(out, vi_s, vrtxidx[model->bone[i].ori]);
            }
            if(numskin && skin && sk_s) {
                last = M3D_UNDEF;
                for(i = 0; i < numskin; i++) {
                    if(skin[i].newidx == last) continue;
                    last = skin[i].newidx;
                    memset(&weights, 0, nb_s);
                    for(j = 0; j < (uint32_t)nb_s && skin[i].data.boneid[j] != M3D_UNDEF &&
                        skin[i].data.weight[j] > (M3D_FLOAT)0.0; j++)
                            weights[j] = (uint8_t)(skin[i].data.weight[j] * 255);
                    switch(nb_s) {
                        case 1: weights[0] = 255; break;
                        case 2: memcpy(out, weights, 2); out += 2; break;
                        case 4: memcpy(out, weights, 4); out += 4; break;
                        case 8: memcpy(out, weights, 8); out += 8; break;
                    }
                    for(j = 0; j < (uint32_t)nb_s && skin[i].data.boneid[j] != M3D_UNDEF && weights[j]; j++) {
                        out = _m3d_addidx(out, bi_s, skin[i].data.boneid[j]);
                        *length += bi_s;
                    }
                }
            }
            *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
            out = NULL;
            len += *length;
        }
        /* materials */
        if(model->nummaterial && !(flags & M3D_EXP_NOMATERIAL)) {
            for(j = 0; j < model->nummaterial; j++) {
                if(mtrlidx[j] == M3D_UNDEF || !model->material[j].numprop || !model->material[j].prop) continue;
                m = &model->material[j];
                chunklen = 12 + si_s + m->numprop * 5;
                h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                if(!h) goto memerr;
                memcpy((uint8_t*)h + len, "MTRL", 4);
                length = (uint32_t*)((uint8_t*)h + len + 4);
                out = (uint8_t*)h + len + 8;
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, m->name));
                for(i = 0; i < m->numprop; i++) {
                    if(m->prop[i].type >= 128) {
                        if(m->prop[i].value.textureid >= model->numtexture ||
                            !model->texture[m->prop[i].value.textureid].name) continue;
                        k = m3dpf_map;
                    } else {
                        for(k = 256, l = 0; l < sizeof(m3d_propertytypes)/sizeof(m3d_propertytypes[0]); l++)
                            if(m->prop[i].type == m3d_propertytypes[l].id) { k = m3d_propertytypes[l].format; break; }
                    }
                    if(k == 256) continue;
                    *out++ = m->prop[i].type;
                    switch(k) {
                        case m3dpf_color:
                            if(!(flags & M3D_EXP_NOCMAP)) {
                                idx = _m3d_cmapidx(cmap, numcmap, m->prop[i].value.color);
                                switch(ci_s) {
                                    case 1: *out++ = (uint8_t)(idx); break;
                                    case 2: *((uint16_t*)out) = (uint16_t)(idx); out += 2; break;
                                    case 4: *((uint32_t*)out) = (uint32_t)(m->prop[i].value.color); out += 4; break;
                                }
                            } else out--;
                        break;
                        case m3dpf_uint8:  *out++ = m->prop[i].value.num; break;
                        case m3dpf_uint16: *((uint16_t*)out) = m->prop[i].value.num; out += 2; break;
                        case m3dpf_uint32: *((uint32_t*)out) = m->prop[i].value.num; out += 4; break;
                        case m3dpf_float:  *((float*)out) = m->prop[i].value.fnum; out += 4; break;

                        case m3dpf_map:
                            idx = _m3d_stridx(str, numstr, model->texture[m->prop[i].value.textureid].name);
                            out = _m3d_addidx(out, si_s, idx);
                        break;
                    }
                }
                *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
                len += *length;
                out = NULL;
            }
        }
        /* procedural face */
        if(model->numinlined && model->inlined && !(flags & M3D_EXP_NOFACE)) {
            /* all inlined assets which are not textures should be procedural surfaces */
            for(j = 0; j < model->numinlined; j++) {
                if(!model->inlined[j].name || !model->inlined[j].name[0] || model->inlined[j].length < 4 ||
                    !model->inlined[j].data || (model->inlined[j].data[1] == 'P' && model->inlined[j].data[2] == 'N' &&
                    model->inlined[j].data[3] == 'G'))
                    continue;
                for(i = k = 0; i < model->numtexture; i++) {
                    if(!strcmp(model->inlined[j].name, model->texture[i].name)) { k = 1; break; }
                }
                if(k) continue;
                numproc++;
                chunklen = 8 + si_s;
                h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                if(!h) goto memerr;
                memcpy((uint8_t*)h + len, "PROC", 4);
                *((uint32_t*)((uint8_t*)h + len + 4)) = chunklen;
                out = (uint8_t*)h + len + 8;
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->inlined[j].name));
                out = NULL;
                len += chunklen;
            }
        }
        /* mesh face */
        if(model->numface && face && !(flags & M3D_EXP_NOFACE)) {
            chunklen = 8 + si_s + model->numface * (6 * vi_s + 3 * ti_s + si_s + 1);
            h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
            if(!h) goto memerr;
            memcpy((uint8_t*)h + len, "MESH", 4);
            length = (uint32_t*)((uint8_t*)h + len + 4);
            out = (uint8_t*)h + len + 8;
            last = M3D_UNDEF;
#ifdef M3D_VERTEXMAX
            lastp = M3D_UNDEF;
#endif
            for(i = 0; i < model->numface; i++) {
                if(!(flags & M3D_EXP_NOMATERIAL) && face[i].data.materialid != last) {
                    last = face[i].data.materialid;
                    idx = last < model->nummaterial ? _m3d_stridx(str, numstr, model->material[last].name) : 0;
                    *out++ = 0;
                    out = _m3d_addidx(out, si_s, idx);
                }
#ifdef M3D_VERTEXMAX
                if(!(flags & M3D_EXP_NOVRTMAX) && face[i].data.paramid != lastp) {
                    lastp = face[i].data.paramid;
                    idx = lastp < model->numparam ? _m3d_stridx(str, numstr, model->param[lastp].name) : 0;
                    *out++ = 0;
                    out = _m3d_addidx(out, si_s, idx);
                }
#endif
                /* hardcoded triangles. */
                k = (3 << 4) |
                    (((flags & M3D_EXP_NOTXTCRD) || !ti_s || face[i].data.texcoord[0] == M3D_UNDEF ||
                    face[i].data.texcoord[1] == M3D_UNDEF || face[i].data.texcoord[2] == M3D_UNDEF) ? 0 : 1) |
                    (((flags & M3D_EXP_NONORMAL) || face[i].data.normal[0] == M3D_UNDEF ||
                    face[i].data.normal[1] == M3D_UNDEF || face[i].data.normal[2] == M3D_UNDEF) ? 0 : 2)
#ifdef M3D_VERTEXMAX
                    | (((flags & M3D_EXP_NOVRTMAX) || face[i].data.vertmax[0] == M3D_UNDEF ||
                    face[i].data.vertmax[1] == M3D_UNDEF || face[i].data.vertmax[2] == M3D_UNDEF) ? 0 : 4)
#endif
                    ;
                *out++ = k;
                for(j = 0; j < 3; j++) {
                    out = _m3d_addidx(out, vi_s, vrtxidx[face[i].data.vertex[j]]);
                    if(k & 1)
                        out = _m3d_addidx(out, ti_s, tmapidx[face[i].data.texcoord[j]]);
                    if(k & 2)
                        out = _m3d_addidx(out, vi_s, vrtxidx[face[i].data.normal[j]]);
#ifdef M3D_VERTEXMAX
                    if(k & 4)
                        out = _m3d_addidx(out, vi_s, vrtxidx[face[i].data.vertmax[j]]);
#endif
                }
            }
            *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
            len += *length;
            out = NULL;
        }
        /* voxel face */
        if(model->numvoxtype && model->voxtype && !(flags & M3D_EXP_NOFACE)) {
            chunklen = 8 + si_s + model->numvoxtype * (ci_s + si_s + 3 + sk_s);
            for(i = 0; i < model->numvoxtype; i++)
                chunklen += model->voxtype[i].numitem * (2 + si_s);
            h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
            if(!h) goto memerr;
            memcpy((uint8_t*)h + len, "VOXT", 4);
            length = (uint32_t*)((uint8_t*)h + len + 4);
            out = (uint8_t*)h + len + 8;
            for(i = 0; i < model->numvoxtype; i++) {
                if(!(flags & M3D_EXP_NOCMAP)) {
                    idx = _m3d_cmapidx(cmap, numcmap, model->voxtype[i].color);
                    switch(ci_s) {
                        case 1: *out++ = (uint8_t)(idx); break;
                        case 2: *((uint16_t*)out) = (uint16_t)(idx); out += 2; break;
                        case 4: *((uint32_t*)out) = (uint32_t)(model->voxtype[i].color); out += 4; break;
                    }
                }
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->voxtype[i].name));
                *out++ = (model->voxtype[i].rotation & 0xBF) | (((model->voxtype[i].voxshape >> 8) & 1) << 6);
                *out++ = model->voxtype[i].voxshape;
                *out++ = model->voxtype[i].numitem;
                if(!(flags & M3D_EXP_NOBONE) && model->numbone && maxskin)
                    out = _m3d_addidx(out, sk_s, skinidx[model->voxtype[i].skinid]);
                for(j = 0; j < model->voxtype[i].numitem; j++) {
                    out = _m3d_addidx(out, 2, model->voxtype[i].item[j].count);
                    out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->voxtype[i].item[j].name));
                }
            }
            *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
            len += *length;
            out = NULL;
        }
        if(model->numvoxel && model->voxel && !(flags & M3D_EXP_NOFACE)) {
            for(j = 0; j < model->numvoxel; j++) {
                chunklen = 8 + si_s + 6 * vd_s + 2 + model->voxel[j].w * model->voxel[j].h * model->voxel[j].d * 3;
                h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                if(!h) goto memerr;
                memcpy((uint8_t*)h + len, "VOXD", 4);
                length = (uint32_t*)((uint8_t*)h + len + 4);
                out = (uint8_t*)h + len + 8;
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->voxel[j].name));
                out = _m3d_addidx(out, vd_s, model->voxel[j].x);
                out = _m3d_addidx(out, vd_s, model->voxel[j].y);
                out = _m3d_addidx(out, vd_s, model->voxel[j].z);
                out = _m3d_addidx(out, vd_s, model->voxel[j].w);
                out = _m3d_addidx(out, vd_s, model->voxel[j].h);
                out = _m3d_addidx(out, vd_s, model->voxel[j].d);
                *out++ = model->voxel[j].uncertain;
                *out++ = model->voxel[j].groupid;
                /* RLE compress voxel data */
                n = model->voxel[j].w * model->voxel[j].h * model->voxel[j].d;
                k = o = 0; out[o++] = 0;
                for(i = 0; i < n; i++) {
                    for(l = 1; l < 128 && i + l < n && model->voxel[j].data[i] == model->voxel[j].data[i + l]; l++);
                    if(l > 1) {
                        l--;
                        if(out[k]) { out[k]--; out[o++] = 0x80 | l; }
                        else out[k] = 0x80 | l;
                        switch(vp_s) {
                            case 1: out[o++] = model->voxel[j].data[i]; break;
                            default: *((uint16_t*)(out + o)) = model->voxel[j].data[i]; o += 2; break;
                        }
                        k = o; out[o++] = 0;
                        i += l;
                        continue;
                    }
                    out[k]++;
                    switch(vp_s) {
                        case 1: out[o++] = model->voxel[j].data[i]; break;
                        default: *((uint16_t*)(out + o)) = model->voxel[j].data[i]; o += 2; break;
                    }
                    if(out[k] > 127) { out[k]--; k = o; out[o++] = 0; }
                }
                if(!(out[k] & 0x80)) { if(out[k]) out[k]--; else o--; }
                *length = (uint32_t)((uintptr_t)out + (uintptr_t)o - (uintptr_t)((uint8_t*)h + len));
                len += *length;
                out = NULL;
            }
        }
        /* mathematical shapes face */
        if(model->numshape && model->shape && !(flags & M3D_EXP_NOFACE)) {
            for(j = 0; j < model->numshape; j++) {
                chunklen = 12 + si_s + model->shape[j].numcmd * (M3D_CMDMAXARG + 1) * 4;
                h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                if(!h) goto memerr;
                memcpy((uint8_t*)h + len, "SHPE", 4);
                length = (uint32_t*)((uint8_t*)h + len + 4);
                out = (uint8_t*)h + len + 8;
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->shape[j].name));
                out = _m3d_addidx(out, bi_s, model->shape[j].group);
                for(i = 0; i < model->shape[j].numcmd; i++) {
                    cmd = &model->shape[j].cmd[i];
                    if(cmd->type >= (unsigned int)(sizeof(m3d_commandtypes)/sizeof(m3d_commandtypes[0])) || !cmd->arg)
                        continue;
                    cd = &m3d_commandtypes[cmd->type];
                    *out++ = (cmd->type & 0x7F) | (cmd->type > 127 ? 0x80 : 0);
                    if(cmd->type > 127) *out++ = (cmd->type >> 7) & 0xff;
                    for(k = n = 0, l = cd->p; k < l; k++) {
                        switch(cd->a[((k - n) % (cd->p - n)) + n]) {
                            case m3dcp_mi_t:
                                out = _m3d_addidx(out, si_s, cmd->arg[k] < model->nummaterial ?
                                    _m3d_stridx(str, numstr, model->material[cmd->arg[k]].name) : 0);
                            break;
                            case m3dcp_vc_t:
                                min_x = *((float*)&cmd->arg[k]);
                                switch(vc_s) {
                                    case 1: *out++ = (int8_t)(min_x * 127); break;
                                    case 2: *((int16_t*)out) = (int16_t)(min_x * 32767); out += 2; break;
                                    case 4: *((float*)out) = min_x; out += 4; break;
                                    case 8: *((double*)out) = min_x; out += 8; break;
                                }
                            break;
                            case m3dcp_hi_t: out = _m3d_addidx(out, hi_s, cmd->arg[k]); break;
                            case m3dcp_fi_t: out = _m3d_addidx(out, fi_s, cmd->arg[k]); break;
                            case m3dcp_ti_t: out = _m3d_addidx(out, ti_s, cmd->arg[k]); break;
                            case m3dcp_qi_t:
                            case m3dcp_vi_t: out = _m3d_addidx(out, vi_s, cmd->arg[k]); break;
                            case m3dcp_i1_t: out = _m3d_addidx(out, 1, cmd->arg[k]); break;
                            case m3dcp_i2_t: out = _m3d_addidx(out, 2, cmd->arg[k]); break;
                            case m3dcp_i4_t: out = _m3d_addidx(out, 4, cmd->arg[k]); break;
                            case m3dcp_va_t: out = _m3d_addidx(out, 4, cmd->arg[k]);
                                n = k + 1; l += (cmd->arg[k] - 1) * (cd->p - k - 1);
                            break;
                        }
                    }
                }
                *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
                len += *length;
                out = NULL;
            }
        }
        /* annotation labels */
        if(model->numlabel && model->label) {
            for(i = 0, length = NULL; i < model->numlabel; i++) {
                if(!i || _m3d_strcmp(sl, model->label[i].lang) || _m3d_strcmp(sn, model->label[i].name)) {
                    sl = model->label[i].lang;
                    sn = model->label[i].name;
                    if(length) {
                        *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
                        len += *length;
                    }
                    chunklen = 8 + 2 * si_s + ci_s + model->numlabel * (vi_s + si_s);
                    h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                    if(!h) { sn = NULL; sl = NULL; goto memerr; }
                    memcpy((uint8_t*)h + len, "LBLS", 4);
                    length = (uint32_t*)((uint8_t*)h + len + 4);
                    out = (uint8_t*)h + len + 8;
                    out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->label[l].name));
                    out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->label[l].lang));
                    idx = _m3d_cmapidx(cmap, numcmap, model->label[i].color);
                    switch(ci_s) {
                        case 1: *out++ = (uint8_t)(idx); break;
                        case 2: *((uint16_t*)out) = (uint16_t)(idx); out += 2; break;
                        case 4: *((uint32_t*)out) = model->label[i].color; out += 4; break;
                    }
                }
                out = _m3d_addidx(out, vi_s, vrtxidx[model->label[i].vertexid]);
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->label[l].text));
            }
            if(length) {
                *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
                len += *length;
            }
            out = NULL;
            sn = sl = NULL;
        }
        /* actions */
        if(model->numaction && model->action && model->numbone && model->bone && !(flags & M3D_EXP_NOACTION)) {
            for(j = 0; j < model->numaction; j++) {
                a = &model->action[j];
                chunklen = 14 + si_s + a->numframe * (4 + fc_s + maxt * (bi_s + 2 * vi_s));
                h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                if(!h) goto memerr;
                memcpy((uint8_t*)h + len, "ACTN", 4);
                length = (uint32_t*)((uint8_t*)h + len + 4);
                out = (uint8_t*)h + len + 8;
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, a->name));
                *((uint16_t*)out) = (uint16_t)(a->numframe); out += 2;
                *((uint32_t*)out) = (uint32_t)(a->durationmsec); out += 4;
                for(i = 0; i < a->numframe; i++) {
                    *((uint32_t*)out) = (uint32_t)(a->frame[i].msec); out += 4;
                    out = _m3d_addidx(out, fc_s, a->frame[i].numtransform);
                    for(k = 0; k < a->frame[i].numtransform; k++) {
                        out = _m3d_addidx(out, bi_s, a->frame[i].transform[k].boneid);
                        out = _m3d_addidx(out, vi_s, vrtxidx[a->frame[i].transform[k].pos]);
                        out = _m3d_addidx(out, vi_s, vrtxidx[a->frame[i].transform[k].ori]);
                    }
                }
                *length = (uint32_t)((uintptr_t)out - (uintptr_t)((uint8_t*)h + len));
                len += *length;
                out = NULL;
            }
        }
        /* inlined assets */
        if(model->numinlined && model->inlined && (numproc || (flags & M3D_EXP_INLINE))) {
            for(j = 0; j < model->numinlined; j++) {
                if(!model->inlined[j].name || !model->inlined[j].name[0] || model->inlined[j].length<4 || !model->inlined[j].data)
                    continue;
                if(!(flags & M3D_EXP_INLINE)) {
                    if(model->inlined[j].data[1] == 'P' && model->inlined[j].data[2] == 'N' && model->inlined[j].data[3] == 'G')
                        continue;
                    for(i = k = 0; i < model->numtexture; i++) {
                        if(!strcmp(model->inlined[j].name, model->texture[i].name)) { k = 1; break; }
                    }
                    if(k) continue;
                }
                chunklen = 8 + si_s + model->inlined[j].length;
                h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                if(!h) goto memerr;
                memcpy((uint8_t*)h + len, "ASET", 4);
                *((uint32_t*)((uint8_t*)h + len + 4)) = chunklen;
                out = (uint8_t*)h + len + 8;
                out = _m3d_addidx(out, si_s, _m3d_stridx(str, numstr, model->inlined[j].name));
                memcpy(out, model->inlined[j].data, model->inlined[j].length);
                out = NULL;
                len += chunklen;
            }
        }
        /* extra chunks */
        if(model->numextra && model->extra && (flags & M3D_EXP_EXTRA)) {
            for(j = 0; j < model->numextra; j++) {
                if(!model->extra[j] || model->extra[j]->length < 8)
                    continue;
                chunklen = model->extra[j]->length;
                h = (m3dhdr_t*)M3D_REALLOC(h, len + chunklen);
                if(!h) goto memerr;
                memcpy((uint8_t*)h + len, model->extra[j], chunklen);
                len += chunklen;
            }
        }
        /* add end chunk */
        h = (m3dhdr_t*)M3D_REALLOC(h, len + 4);
        if(!h) goto memerr;
        memcpy((uint8_t*)h + len, "OMD3", 4);
        len += 4;
        /* zlib compress */
        if(!(flags & M3D_EXP_NOZLIB)) {
            M3D_LOG("Deflating chunks");
            z = stbi_zlib_compress((unsigned char *)h, len, (int*)&l, 9);
            if(z && l > 0 && l < len) { len = l; M3D_FREE(h); h = (m3dhdr_t*)z; }
        }
        /* add file header at the begining */
        len += 8;
        out = (unsigned char*)M3D_MALLOC(len);
        if(!out) goto memerr;
        memcpy(out, "3DMO", 4);
        *((uint32_t*)(out + 4)) = len;
        /* preview image chunk, must be the first if exists */
        if(model->preview.data && model->preview.length) {
            chunklen = 8 + model->preview.length;
            out = (unsigned char*)M3D_REALLOC(out, len + chunklen);
            if(!out) goto memerr;
            memcpy((uint8_t*)out + 8, "PRVW", 4);
            *((uint32_t*)((uint8_t*)out + 8 + 4)) = chunklen;
            memcpy((uint8_t*)out + 8 + 8, model->preview.data, model->preview.length);
            *((uint32_t*)(out + 4)) += chunklen;
        } else
            chunklen = 0;
        memcpy(out + 8 + chunklen, h, len - 8);
    }
    if(size) *size = out ? len : 0;
    if(vrtxidx) M3D_FREE(vrtxidx);
    if(mtrlidx) M3D_FREE(mtrlidx);
    if(tmapidx) M3D_FREE(tmapidx);
    if(skinidx) M3D_FREE(skinidx);
    if(norm) M3D_FREE(norm);
    if(face) M3D_FREE(face);
    if(cmap) M3D_FREE(cmap);
    if(tmap) M3D_FREE(tmap);
    if(skin) M3D_FREE(skin);
    if(str) M3D_FREE(str);
    if(vrtx) M3D_FREE(vrtx);
    if(h) M3D_FREE(h);
    return out;
}
#endif

#endif

#ifdef  __cplusplus
}
#ifdef M3D_CPPWRAPPER
#include <vector>
#include <string>
#include <memory>

/*** C++ wrapper class ***/
namespace M3D {
#ifdef M3D_IMPLEMENTATION

    class Model {
        public:
            m3d_t *model;

        public:
            Model() {
                this->model = (m3d_t*)malloc(sizeof(m3d_t)); memset(this->model, 0, sizeof(m3d_t));
            }
            Model(_unused const std::string &data, _unused m3dread_t ReadFileCB,
                _unused m3dfree_t FreeCB, _unused M3D::Model mtllib) {
#ifndef M3D_NOIMPORTER
                this->model = m3d_load((unsigned char *)data.data(), ReadFileCB, FreeCB, mtllib.model);
#else
                Model();
#endif
            }
            Model(_unused const std::vector<unsigned char> data, _unused m3dread_t ReadFileCB,
                _unused m3dfree_t FreeCB, _unused M3D::Model mtllib) {
#ifndef M3D_NOIMPORTER
                this->model = m3d_load((unsigned char *)&data[0], ReadFileCB, FreeCB, mtllib.model);
#else
                Model();
#endif
            }
            Model(_unused const unsigned char *data, _unused m3dread_t ReadFileCB,
                _unused m3dfree_t FreeCB, _unused M3D::Model mtllib) {
#ifndef M3D_NOIMPORTER
                this->model = m3d_load((unsigned char*)data, ReadFileCB, FreeCB, mtllib.model);
#else
                Model();
#endif
            }
            ~Model() { m3d_free(this->model); }

        public:
            m3d_t *getCStruct() { return this->model; }
            std::string getName() { return std::string(this->model->name); }
            void setName(std::string name) { this->model->name = (char*)name.c_str(); }
            std::string getLicense() { return std::string(this->model->license); }
            void setLicense(std::string license) { this->model->license = (char*)license.c_str(); }
            std::string getAuthor() { return std::string(this->model->author); }
            void setAuthor(std::string author) { this->model->author = (char*)author.c_str(); }
            std::string getDescription() { return std::string(this->model->desc); }
            void setDescription(std::string desc) { this->model->desc = (char*)desc.c_str(); }
            float getScale() { return this->model->scale; }
            void setScale(float scale) { this->model->scale = scale; }
            std::vector<unsigned char> getPreview() { return this->model->preview.data ?
                std::vector<unsigned char>(this->model->preview.data, this->model->preview.data + this->model->preview.length) :
                std::vector<unsigned char>(); }
            std::vector<uint32_t> getColorMap() { return this->model->cmap ? std::vector<uint32_t>(this->model->cmap,
                this->model->cmap + this->model->numcmap) : std::vector<uint32_t>(); }
            std::vector<m3dti_t> getTextureMap() { return this->model->tmap ? std::vector<m3dti_t>(this->model->tmap,
                this->model->tmap + this->model->numtmap) : std::vector<m3dti_t>(); }
            std::vector<m3dtx_t> getTextures() { return this->model->texture ? std::vector<m3dtx_t>(this->model->texture,
                this->model->texture + this->model->numtexture) : std::vector<m3dtx_t>(); }
            std::string getTextureName(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numtexture ?
                std::string(this->model->texture[idx].name) : nullptr; }
            std::vector<m3db_t> getBones() { return this->model->bone ? std::vector<m3db_t>(this->model->bone, this->model->bone +
                this->model->numbone) : std::vector<m3db_t>(); }
            std::string getBoneName(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numbone ?
                std::string(this->model->bone[idx].name) : nullptr; }
            std::vector<m3dm_t> getMaterials() { return this->model->material ? std::vector<m3dm_t>(this->model->material,
                this->model->material + this->model->nummaterial) : std::vector<m3dm_t>(); }
            std::string getMaterialName(int idx) { return idx >= 0 && (unsigned int)idx < this->model->nummaterial ?
                std::string(this->model->material[idx].name) : nullptr; }
            int getMaterialPropertyInt(int idx, int type) {
                    if (idx < 0 || (unsigned int)idx >= this->model->nummaterial || type < 0 || type >= 127 ||
                        !this->model->material[idx].prop) return -1;
                    for (int i = 0; i < this->model->material[idx].numprop; i++) {
                        if (this->model->material[idx].prop[i].type == type)
                            return this->model->material[idx].prop[i].value.num;
                    }
                    return -1;
                }
            uint32_t getMaterialPropertyColor(int idx, int type) { return this->getMaterialPropertyInt(idx, type); }
            float getMaterialPropertyFloat(int idx, int type) {
                    if (idx < 0 || (unsigned int)idx >= this->model->nummaterial || type < 0 || type >= 127 ||
                        !this->model->material[idx].prop) return -1.0f;
                    for (int i = 0; i < this->model->material[idx].numprop; i++) {
                        if (this->model->material[idx].prop[i].type == type)
                            return this->model->material[idx].prop[i].value.fnum;
                    }
                    return -1.0f;
                }
            m3dtx_t* getMaterialPropertyMap(int idx, int type) {
                    if (idx < 0 || (unsigned int)idx >= this->model->nummaterial || type < 128 || type > 255 ||
                        !this->model->material[idx].prop) return nullptr;
                    for (int i = 0; i < this->model->material[idx].numprop; i++) {
                        if (this->model->material[idx].prop[i].type == type)
                            return this->model->material[idx].prop[i].value.textureid < this->model->numtexture ?
                                &this->model->texture[this->model->material[idx].prop[i].value.textureid] : nullptr;
                    }
                    return nullptr;
                }
            std::vector<m3dv_t> getVertices() { return this->model->vertex ? std::vector<m3dv_t>(this->model->vertex,
                this->model->vertex + this->model->numvertex) : std::vector<m3dv_t>(); }
            std::vector<m3df_t> getFace() { return this->model->face ? std::vector<m3df_t>(this->model->face, this->model->face +
                this->model->numface) : std::vector<m3df_t>(); }
            std::vector<m3dvt_t> getVoxelTypes() { return this->model->voxtype ? std::vector<m3dvt_t>(this->model->voxtype,
                this->model->voxtype + this->model->numvoxtype) : std::vector<m3dvt_t>(); }
            std::string getVoxelTypeName(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numvoxtype &&
                this->model->voxtype[idx].name && this->model->voxtype[idx].name[0] ?
                std::string(this->model->voxtype[idx].name) : nullptr; }
            std::vector<m3dvi_t> getVoxelTypeItems(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numvoxtype &&
                this->model->voxtype[idx].item ? std::vector<m3dvi_t>(this->model->voxtype[idx].item,
                this->model->voxtype[idx].item + this->model->voxtype[idx].numitem) : std::vector<m3dvi_t>(); }
            std::vector<m3dvx_t> getVoxelBlocks() { return this->model->voxel ? std::vector<m3dvx_t>(this->model->voxel,
                this->model->voxel + this->model->numvoxel) : std::vector<m3dvx_t>(); }
            std::string getVoxelBlockName(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numvoxel &&
                this->model->voxel[idx].name && this->model->voxel[idx].name[0] ?
                std::string(this->model->voxel[idx].name) : nullptr; }
            std::vector<M3D_VOXEL> getVoxelBlockData(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numvoxel &&
                this->model->voxel[idx].data ? std::vector<M3D_VOXEL>(this->model->voxel[idx].data,
                this->model->voxel[idx].data + this->model->voxel[idx].w*this->model->voxel[idx].h*this->model->voxel[idx].d) :
                std::vector<M3D_VOXEL>(); }
            std::vector<m3dh_t> getShape() { return this->model->shape ? std::vector<m3dh_t>(this->model->shape,
                this->model->shape + this->model->numshape) : std::vector<m3dh_t>(); }
            std::string getShapeName(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numshape &&
                this->model->shape[idx].name && this->model->shape[idx].name[0] ?
                std::string(this->model->shape[idx].name) : nullptr; }
            unsigned int getShapeGroup(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numshape ?
                this->model->shape[idx].group : 0xFFFFFFFF; }
            std::vector<m3dc_t> getShapeCommands(int idx) { return idx >= 0 && (unsigned int)idx < this->model->numshape &&
                this->model->shape[idx].cmd ? std::vector<m3dc_t>(this->model->shape[idx].cmd, this->model->shape[idx].cmd +
                this->model->shape[idx].numcmd) : std::vector<m3dc_t>(); }
            std::vector<m3dl_t> getAnnotationLabels() { return this->model->label ? std::vector<m3dl_t>(this->model->label,
                this->model->label + this->model->numlabel) : std::vector<m3dl_t>(); }
            std::vector<m3ds_t> getSkin() { return this->model->skin ? std::vector<m3ds_t>(this->model->skin, this->model->skin +
                this->model->numskin) : std::vector<m3ds_t>(); }
            std::vector<m3da_t> getActions() { return this->model->action ? std::vector<m3da_t>(this->model->action,
                this->model->action + this->model->numaction) : std::vector<m3da_t>(); }
            std::string getActionName(int aidx) { return aidx >= 0 && (unsigned int)aidx < this->model->numaction ?
                std::string(this->model->action[aidx].name) : nullptr; }
            unsigned int getActionDuration(int aidx) { return aidx >= 0 && (unsigned int)aidx < this->model->numaction ?
                this->model->action[aidx].durationmsec : 0; }
            std::vector<m3dfr_t> getActionFrames(int aidx) { return aidx >= 0 && (unsigned int)aidx < this->model->numaction ?
                std::vector<m3dfr_t>(this->model->action[aidx].frame, this->model->action[aidx].frame +
                this->model->action[aidx].numframe) : std::vector<m3dfr_t>(); }
            unsigned int getActionFrameTimestamp(int aidx, int fidx) { return aidx >= 0 && (unsigned int)aidx < this->model->numaction?
                    (fidx >= 0 && (unsigned int)fidx < this->model->action[aidx].numframe ?
                    this->model->action[aidx].frame[fidx].msec : 0) : 0; }
            std::vector<m3dtr_t> getActionFrameTransforms(int aidx, int fidx) {
                return aidx >= 0 && (unsigned int)aidx < this->model->numaction ? (
                    fidx >= 0 && (unsigned int)fidx < this->model->action[aidx].numframe ?
                    std::vector<m3dtr_t>(this->model->action[aidx].frame[fidx].transform,
                    this->model->action[aidx].frame[fidx].transform + this->model->action[aidx].frame[fidx].numtransform) :
                    std::vector<m3dtr_t>()) : std::vector<m3dtr_t>(); }
            std::vector<m3dtr_t> getActionFrame(int aidx, int fidx, std::vector<m3dtr_t> skeleton) {
                m3dtr_t *pose = m3d_frame(this->model, (unsigned int)aidx, (unsigned int)fidx,
                    skeleton.size() ? &skeleton[0] : nullptr);
                return std::vector<m3dtr_t>(pose, pose + this->model->numbone); }
            std::vector<m3db_t> getActionPose(int aidx, unsigned int msec) {
                m3db_t *pose = m3d_pose(this->model, (unsigned int)aidx, (unsigned int)msec);
                return std::vector<m3db_t>(pose, pose + this->model->numbone); }
            std::vector<m3di_t> getInlinedAssets() { return this->model->inlined ? std::vector<m3di_t>(this->model->inlined,
                this->model->inlined + this->model->numinlined) : std::vector<m3di_t>(); }
            std::vector<std::unique_ptr<m3dchunk_t>> getExtras() { return this->model->extra ?
                std::vector<std::unique_ptr<m3dchunk_t>>(this->model->extra,
                this->model->extra + this->model->numextra) : std::vector<std::unique_ptr<m3dchunk_t>>(); }
            std::vector<unsigned char> Save(_unused int quality, _unused int flags) {
#ifdef M3D_EXPORTER
                unsigned int size;
                unsigned char *ptr = m3d_save(this->model, quality, flags, &size);
                return ptr && size ? std::vector<unsigned char>(ptr, ptr + size) : std::vector<unsigned char>();
#else
                return std::vector<unsigned char>();
#endif
            }
    };

#else
    class Model {
        private:
            m3d_t *model;

        public:
            Model(const std::string &data, m3dread_t ReadFileCB, m3dfree_t FreeCB);
            Model(const std::vector<unsigned char> data, m3dread_t ReadFileCB, m3dfree_t FreeCB);
            Model(const unsigned char *data, m3dread_t ReadFileCB, m3dfree_t FreeCB);
            Model();
            ~Model();

        public:
            m3d_t *getCStruct();
            std::string getName();
            void setName(std::string name);
            std::string getLicense();
            void setLicense(std::string license);
            std::string getAuthor();
            void setAuthor(std::string author);
            std::string getDescription();
            void setDescription(std::string desc);
            float getScale();
            void setScale(float scale);
            std::vector<unsigned char> getPreview();
            std::vector<uint32_t> getColorMap();
            std::vector<m3dti_t> getTextureMap();
            std::vector<m3dtx_t> getTextures();
            std::string getTextureName(int idx);
            std::vector<m3db_t> getBones();
            std::string getBoneName(int idx);
            std::vector<m3dm_t> getMaterials();
            std::string getMaterialName(int idx);
            int getMaterialPropertyInt(int idx, int type);
            uint32_t getMaterialPropertyColor(int idx, int type);
            float getMaterialPropertyFloat(int idx, int type);
            m3dtx_t* getMaterialPropertyMap(int idx, int type);
            std::vector<m3dv_t> getVertices();
            std::vector<m3df_t> getFace();
            std::vector<m3dvt_t> getVoxelTypes();
            std::string getVoxelTypeName(int idx);
            std::vector<m3dvi_t> getVoxelTypeItems(int idx);
            std::vector<m3dvx_t> getVoxelBlocks();
            std::string getVoxelBlockName(int idx);
            std::vector<M3D_VOXEL> getVoxelBlockData(int idx);
            std::vector<m3dh_t> getShape();
            std::string getShapeName(int idx);
            unsigned int getShapeGroup(int idx);
            std::vector<m3dc_t> getShapeCommands(int idx);
            std::vector<m3dl_t> getAnnotationLabels();
            std::vector<m3ds_t> getSkin();
            std::vector<m3da_t> getActions();
            std::string getActionName(int aidx);
            unsigned int getActionDuration(int aidx);
            std::vector<m3dfr_t> getActionFrames(int aidx);
            unsigned int getActionFrameTimestamp(int aidx, int fidx);
            std::vector<m3dtr_t> getActionFrameTransforms(int aidx, int fidx);
            std::vector<m3dtr_t> getActionFrame(int aidx, int fidx, std::vector<m3dtr_t> skeleton);
            std::vector<m3db_t> getActionPose(int aidx, unsigned int msec);
            std::vector<m3di_t> getInlinedAssets();
            std::vector<std::unique_ptr<m3dchunk_t>> getExtras();
            std::vector<unsigned char> Save(int quality, int flags);
    };

#endif /* impl */
}
#endif

#endif /* __cplusplus */

#endif
