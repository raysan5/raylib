/**********************************************************************************************
*
*   rl_gputex - GPU compressed textures loading and saving
*
*   DESCRIPTION:
*
*     Load GPU compressed image data from image files provided as memory data arrays,
*     data is loaded compressed, ready to be loaded into GPU.
*
*     Note that some file formats (DDS, PVR, KTX) also support uncompressed data storage.
*     In those cases data is loaded uncompressed and format is returned.
*
*   TODO:
*     - Implement raylib function: rlGetGlTextureFormats(), required by rl_save_ktx_to_memory()
*     - Review rl_load_ktx_from_memory() to support KTX v2.2 specs
*
*   CONFIGURATION:
*
*   #define RL_GPUTEX_SUPPORT_DDS
*   #define RL_GPUTEX_SUPPORT_PKM
*   #define RL_GPUTEX_SUPPORT_KTX
*   #define RL_GPUTEX_SUPPORT_PVR
*   #define RL_GPUTEX_SUPPORT_ASTC
*       Define desired file formats to be supported
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RL_GPUTEX_H
#define RL_GPUTEX_H

#ifndef RLAPI
    #define RLAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

// Load image data from memory data files
RLAPI void *rl_load_dds_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips);
RLAPI void *rl_load_pkm_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips);
RLAPI void *rl_load_ktx_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips);
RLAPI void *rl_load_pvr_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips);
RLAPI void *rl_load_astc_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips);

RLAPI int rl_save_ktx_to_memory(const char *fileName, void *data, int width, int height, int format, int mipmaps);  // Save image data as KTX file

#if defined(__cplusplus)
}
#endif

#endif // RL_GPUTEX_H


/***********************************************************************************
*
*   RL_GPUTEX IMPLEMENTATION
*
************************************************************************************/

#if defined(RL_GPUTEX_IMPLEMENTATION)

// Simple log system to avoid RPNG_LOG() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define RL_GPUTEX_SHOW_LOG_INFO
#if defined(RL_GPUTEX_SHOW_LOG_INFO) && !defined(LOG)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
// Get pixel data size in bytes for certain pixel format
static int get_pixel_data_size(int width, int height, int format);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
#if defined(RL_GPUTEX_SUPPORT_DDS)
// Loading DDS from memory image data (compressed or uncompressed)
void *rl_load_dds_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips)
{
    void *image_data = NULL;        // Image data pointer
    int image_pixel_size = 0;       // Image pixel size

    unsigned char *file_data_ptr = (unsigned char *)file_data;

    // Required extension:
    // GL_EXT_texture_compression_s3tc

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_S3TC_DXT1_EXT      0x83F0
    // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT     0x83F1
    // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT     0x83F2
    // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT     0x83F3

    #define FOURCC_DXT1 0x31545844  // Equivalent to "DXT1" in ASCII
    #define FOURCC_DXT3 0x33545844  // Equivalent to "DXT3" in ASCII
    #define FOURCC_DXT5 0x35545844  // Equivalent to "DXT5" in ASCII

    // DDS Pixel Format
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int fourcc;
        unsigned int rgb_bit_count;
        unsigned int r_bit_mask;
        unsigned int g_bit_mask;
        unsigned int b_bit_mask;
        unsigned int a_bit_mask;
    } dds_pixel_format;

    // DDS Header (124 bytes)
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int height;
        unsigned int width;
        unsigned int pitch_or_linear_size;
        unsigned int depth;
        unsigned int mipmap_count;
        unsigned int reserved1[11];
        dds_pixel_format ddspf;
        unsigned int caps;
        unsigned int caps2;
        unsigned int caps3;
        unsigned int caps4;
        unsigned int reserved2;
    } dds_header;

    if (file_data_ptr != NULL)
    {
        // Verify the type of file
        unsigned char *dds_header_id = file_data_ptr;
        file_data_ptr += 4;

        if ((dds_header_id[0] != 'D') || (dds_header_id[1] != 'D') || (dds_header_id[2] != 'S') || (dds_header_id[3] != ' '))
        {
            LOG("WARNING: IMAGE: DDS file data not valid");
        }
        else
        {
            dds_header *header = (dds_header *)file_data_ptr;

            file_data_ptr += sizeof(dds_header);        // Skip header

            *width = header->width;
            *height = header->height;
            image_pixel_size = header->width*header->height;

            if (header->mipmap_count == 0) *mips = 1;   // Parameter not used
            else *mips = header->mipmap_count;

            if (header->ddspf.rgb_bit_count == 16)      // 16bit mode, no compressed
            {
                if (header->ddspf.flags == 0x40)        // No alpha channel
                {
                    int data_size = image_pixel_size*sizeof(unsigned short);
                    image_data = RL_MALLOC(data_size);

                    memcpy(image_data, file_data_ptr, data_size);

                    *format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
                }
                else if (header->ddspf.flags == 0x41)           // With alpha channel
                {
                    if (header->ddspf.a_bit_mask == 0x8000)     // 1bit alpha
                    {
                        int data_size = image_pixel_size*sizeof(unsigned short);
                        image_data = RL_MALLOC(data_size);

                        memcpy(image_data, file_data_ptr, data_size);

                        unsigned char alpha = 0;

                        // NOTE: Data comes as A1R5G5B5, it must be reordered to R5G5B5A1
                        for (int i = 0; i < image_pixel_size; i++)
                        {
                            alpha = ((unsigned short *)image_data)[i] >> 15;
                            ((unsigned short *)image_data)[i] = ((unsigned short *)image_data)[i] << 1;
                            ((unsigned short *)image_data)[i] += alpha;
                        }

                        *format = PIXELFORMAT_UNCOMPRESSED_R5G5B5A1;
                    }
                    else if (header->ddspf.a_bit_mask == 0xf000)   // 4bit alpha
                    {
                        int data_size = image_pixel_size*sizeof(unsigned short);
                        image_data = RL_MALLOC(data_size);

                        memcpy(image_data, file_data_ptr, data_size);

                        unsigned char alpha = 0;

                        // NOTE: Data comes as A4R4G4B4, it must be reordered R4G4B4A4
                        for (int i = 0; i < image_pixel_size; i++)
                        {
                            alpha = ((unsigned short *)image_data)[i] >> 12;
                            ((unsigned short *)image_data)[i] = ((unsigned short *)image_data)[i] << 4;
                            ((unsigned short *)image_data)[i] += alpha;
                        }

                        *format = PIXELFORMAT_UNCOMPRESSED_R4G4B4A4;
                    }
                }
            }
            else if (header->ddspf.flags == 0x40 && header->ddspf.rgb_bit_count == 24)   // DDS_RGB, no compressed
            {
                int data_size = image_pixel_size*3*sizeof(unsigned char);
                image_data = RL_MALLOC(data_size);

                memcpy(image_data, file_data_ptr, data_size);

                *format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
            }
            else if (header->ddspf.flags == 0x41 && header->ddspf.rgb_bit_count == 32) // DDS_RGBA, no compressed
            {
                int data_size = image_pixel_size*4*sizeof(unsigned char);
                image_data = RL_MALLOC(data_size);

                memcpy(image_data, file_data_ptr, data_size);

                unsigned char blue = 0;

                // NOTE: Data comes as A8R8G8B8, it must be reordered R8G8B8A8 (view next comment)
                // DirecX understand ARGB as a 32bit DWORD but the actual memory byte alignment is BGRA
                // So, we must realign B8G8R8A8 to R8G8B8A8
                for (int i = 0; i < image_pixel_size*4; i += 4)
                {
                    blue = ((unsigned char *)image_data)[i];
                    ((unsigned char *)image_data)[i] = ((unsigned char *)image_data)[i + 2];
                    ((unsigned char *)image_data)[i + 2] = blue;
                }

                *format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            }
            else if (((header->ddspf.flags == 0x04) || (header->ddspf.flags == 0x05)) && (header->ddspf.fourcc > 0)) // Compressed
            {
                int data_size = 0;

                // Calculate data size, including all mipmaps
                if (header->mipmap_count > 1) data_size = header->pitch_or_linear_size*2;
                else data_size = header->pitch_or_linear_size;

                image_data = RL_MALLOC(data_size*sizeof(unsigned char));

                memcpy(image_data, file_data_ptr, data_size);

                switch (header->ddspf.fourcc)
                {
                    case FOURCC_DXT1:
                    {
                        if (header->ddspf.flags == 0x04) *format = PIXELFORMAT_COMPRESSED_DXT1_RGB;
                        else *format = PIXELFORMAT_COMPRESSED_DXT1_RGBA;
                    } break;
                    case FOURCC_DXT3: *format = PIXELFORMAT_COMPRESSED_DXT3_RGBA; break;
                    case FOURCC_DXT5: *format = PIXELFORMAT_COMPRESSED_DXT5_RGBA; break;
                    default: break;
                }
            }
        }
    }

    return image_data;
}
#endif

#if defined(RL_GPUTEX_SUPPORT_PKM)
// Loading PKM image data (ETC1/ETC2 compression)
// NOTE: KTX is the standard Khronos Group compression format (ETC1/ETC2, mipmaps)
// PKM is a much simpler file format used mainly to contain a single ETC1/ETC2 compressed image (no mipmaps)
void *rl_load_pkm_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips)
{
    void *image_data = NULL;        // Image data pointer

    unsigned char *file_data_ptr = (unsigned char *)file_data;

    // Required extensions:
    // GL_OES_compressed_ETC1_RGB8_texture  (ETC1) (OpenGL ES 2.0)
    // GL_ARB_ES3_compatibility  (ETC2/EAC) (OpenGL ES 3.0)

    // Supported tokens (defined by extensions)
    // GL_ETC1_RGB8_OES                 0x8D64
    // GL_COMPRESSED_RGB8_ETC2          0x9274
    // GL_COMPRESSED_RGBA8_ETC2_EAC     0x9278

    // PKM file (ETC1) Header (16 bytes)
    typedef struct {
        char id[4];                 // "PKM "
        char version[2];            // "10" or "20"
        unsigned short format;      // Data format (big-endian) (Check list below)
        unsigned short width;       // Texture width (big-endian) (orig_width rounded to multiple of 4)
        unsigned short height;      // Texture height (big-endian) (orig_height rounded to multiple of 4)
        unsigned short orig_width;   // Original width (big-endian)
        unsigned short orig_height;  // Original height (big-endian)
    } pkm_header;

    // Formats list
    // version 10: format: 0=ETC1_RGB, [1=ETC1_RGBA, 2=ETC1_RGB_MIP, 3=ETC1_RGBA_MIP] (not used)
    // version 20: format: 0=ETC1_RGB, 1=ETC2_RGB, 2=ETC2_RGBA_OLD, 3=ETC2_RGBA, 4=ETC2_RGBA1, 5=ETC2_R, 6=ETC2_RG, 7=ETC2_SIGNED_R, 8=ETC2_SIGNED_R

    // NOTE: The extended width and height are the widths rounded up to a multiple of 4.
    // NOTE: ETC is always 4bit per pixel (64 bit for each 4x4 block of pixels)

    if (file_data_ptr != NULL)
    {
        pkm_header *header = (pkm_header *)file_data_ptr;

        if ((header->id[0] != 'P') || (header->id[1] != 'K') || (header->id[2] != 'M') || (header->id[3] != ' '))
        {
            LOG("WARNING: IMAGE: PKM file data not valid");
        }
        else
        {
            file_data_ptr += sizeof(pkm_header);   // Skip header

            // NOTE: format, width and height come as big-endian, data must be swapped to little-endian
            header->format = ((header->format & 0x00FF) << 8) | ((header->format & 0xFF00) >> 8);
            header->width = ((header->width & 0x00FF) << 8) | ((header->width & 0xFF00) >> 8);
            header->height = ((header->height & 0x00FF) << 8) | ((header->height & 0xFF00) >> 8);

            *width = header->width;
            *height = header->height;
            *mips = 1;

            int bpp = 4;
            if (header->format == 3) bpp = 8;

            int data_size = (*width)*(*height)*bpp/8;  // Total data size in bytes

            image_data = RL_MALLOC(data_size*sizeof(unsigned char));

            memcpy(image_data, file_data_ptr, data_size);

            if (header->format == 0) *format = PIXELFORMAT_COMPRESSED_ETC1_RGB;
            else if (header->format == 1) *format = PIXELFORMAT_COMPRESSED_ETC2_RGB;
            else if (header->format == 3) *format = PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA;
        }
    }

    return image_data;
}
#endif

#if defined(RL_GPUTEX_SUPPORT_KTX)
// Load KTX compressed image data (ETC1/ETC2 compression)
// TODO: Review KTX loading, many things changed!
void *rl_load_ktx_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips)
{
    void *image_data = NULL;        // Image data pointer

    unsigned char *file_data_ptr = (unsigned char *)file_data;

    // Required extensions:
    // GL_OES_compressed_ETC1_RGB8_texture  (ETC1)
    // GL_ARB_ES3_compatibility  (ETC2/EAC)

    // Supported tokens (defined by extensions)
    // GL_ETC1_RGB8_OES                 0x8D64
    // GL_COMPRESSED_RGB8_ETC2          0x9274
    // GL_COMPRESSED_RGBA8_ETC2_EAC     0x9278

    // KTX file Header (64 bytes)
    // v1.1 - https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
    // v2.0 - http://github.khronos.org/KTX-Specification/

    // KTX 1.1 Header
    // TODO: Support KTX 2.2 specs!
    typedef struct {
        char id[12];                            // Identifier: "«KTX 11»\r\n\x1A\n"
        unsigned int endianness;                // Little endian: 0x01 0x02 0x03 0x04
        unsigned int gl_type;                   // For compressed textures, glType must equal 0
        unsigned int gl_type_size;              // For compressed texture data, usually 1
        unsigned int gl_format;                 // For compressed textures is 0
        unsigned int gl_internal_format;        // Compressed internal format
        unsigned int gl_base_internal_format;   // Same as glFormat (RGB, RGBA, ALPHA...)
        unsigned int width;                     // Texture image width in pixels
        unsigned int height;                    // Texture image height in pixels
        unsigned int depth;                     // For 2D textures is 0
        unsigned int elements;                  // Number of array elements, usually 0
        unsigned int faces;                     // Cubemap faces, for no-cubemap = 1
        unsigned int mipmap_levels;             // Non-mipmapped textures = 1
        unsigned int key_value_data_size;       // Used to encode any arbitrary data...
    } ktx_header;

    // NOTE: Before start of every mipmap data block, we have: unsigned int data_size

    if (file_data_ptr != NULL)
    {
        ktx_header *header = (ktx_header *)file_data_ptr;

        if ((header->id[1] != 'K') || (header->id[2] != 'T') || (header->id[3] != 'X') ||
            (header->id[4] != ' ') || (header->id[5] != '1') || (header->id[6] != '1'))
        {
            LOG("WARNING: IMAGE: KTX file data not valid");
        }
        else
        {
            file_data_ptr += sizeof(ktx_header);           // Move file data pointer

            *width = header->width;
            *height = header->height;
            *mips = header->mipmap_levels;

            file_data_ptr += header->key_value_data_size; // Skip value data size

            int data_size = ((int *)file_data_ptr)[0];
            file_data_ptr += sizeof(int);

            image_data = RL_MALLOC(data_size*sizeof(unsigned char));

            memcpy(image_data, file_data_ptr, data_size);

            if (header->gl_internal_format == 0x8D64) *format = PIXELFORMAT_COMPRESSED_ETC1_RGB;
            else if (header->gl_internal_format == 0x9274) *format = PIXELFORMAT_COMPRESSED_ETC2_RGB;
            else if (header->gl_internal_format == 0x9278) *format = PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA;

            // TODO: Support uncompressed data formats? Right now it returns format = 0!
        }
    }

    return image_data;
}

// Save image data as KTX file
// NOTE: By default KTX 1.1 spec is used, 2.0 is still on draft (01Oct2018)
// TODO: Review KTX saving, many things changed!
int rl_save_ktx(const char *file_name, void *data, int width, int height, int format, int mipmaps)
{
    // KTX file Header (64 bytes)
    // v1.1 - https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
    // v2.0 - http://github.khronos.org/KTX-Specification/ - Final specs by 2021-04-18
    typedef struct {
        char id[12];                            // Identifier: "«KTX 11»\r\n\x1A\n"         // KTX 2.0: "«KTX 22»\r\n\x1A\n"
        unsigned int endianness;                // Little endian: 0x01 0x02 0x03 0x04
        unsigned int gl_type;                   // For compressed textures, glType must equal 0
        unsigned int gl_type_size;              // For compressed texture data, usually 1
        unsigned int gl_format;                 // For compressed textures is 0
        unsigned int gl_internal_format;        // Compressed internal format
        unsigned int gl_base_internal_format;   // Same as glFormat (RGB, RGBA, ALPHA...)   // KTX 2.0: UInt32 vkFormat
        unsigned int width;                     // Texture image width in pixels
        unsigned int height;                    // Texture image height in pixels
        unsigned int depth;                     // For 2D textures is 0
        unsigned int elements;                  // Number of array elements, usually 0
        unsigned int faces;                     // Cubemap faces, for no-cubemap = 1
        unsigned int mipmap_levels;             // Non-mipmapped textures = 1
        unsigned int key_value_data_size;       // Used to encode any arbitrary data...     // KTX 2.0: UInt32 levelOrder - ordering of the mipmap levels, usually 0
                                                                                            // KTX 2.0: UInt32 supercompressionScheme - 0 (None), 1 (Crunch CRN), 2 (Zlib DEFLATE)...
        // KTX 2.0 defines additional header elements...
    } ktx_header;

    // Calculate file data_size required
    int data_size = sizeof(ktx_header);

    for (int i = 0, w = width, h = height; i < mipmaps; i++)
    {
        data_size += get_pixel_data_size(w, h, format);
        w /= 2; h /= 2;
    }

    unsigned char *file_data = RL_CALLOC(data_size, 1);
    unsigned char *file_data_ptr = file_data;

    ktx_header header = { 0 };

    // KTX identifier (v1.1)
    //unsigned char id[12] = { '«', 'K', 'T', 'X', ' ', '1', '1', '»', '\r', '\n', '\x1A', '\n' };
    //unsigned char id[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

    const char ktx_identifier[12] = { 0xAB, 'K', 'T', 'X', ' ', '1', '1', 0xBB, '\r', '\n', 0x1A, '\n' };

    // Get the image header
    memcpy(header.id, ktx_identifier, 12);  // KTX 1.1 signature
    header.endianness = 0;
    header.gl_type = 0;                     // Obtained from format
    header.gl_type_size = 1;
    header.gl_format = 0;                   // Obtained from format
    header.gl_internal_format = 0;          // Obtained from format
    header.gl_base_internal_format = 0;
    header.width = width;
    header.height = height;
    header.depth = 0;
    header.elements = 0;
    header.faces = 1;
    header.mipmap_levels = mipmaps;         // If it was 0, it means mipmaps should be generated on loading (not for compressed formats)
    header.key_value_data_size = 0;         // No extra data after the header

    rlGetGlTextureFormats(format, &header.gl_internal_format, &header.gl_format, &header.gl_type);   // rlgl module function
    header.gl_base_internal_format = header.gl_format;    // KTX 1.1 only

    // NOTE: We can save into a .ktx all PixelFormats supported by raylib, including compressed formats like DXT, ETC or ASTC

    if (header.gl_format == -1) LOG("WARNING: IMAGE: GL format not supported for KTX export (%i)", header.gl_format);
    else
    {
        memcpy(file_data_ptr, &header, sizeof(ktx_header));
        file_data_ptr += sizeof(ktx_header);

        int temp_width = width;
        int temp_height = height;
        int data_offset = 0;

        // Save all mipmaps data
        for (int i = 0; i < mipmaps; i++)
        {
            unsigned int data_size = get_pixel_data_size(temp_width, temp_height, format);

            memcpy(file_data_ptr, &data_size, sizeof(unsigned int));
            memcpy(file_data_ptr + 4, (unsigned char *)data + data_offset, data_size);

            temp_width /= 2;
            temp_height /= 2;
            data_offset += data_size;
            file_data_ptr += (4 + data_size);
        }
    }

    // Save file data to file
    int success = false;
    FILE *file = fopen(file_name, "wb");

    if (file != NULL)
    {
        unsigned int count = (unsigned int)fwrite(file_data, sizeof(unsigned char), data_size, file);

        if (count == 0) LOG("WARNING: FILEIO: [%s] Failed to write file", file_name);
        else if (count != data_size) LOG("WARNING: FILEIO: [%s] File partially written", file_name);
        else LOG("INFO: FILEIO: [%s] File saved successfully", file_name);

        int result = fclose(file);
        if (result == 0) success = true;
    }
    else LOG("WARNING: FILEIO: [%s] Failed to open file", file_name);

    RL_FREE(file_data);    // Free file data buffer

    // If all data has been written correctly to file, success = 1
    return success;
}
#endif

#if defined(RL_GPUTEX_SUPPORT_PVR)
// Loading PVR image data (uncompressed or PVRT compression)
// NOTE: PVR v2 not supported, use PVR v3 instead
void *rl_load_pvr_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips)
{
    void *image_data = NULL;        // Image data pointer

    unsigned char *file_data_ptr = (unsigned char *)file_data;

    // Required extension:
    // GL_IMG_texture_compression_pvrtc

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG       0x8C00
    // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG      0x8C02

#if 0   // Not used...
    // PVR file v2 Header (52 bytes)
    typedef struct {
        unsigned int headerLength;
        unsigned int height;
        unsigned int width;
        unsigned int numMipmaps;
        unsigned int flags;
        unsigned int dataLength;
        unsigned int bpp;
        unsigned int bitmaskRed;
        unsigned int bitmaskGreen;
        unsigned int bitmaskBlue;
        unsigned int bitmaskAlpha;
        unsigned int pvrTag;
        unsigned int numSurfs;
    } PVRHeaderV2;
#endif

    // PVR file v3 Header (52 bytes)
    // NOTE: After it could be metadata (15 bytes?)
    typedef struct {
        char id[4];
        unsigned int flags;
        unsigned char channels[4];      // pixelFormat high part
        unsigned char channel_depth[4];  // pixelFormat low part
        unsigned int color_space;
        unsigned int channel_type;
        unsigned int height;
        unsigned int width;
        unsigned int depth;
        unsigned int num_surfaces;
        unsigned int num_faces;
        unsigned int num_mipmaps;
        unsigned int metadata_size;
    } pvr_header;

#if 0   // Not used...
    // Metadata (usually 15 bytes)
    typedef struct {
        unsigned int devFOURCC;
        unsigned int key;
        unsigned int data_size;      // Not used?
        unsigned char *data;        // Not used?
    } PVRMetadata;
#endif

    if (file_data_ptr != NULL)
    {
        // Check PVR image version
        unsigned char pvr_version = file_data_ptr[0];

        // Load different PVR data formats
        if (pvr_version == 0x50)
        {
            pvr_header *header = (pvr_header *)file_data_ptr;

            if ((header->id[0] != 'P') || (header->id[1] != 'V') || (header->id[2] != 'R') || (header->id[3] != 3))
            {
                LOG("WARNING: IMAGE: PVR file data not valid");
            }
            else
            {
                file_data_ptr += sizeof(pvr_header);   // Skip header

                *width = header->width;
                *height = header->height;
                *mips = header->num_mipmaps;

                // Check data format
                if (((header->channels[0] == 'l') && (header->channels[1] == 0)) && (header->channel_depth[0] == 8)) *format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
                else if (((header->channels[0] == 'l') && (header->channels[1] == 'a')) && ((header->channel_depth[0] == 8) && (header->channel_depth[1] == 8))) *format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
                else if ((header->channels[0] == 'r') && (header->channels[1] == 'g') && (header->channels[2] == 'b'))
                {
                    if (header->channels[3] == 'a')
                    {
                        if ((header->channel_depth[0] == 5) && (header->channel_depth[1] == 5) && (header->channel_depth[2] == 5) && (header->channel_depth[3] == 1)) *format = PIXELFORMAT_UNCOMPRESSED_R5G5B5A1;
                        else if ((header->channel_depth[0] == 4) && (header->channel_depth[1] == 4) && (header->channel_depth[2] == 4) && (header->channel_depth[3] == 4)) *format = PIXELFORMAT_UNCOMPRESSED_R4G4B4A4;
                        else if ((header->channel_depth[0] == 8) && (header->channel_depth[1] == 8) && (header->channel_depth[2] == 8) && (header->channel_depth[3] == 8)) *format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
                    }
                    else if (header->channels[3] == 0)
                    {
                        if ((header->channel_depth[0] == 5) && (header->channel_depth[1] == 6) && (header->channel_depth[2] == 5)) *format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
                        else if ((header->channel_depth[0] == 8) && (header->channel_depth[1] == 8) && (header->channel_depth[2] == 8)) *format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
                    }
                }
                else if (header->channels[0] == 2) *format = PIXELFORMAT_COMPRESSED_PVRT_RGB;
                else if (header->channels[0] == 3) *format = PIXELFORMAT_COMPRESSED_PVRT_RGBA;

                file_data_ptr += header->metadata_size;    // Skip meta data header

                // Calculate data size (depends on format)
                int bpp = 0;
                switch (*format)
                {
                    case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
                    case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
                    case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
                    case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
                    case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
                    case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
                    case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
                    case PIXELFORMAT_COMPRESSED_PVRT_RGB:
                    case PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
                    default: break;
                }

                int data_size = (*width)*(*height)*bpp/8;  // Total data size in bytes
                image_data = RL_MALLOC(data_size*sizeof(unsigned char));

                memcpy(image_data, file_data_ptr, data_size);
            }
        }
        else if (pvr_version == 52) LOG("INFO: IMAGE: PVRv2 format not supported, update your files to PVRv3");
    }

    return image_data;
}
#endif

#if defined(RL_GPUTEX_SUPPORT_ASTC)
// Load ASTC compressed image data (ASTC compression)
void *rl_load_astc_from_memory(const unsigned char *file_data, unsigned int file_size, int *width, int *height, int *format, int *mips)
{
    void *image_data = NULL;        // Image data pointer

    unsigned char *file_data_ptr = (unsigned char *)file_data;

    // Required extensions:
    // GL_KHR_texture_compression_astc_hdr
    // GL_KHR_texture_compression_astc_ldr

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGBA_ASTC_4x4_KHR      0x93b0
    // GL_COMPRESSED_RGBA_ASTC_8x8_KHR      0x93b7

    // ASTC file Header (16 bytes)
    typedef struct {
        unsigned char id[4];        // Signature: 0x13 0xAB 0xA1 0x5C
        unsigned char blockX;       // Block X dimensions
        unsigned char blockY;       // Block Y dimensions
        unsigned char blockZ;       // Block Z dimensions (1 for 2D images)
        unsigned char width[3];     // void *width in pixels (24bit value)
        unsigned char height[3];    // void *height in pixels (24bit value)
        unsigned char length[3];    // void *Z-size (1 for 2D images)
    } astc_header;

    if (file_data_ptr != NULL)
    {
        astc_header *header = (astc_header *)file_data_ptr;

        if ((header->id[3] != 0x5c) || (header->id[2] != 0xa1) || (header->id[1] != 0xab) || (header->id[0] != 0x13))
        {
            LOG("WARNING: IMAGE: ASTC file data not valid");
        }
        else
        {
            file_data_ptr += sizeof(astc_header);   // Skip header

            // NOTE: Assuming Little Endian (could it be wrong?)
            *width = 0x00000000 | ((int)header->width[2] << 16) | ((int)header->width[1] << 8) | ((int)header->width[0]);
            *height = 0x00000000 | ((int)header->height[2] << 16) | ((int)header->height[1] << 8) | ((int)header->height[0]);
            *mips = 1;      // NOTE: ASTC format only contains one mipmap level

            // NOTE: Each block is always stored in 128bit so we can calculate the bpp
            int bpp = 128/(header->blockX*header->blockY);

            // NOTE: Currently we only support 2 blocks configurations: 4x4 and 8x8
            if ((bpp == 8) || (bpp == 2))
            {
                int data_size = (*width)*(*height)*bpp/8;  // Data size in bytes

                image_data = RL_MALLOC(data_size*sizeof(unsigned char));

                memcpy(image_data, file_data_ptr, data_size);

                if (bpp == 8) *format = PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA;
                else if (bpp == 2) *format = PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA;
            }
            else LOG("WARNING: IMAGE: ASTC block size configuration not supported");
        }
    }

    return image_data;
}
#endif

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
// Get pixel data size in bytes for certain pixel format
static int get_pixel_data_size(int width, int height, int format)
{
    int data_size = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel

    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
        default: break;
    }

    data_size = width*height*bpp/8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4))
    {
        if ((format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < PIXELFORMAT_COMPRESSED_DXT3_RGBA)) data_size = 8;
        else if ((format >= PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) data_size = 16;
    }

    return data_size;
}
#endif // RL_GPUTEX_IMPLEMENTATION
