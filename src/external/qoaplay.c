/*******************************************************************************************
*
*   qoaplay - QOA stream playing helper functions
*
*   qoaplay is a tiny abstraction to read and decode a QOA file "on the fly".
*   It reads and decodes one frame at a time with minimal memory requirements.
*   qoaplay also provides some functions to seek to a specific frame.
*
*   LICENSE: MIT License
*
*   Copyright (c) 2023 Dominic Szablewski (@phoboslab), reviewed by Ramon Santamaria (@raysan5)
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// QOA streaming data descriptor
typedef struct {
    qoa_desc info;                  // QOA descriptor data

    FILE *file;                     // QOA file to read, if NULL, using memory buffer -> file_data
    unsigned char *file_data;       // QOA file data on memory
    unsigned int file_data_size;    // QOA file data on memory size
    unsigned int file_data_offset;  // QOA file data on memory offset for next read

    unsigned int first_frame_pos;   // First frame position (after QOA header, required for offset)
    unsigned int sample_position;   // Current streaming sample position

    unsigned char *buffer;          // Buffer used to read samples from file/memory (used on decoding)

    short *sample_data;             // Sample data decoded
    unsigned int sample_data_len;   // Sample data decoded length
    unsigned int sample_data_pos;   // Sample data decoded position

} qoaplay_desc;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

qoaplay_desc *qoaplay_open(const char *path);
qoaplay_desc *qoaplay_open_memory(const unsigned char *data, int data_size);
void qoaplay_close(qoaplay_desc *qoa_ctx);

void qoaplay_rewind(qoaplay_desc *qoa_ctx);
void qoaplay_seek_frame(qoaplay_desc *qoa_ctx, int frame);
unsigned int qoaplay_decode(qoaplay_desc *qoa_ctx, float *sample_data, int num_samples);
unsigned int qoaplay_decode_frame(qoaplay_desc *qoa_ctx);
double qoaplay_get_duration(qoaplay_desc *qoa_ctx);
double qoaplay_get_time(qoaplay_desc *qoa_ctx);
int qoaplay_get_frame(qoaplay_desc *qoa_ctx);

#if defined(__cplusplus)
}            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Open QOA file, keep FILE pointer to keep reading from file
qoaplay_desc *qoaplay_open(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;

    // Read and decode the file header
    unsigned char header[QOA_MIN_FILESIZE];
    int read = fread(header, QOA_MIN_FILESIZE, 1, file);
    if (!read) return NULL;

    qoa_desc qoa;
    unsigned int first_frame_pos = qoa_decode_header(header, QOA_MIN_FILESIZE, &qoa);
    if (!first_frame_pos) return NULL;

    // Rewind the file back to beginning of the first frame
    fseek(file, first_frame_pos, SEEK_SET);

    // Allocate one chunk of memory for the qoaplay_desc struct
    // + the sample data for one frame
    // + a buffer to hold one frame of encoded data
    unsigned int buffer_size = qoa_max_frame_size(&qoa);
    unsigned int sample_data_size = qoa.channels*QOA_FRAME_LEN*sizeof(short)*2;
    qoaplay_desc *qoa_ctx = QOA_MALLOC(sizeof(qoaplay_desc) + buffer_size + sample_data_size);
    memset(qoa_ctx, 0, sizeof(qoaplay_desc));

    qoa_ctx->file = file;
    qoa_ctx->file_data = NULL;
    qoa_ctx->file_data_size = 0;
    qoa_ctx->file_data_offset = first_frame_pos;
    qoa_ctx->first_frame_pos = first_frame_pos;

    // Setup data pointers to previously allocated data
    qoa_ctx->buffer = ((unsigned char *)qoa_ctx) + sizeof(qoaplay_desc);
    qoa_ctx->sample_data = (short *)(((unsigned char *)qoa_ctx) + sizeof(qoaplay_desc) + buffer_size);

    qoa_ctx->info.channels = qoa.channels;
    qoa_ctx->info.samplerate = qoa.samplerate;
    qoa_ctx->info.samples = qoa.samples;

    return qoa_ctx;
}

// Open QOA file from memory, no FILE pointer required
qoaplay_desc *qoaplay_open_memory(const unsigned char *data, int data_size)
{
    qoa_desc qoa;
    if (data_size < QOA_MIN_FILESIZE) return NULL;
    unsigned int first_frame_pos = qoa_decode_header(data, QOA_MIN_FILESIZE, &qoa);
    if (!first_frame_pos) return NULL;

    // Allocate one chunk of memory for the qoaplay_desc struct
    // + the sample data for one frame
    // + a buffer to hold one frame of encoded data
    unsigned int sample_data_size = qoa.channels*QOA_FRAME_LEN*sizeof(short)*2;
    qoaplay_desc *qoa_ctx = QOA_MALLOC(sizeof(qoaplay_desc) + sample_data_size + data_size);
    memset(qoa_ctx, 0, sizeof(qoaplay_desc));

    qoa_ctx->file = NULL;

    // Keep a copy of file data provided to be managed internally
    qoa_ctx->file_data = (((unsigned char *)qoa_ctx) + sizeof(qoaplay_desc) + sample_data_size);
    memcpy(qoa_ctx->file_data, data, data_size);
    qoa_ctx->file_data_size = data_size;
    qoa_ctx->file_data_offset = first_frame_pos;
    qoa_ctx->first_frame_pos = first_frame_pos;

    // Setup data pointers to previously allocated data
    qoa_ctx->buffer = NULL;
    qoa_ctx->sample_data = (short *)(((unsigned char *)qoa_ctx) + sizeof(qoaplay_desc));

    qoa_ctx->info.channels = qoa.channels;
    qoa_ctx->info.samplerate = qoa.samplerate;
    qoa_ctx->info.samples = qoa.samples;

    return qoa_ctx;
}

// Close QOA file (if open) and free internal memory
void qoaplay_close(qoaplay_desc *qoa_ctx)
{
    if (qoa_ctx->file) fclose(qoa_ctx->file);

    qoa_ctx->file_data_size = 0;

    QOA_FREE(qoa_ctx);
}

// Decode one frame from QOA data
unsigned int qoaplay_decode_frame(qoaplay_desc *qoa_ctx)
{
    unsigned char *buffer;
    unsigned int buffer_len;

    if (qoa_ctx->file)
    {
        buffer = qoa_ctx->buffer;
        buffer_len = fread(buffer, 1, qoa_max_frame_size(&qoa_ctx->info), qoa_ctx->file);
    }
    else
    {
        buffer = qoa_ctx->file_data + qoa_ctx->file_data_offset;
        buffer_len = qoa_max_frame_size(&qoa_ctx->info);
        qoa_ctx->file_data_offset += buffer_len;
    }

    unsigned int frame_len;
    qoa_decode_frame(buffer, buffer_len, &qoa_ctx->info, qoa_ctx->sample_data, &frame_len);
    qoa_ctx->sample_data_pos = 0;
    qoa_ctx->sample_data_len = frame_len;

    return frame_len;
}

// Rewind QOA file or memory pointer to beginning
void qoaplay_rewind(qoaplay_desc *qoa_ctx)
{
    if (qoa_ctx->file) fseek(qoa_ctx->file, qoa_ctx->first_frame_pos, SEEK_SET);
    else qoa_ctx->file_data_offset = qoa_ctx->first_frame_pos;

    qoa_ctx->sample_position = 0;
    qoa_ctx->sample_data_len = 0;
    qoa_ctx->sample_data_pos = 0;
}

// Decode required QOA frames
unsigned int qoaplay_decode(qoaplay_desc *qoa_ctx, float *sample_data, int num_samples)
{
    int src_index = qoa_ctx->sample_data_pos*qoa_ctx->info.channels;
    int dst_index = 0;

    for (int i = 0; i < num_samples; i++)
    {
        // Do we have to decode more samples?
        if (qoa_ctx->sample_data_len - qoa_ctx->sample_data_pos == 0)
        {
            if (!qoaplay_decode_frame(qoa_ctx))
            {
                // Loop to the beginning
                qoaplay_rewind(qoa_ctx);
                qoaplay_decode_frame(qoa_ctx);
            }

            src_index = 0;
        }

        // Normalize to -1..1 floats and write to dest
        for (int c = 0; c < qoa_ctx->info.channels; c++)
        {
            sample_data[dst_index++] = qoa_ctx->sample_data[src_index++]/32768.0;
        }

        qoa_ctx->sample_data_pos++;
        qoa_ctx->sample_position++;
    }

    return num_samples;
}

// Get QOA total time duration in seconds
double qoaplay_get_duration(qoaplay_desc *qoa_ctx)
{
    return (double)qoa_ctx->info.samples/(double)qoa_ctx->info.samplerate;
}

// Get QOA current time position in seconds
double qoaplay_get_time(qoaplay_desc *qoa_ctx)
{
    return (double)qoa_ctx->sample_position/(double)qoa_ctx->info.samplerate;
}

// Get QOA current audio frame
int qoaplay_get_frame(qoaplay_desc *qoa_ctx)
{
    return qoa_ctx->sample_position/QOA_FRAME_LEN;
}

// Seek QOA audio frame
void qoaplay_seek_frame(qoaplay_desc *qoa_ctx, int frame)
{
    if (frame < 0) frame = 0;

    if (frame > qoa_ctx->info.samples/QOA_FRAME_LEN) frame = qoa_ctx->info.samples/QOA_FRAME_LEN;

    qoa_ctx->sample_position = frame*QOA_FRAME_LEN;
    qoa_ctx->sample_data_len = 0;
    qoa_ctx->sample_data_pos = 0;

    unsigned int offset = qoa_ctx->first_frame_pos + frame*qoa_max_frame_size(&qoa_ctx->info);

    if (qoa_ctx->file) fseek(qoa_ctx->file, offset, SEEK_SET);
    else qoa_ctx->file_data_offset = offset;
}
