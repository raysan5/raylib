// jar_xm.h
//
// ORIGINAL LICENSE - FOR LIBXM:
//
// Author: Romain "Artefact2" Dalmaso <artefact2@gmail.com>
// Contributor: Dan Spencer <dan@atomicpotato.net>
// Repackaged into jar_xm.h By: Joshua Adam Reisenauer <kd7tck@gmail.com>
// This program is free software. It comes without any warranty, to the
// extent permitted by applicable law. You can redistribute it and/or
// modify it under the terms of the Do What The Fuck You Want To Public
// License, Version 2, as published by Sam Hocevar. See
// http://sam.zoy.org/wtfpl/COPYING for more details.
//
// HISTORY:
//   v0.1.0 2016-02-22  jar_xm.h - development by Joshua Reisenauer, MAR 2016
//   v0.2.1 2021-03-07  m4ntr0n1c: Fix clipping noise for "bad" xm's (they will always clip), avoid clip noise and just put a ceiling)
//   v0.2.2 2021-03-09  m4ntr0n1c: Add complete debug solution (raylib.h must be included)
//   v0.2.3 2021-03-11  m4ntr0n1c: Fix tempo, bpm and volume on song stop / start / restart / loop
//   v0.2.4 2021-03-17  m4ntr0n1c: Sanitize code for readability
//   v0.2.5 2021-03-22  m4ntr0n1c: Minor adjustments
//   v0.2.6 2021-04-01  m4ntr0n1c: Minor fixes and optimisation
//   v0.3.0 2021-04-03  m4ntr0n1c: Addition of Stereo sample support, Linear Interpolation and Ramping now addressable options in code
//   v0.3.1 2021-04-04  m4ntr0n1c: Volume effects column adjustments, sample offset handling adjustments
//
// USAGE:
//
// In ONE source file, put:
//
//    #define JAR_XM_IMPLEMENTATION
//    #include "jar_xm.h"
//
// Other source files should just include jar_xm.h
//
// SAMPLE CODE:
//
// jar_xm_context_t *musicptr;
// float musicBuffer[48000 / 60];
// int intro_load(void)
// {
//     jar_xm_create_context_from_file(&musicptr, 48000, "Song.XM");
//     return 1;
// }
// int intro_unload(void)
// {
//     jar_xm_free_context(musicptr);
//     return 1;
// }
// int intro_tick(long counter)
// {
//     jar_xm_generate_samples(musicptr, musicBuffer, (48000 / 60) / 2);
//     if(IsKeyDown(KEY_ENTER))
//         return 1;
//     return 0;
// }
//
#ifndef INCLUDE_JAR_XM_H
#define INCLUDE_JAR_XM_H

#include <stdint.h>

#define JAR_XM_DEBUG 0
#define JAR_XM_DEFENSIVE 1
//#define JAR_XM_RAYLIB 0 // set to 0 to disable the RayLib visualizer extension

// Allow custom memory allocators
#ifndef JARXM_MALLOC
    #define JARXM_MALLOC(sz)    malloc(sz)
#endif
#ifndef JARXM_FREE
    #define JARXM_FREE(p)       free(p)
#endif

//-------------------------------------------------------------------------------
struct jar_xm_context_s;
typedef struct jar_xm_context_s jar_xm_context_t;

#ifdef __cplusplus
extern "C" {
#endif

//** Create a XM context.
// * @param moddata the contents of the module
// * @param rate play rate in Hz, recommended value of 48000
// * @returns 0 on success
// * @returns 1 if module data is not sane
// * @returns 2 if memory allocation failed
// * @returns 3 unable to open input file
// * @returns 4 fseek() failed
// * @returns 5 fread() failed
// * @returns 6 unkown error
// * @deprecated This function is unsafe!
// * @see jar_xm_create_context_safe()
int jar_xm_create_context_from_file(jar_xm_context_t** ctx, uint32_t rate, const char* filename);

//** Create a XM context.
// * @param moddata the contents of the module
// * @param rate play rate in Hz, recommended value of 48000
// * @returns 0 on success
// * @returns 1 if module data is not sane
// * @returns 2 if memory allocation failed
// * @deprecated This function is unsafe!
// * @see jar_xm_create_context_safe()
int jar_xm_create_context(jar_xm_context_t** ctx, const char* moddata, uint32_t rate);

//** Create a XM context.
// * @param moddata the contents of the module
// * @param moddata_length the length of the contents of the module, in bytes
// * @param rate play rate in Hz, recommended value of 48000
// * @returns 0 on success
// * @returns 1 if module data is not sane
// * @returns 2 if memory allocation failed
int jar_xm_create_context_safe(jar_xm_context_t** ctx, const char* moddata, size_t moddata_length, uint32_t rate);

//** Free a XM context created by jar_xm_create_context(). */
void jar_xm_free_context(jar_xm_context_t* ctx);

//** Play the module and put the sound samples in an output buffer.
// * @param output buffer of 2*numsamples elements (A left and right value for each sample)
// * @param numsamples number of samples to generate
void jar_xm_generate_samples(jar_xm_context_t* ctx, float* output, size_t numsamples);

//** Play the module, resample from float to 16 bit, and put the sound samples in an output buffer.
// * @param output buffer of 2*numsamples elements (A left and right value for each sample)
// * @param numsamples number of samples to generate
void jar_xm_generate_samples_16bit(jar_xm_context_t* ctx, short* output, size_t numsamples) {
    float* musicBuffer = JARXM_MALLOC((2*numsamples)*sizeof(float));
    jar_xm_generate_samples(ctx, musicBuffer, numsamples);

    if(output){
        for(int x=0;x<2*numsamples;x++) output[x] = (musicBuffer[x] * 32767.0f); // scale sample to signed small int
    }
    JARXM_FREE(musicBuffer);
}

//** Play the module, resample from float to 8 bit, and put the sound samples in an output buffer.
// * @param output buffer of 2*numsamples elements (A left and right value for each sample)
// * @param numsamples number of samples to generate
void jar_xm_generate_samples_8bit(jar_xm_context_t* ctx, char* output, size_t numsamples) {
    float* musicBuffer = JARXM_MALLOC((2*numsamples)*sizeof(float));
    jar_xm_generate_samples(ctx, musicBuffer, numsamples);

    if(output){
        for(int x=0;x<2*numsamples;x++) output[x] = (musicBuffer[x] * 127.0f); // scale sample to signed 8 bit
    }
    JARXM_FREE(musicBuffer);
}

//** Set the maximum number of times a module can loop. After the specified number of loops, calls to jar_xm_generate_samples will only generate silence. You can control the current number of loops with jar_xm_get_loop_count().
// * @param loopcnt maximum number of loops. Use 0 to loop indefinitely.
void jar_xm_set_max_loop_count(jar_xm_context_t* ctx, uint8_t loopcnt);

//** Get the loop count of the currently playing module. This value is 0 when the module is still playing, 1 when the module has looped once, etc.
uint8_t jar_xm_get_loop_count(jar_xm_context_t* ctx);

//** Mute or unmute a channel.
// * @note Channel numbers go from 1 to jar_xm_get_number_of_channels(...).
// * @return whether the channel was muted.
bool jar_xm_mute_channel(jar_xm_context_t* ctx, uint16_t, bool);

//** Mute or unmute an instrument.
// * @note Instrument numbers go from 1 to jar_xm_get_number_of_instruments(...).
// * @return whether the instrument was muted.
bool jar_xm_mute_instrument(jar_xm_context_t* ctx, uint16_t, bool);

//** Get the module name as a NUL-terminated string.
const char* jar_xm_get_module_name(jar_xm_context_t* ctx);

//** Get the tracker name as a NUL-terminated string.
const char* jar_xm_get_tracker_name(jar_xm_context_t* ctx);

//** Get the number of channels.
uint16_t jar_xm_get_number_of_channels(jar_xm_context_t* ctx);

//** Get the module length (in patterns).
uint16_t jar_xm_get_module_length(jar_xm_context_t*);

//** Get the number of patterns.
uint16_t jar_xm_get_number_of_patterns(jar_xm_context_t* ctx);

//** Get the number of rows of a pattern.
// * @note Pattern numbers go from 0 to jar_xm_get_number_of_patterns(...)-1.
uint16_t jar_xm_get_number_of_rows(jar_xm_context_t* ctx, uint16_t);

//** Get the number of instruments.
uint16_t jar_xm_get_number_of_instruments(jar_xm_context_t* ctx);

//** Get the number of samples of an instrument.
// * @note Instrument numbers go from 1 to jar_xm_get_number_of_instruments(...).
uint16_t jar_xm_get_number_of_samples(jar_xm_context_t* ctx, uint16_t);

//** Get the current module speed.
// * @param bpm will receive the current BPM
// * @param tempo will receive the current tempo (ticks per line)
void jar_xm_get_playing_speed(jar_xm_context_t* ctx, uint16_t* bpm, uint16_t* tempo);

//** Get the current position in the module being played.
// * @param pattern_index if not NULL, will receive the current pattern index in the POT (pattern order table)
// * @param pattern if not NULL, will receive the current pattern number
// * @param row if not NULL, will receive the current row
// * @param samples if not NULL, will receive the total number of
// * generated samples (divide by sample rate to get seconds of generated audio)
void jar_xm_get_position(jar_xm_context_t* ctx, uint8_t* pattern_index, uint8_t* pattern, uint8_t* row, uint64_t* samples);

//** Get the latest time (in number of generated samples) when a particular instrument was triggered in any channel.
// * @note Instrument numbers go from 1 to jar_xm_get_number_of_instruments(...).
uint64_t jar_xm_get_latest_trigger_of_instrument(jar_xm_context_t* ctx, uint16_t);

//** Get the latest time (in number of generated samples) when a particular sample was triggered in any channel.
// * @note Instrument numbers go from 1 to jar_xm_get_number_of_instruments(...).
// * @note Sample numbers go from 0 to jar_xm_get_nubmer_of_samples(...,instr)-1.
uint64_t jar_xm_get_latest_trigger_of_sample(jar_xm_context_t* ctx, uint16_t instr, uint16_t sample);

//** Get the latest time (in number of generated samples) when any instrument was triggered in a given channel.
// * @note Channel numbers go from 1 to jar_xm_get_number_of_channels(...).
uint64_t jar_xm_get_latest_trigger_of_channel(jar_xm_context_t* ctx, uint16_t);

//** Get the number of remaining samples. Divide by 2 to get the number of individual LR data samples.
// * @note This is the remaining number of samples before the loop starts module again, or halts if on last pass.
// * @note This function is very slow and should only be run once, if at all.
uint64_t jar_xm_get_remaining_samples(jar_xm_context_t* ctx);

#ifdef __cplusplus
}
#endif
//-------------------------------------------------------------------------------

#ifdef JAR_XM_IMPLEMENTATION

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#ifdef DEBUG
    // Undefine DEBUG to avoid external redefinition warnings/conflicts
    // This is probably a common definition for
    // many external build systems' debug configurations
    #undef DEBUG
#endif

#if JAR_XM_DEBUG            //JAR_XM_DEBUG defined as 0
#include <stdio.h>
#define DEBUG(fmt, ...) do {                                        \
        fprintf(stderr, "%s(): " fmt "\n", __func__, __VA_ARGS__);    \
        fflush(stderr);                                                \
    } while(0)
#else
#define DEBUG(...)
#endif

#if jar_xm_BIG_ENDIAN
#error "Big endian platforms are not yet supported, sorry"
/* Make sure the compiler stops, even if #error is ignored */
extern int __fail[-1];
#endif

/* ----- XM constants ----- */
#define SAMPLE_NAME_LENGTH 22
#define INSTRUMENT_NAME_LENGTH 22
#define MODULE_NAME_LENGTH 20
#define TRACKER_NAME_LENGTH 20
#define PATTERN_ORDER_TABLE_LENGTH 256
#define NUM_NOTES 96 // from 1 to 96, where 1 = C-0
#define NUM_ENVELOPE_POINTS 12 // to be verified if 12 is the max
#define MAX_NUM_ROWS 256

#define jar_xm_SAMPLE_RAMPING_POINTS 8

/* ----- Data types ----- */

enum jar_xm_waveform_type_e {
    jar_xm_SINE_WAVEFORM = 0,
    jar_xm_RAMP_DOWN_WAVEFORM = 1,
    jar_xm_SQUARE_WAVEFORM = 2,
    jar_xm_RANDOM_WAVEFORM = 3,
    jar_xm_RAMP_UP_WAVEFORM = 4,
};
typedef enum jar_xm_waveform_type_e jar_xm_waveform_type_t;

enum jar_xm_loop_type_e {
    jar_xm_NO_LOOP,
    jar_xm_FORWARD_LOOP,
    jar_xm_PING_PONG_LOOP,
};
typedef enum jar_xm_loop_type_e jar_xm_loop_type_t;

enum jar_xm_frequency_type_e {
    jar_xm_LINEAR_FREQUENCIES,
    jar_xm_AMIGA_FREQUENCIES,
};
typedef enum jar_xm_frequency_type_e jar_xm_frequency_type_t;

struct jar_xm_envelope_point_s {
    uint16_t frame;
    uint16_t value;
};
typedef struct jar_xm_envelope_point_s jar_xm_envelope_point_t;

struct jar_xm_envelope_s {
    jar_xm_envelope_point_t points[NUM_ENVELOPE_POINTS];
    uint8_t num_points;
    uint8_t sustain_point;
    uint8_t loop_start_point;
    uint8_t loop_end_point;
    bool enabled;
    bool sustain_enabled;
    bool loop_enabled;
};
typedef struct jar_xm_envelope_s jar_xm_envelope_t;

struct jar_xm_sample_s {
    char name[SAMPLE_NAME_LENGTH + 1];
    int8_t bits; /* Either 8 or 16 */
    int8_t stereo;
    uint32_t length;
    uint32_t loop_start;
    uint32_t loop_length;
    uint32_t loop_end;
    float volume;
    int8_t finetune;
    jar_xm_loop_type_t loop_type;
    float panning;
    int8_t relative_note;
    uint64_t latest_trigger;

    float* data;
 };
 typedef struct jar_xm_sample_s jar_xm_sample_t;

 struct jar_xm_instrument_s {
     char name[INSTRUMENT_NAME_LENGTH + 1];
     uint16_t num_samples;
     uint8_t sample_of_notes[NUM_NOTES];
     jar_xm_envelope_t volume_envelope;
     jar_xm_envelope_t panning_envelope;
     jar_xm_waveform_type_t vibrato_type;
     uint8_t vibrato_sweep;
     uint8_t vibrato_depth;
     uint8_t vibrato_rate;
     uint16_t volume_fadeout;
     uint64_t latest_trigger;
     bool muted;

     jar_xm_sample_t* samples;
 };
 typedef struct jar_xm_instrument_s jar_xm_instrument_t;

 struct jar_xm_pattern_slot_s {
     uint8_t note; /* 1-96, 97 = Key Off note */
     uint8_t instrument; /* 1-128 */
     uint8_t volume_column;
     uint8_t effect_type;
     uint8_t effect_param;
 };
 typedef struct jar_xm_pattern_slot_s jar_xm_pattern_slot_t;

 struct jar_xm_pattern_s {
     uint16_t num_rows;
     jar_xm_pattern_slot_t* slots; /* Array of size num_rows * num_channels */
 };
 typedef struct jar_xm_pattern_s jar_xm_pattern_t;

 struct jar_xm_module_s {
     char name[MODULE_NAME_LENGTH + 1];
     char trackername[TRACKER_NAME_LENGTH + 1];
     uint16_t length;
     uint16_t restart_position;
     uint16_t num_channels;
     uint16_t num_patterns;
     uint16_t num_instruments;
     uint16_t linear_interpolation;
     uint16_t ramping;
     jar_xm_frequency_type_t frequency_type;
     uint8_t pattern_table[PATTERN_ORDER_TABLE_LENGTH];

     jar_xm_pattern_t* patterns;
     jar_xm_instrument_t* instruments; /* Instrument 1 has index 0, instrument 2 has index 1, etc. */
 };
 typedef struct jar_xm_module_s jar_xm_module_t;

 struct jar_xm_channel_context_s {
     float note;
     float orig_note; /* The original note before effect modifications, as read in the pattern. */
     jar_xm_instrument_t* instrument; /* Could be NULL */
     jar_xm_sample_t* sample; /* Could be NULL */
     jar_xm_pattern_slot_t* current;

     float sample_position;
     float period;
     float frequency;
     float step;
     bool ping; /* For ping-pong samples: true is -->, false is <-- */

     float volume; /* Ideally between 0 (muted) and 1 (loudest) */
     float panning; /* Between 0 (left) and 1 (right); 0.5 is centered */

     uint16_t autovibrato_ticks;

     bool sustained;
     float fadeout_volume;
     float volume_envelope_volume;
     float panning_envelope_panning;
     uint16_t volume_envelope_frame_count;
     uint16_t panning_envelope_frame_count;

     float autovibrato_note_offset;

     bool arp_in_progress;
     uint8_t arp_note_offset;
     uint8_t volume_slide_param;
     uint8_t fine_volume_slide_param;
     uint8_t global_volume_slide_param;
     uint8_t panning_slide_param;
     uint8_t portamento_up_param;
     uint8_t portamento_down_param;
     uint8_t fine_portamento_up_param;
     uint8_t fine_portamento_down_param;
     uint8_t extra_fine_portamento_up_param;
     uint8_t extra_fine_portamento_down_param;
     uint8_t tone_portamento_param;
     float tone_portamento_target_period;
     uint8_t multi_retrig_param;
     uint8_t note_delay_param;
     uint8_t pattern_loop_origin; /* Where to restart a E6y loop */
     uint8_t pattern_loop_count; /* How many loop passes have been done */
     bool vibrato_in_progress;
     jar_xm_waveform_type_t vibrato_waveform;
     bool vibrato_waveform_retrigger; /* True if a new note retriggers the waveform */
     uint8_t vibrato_param;
     uint16_t vibrato_ticks; /* Position in the waveform */
     float vibrato_note_offset;
     jar_xm_waveform_type_t tremolo_waveform;
     bool tremolo_waveform_retrigger;
     uint8_t tremolo_param;
     uint8_t tremolo_ticks;
     float tremolo_volume;
     uint8_t tremor_param;
     bool tremor_on;

     uint64_t latest_trigger;
     bool muted;

     //* These values are updated at the end of each tick, to save a couple of float operations on every generated sample.
     float target_panning;
     float target_volume;

     unsigned long frame_count;
     float end_of_previous_sample_left[jar_xm_SAMPLE_RAMPING_POINTS];
     float end_of_previous_sample_right[jar_xm_SAMPLE_RAMPING_POINTS];
     float curr_left;
     float curr_right;

     float actual_panning;
     float actual_volume;
 };
 typedef struct jar_xm_channel_context_s jar_xm_channel_context_t;

 struct jar_xm_context_s {
     void* allocated_memory;
     jar_xm_module_t module;
     uint32_t rate;

     uint16_t default_tempo; // Number of ticks per row
     uint16_t default_bpm;
     float default_global_volume;

     uint16_t tempo; // Number of ticks per row
     uint16_t bpm;
     float global_volume;

     float volume_ramp; /* How much is a channel final volume allowed to change per sample; this is used to avoid abrubt volume changes which manifest as "clicks" in the generated sound. */
     float panning_ramp; /* Same for panning. */

     uint8_t current_table_index;
     uint8_t current_row;
     uint16_t current_tick; /* Can go below 255, with high tempo and a pattern delay */
     float remaining_samples_in_tick;
     uint64_t generated_samples;

     bool position_jump;
     bool pattern_break;
     uint8_t jump_dest;
     uint8_t jump_row;

     uint16_t extra_ticks; /* Extra ticks to be played before going to the next row - Used for EEy effect */

     uint8_t* row_loop_count; /* Array of size MAX_NUM_ROWS * module_length */
     uint8_t loop_count;
     uint8_t max_loop_count;

     jar_xm_channel_context_t* channels;
};

#if JAR_XM_DEFENSIVE

//** Check the module data for errors/inconsistencies.
// * @returns 0 if everything looks OK. Module should be safe to load.
int jar_xm_check_sanity_preload(const char*, size_t);

//** Check a loaded module for errors/inconsistencies.
// * @returns 0 if everything looks OK.
int jar_xm_check_sanity_postload(jar_xm_context_t*);

#endif

//** Get the number of bytes needed to store the module data in a dynamically allocated blank context.
// * Things that are dynamically allocated:
// * - sample data
// * - sample structures in instruments
// * - pattern data
// * - row loop count arrays
// * - pattern structures in module
// * - instrument structures in module
// * - channel contexts
// * - context structure itself
// * @returns 0 if everything looks OK.
size_t jar_xm_get_memory_needed_for_context(const char*, size_t);

//** Populate the context from module data.
// * @returns pointer to the memory pool
char* jar_xm_load_module(jar_xm_context_t*, const char*, size_t, char*);

int jar_xm_create_context(jar_xm_context_t** ctxp, const char* moddata, uint32_t rate) {
    return jar_xm_create_context_safe(ctxp, moddata, SIZE_MAX, rate);
}

#define ALIGN(x, b) (((x) + ((b) - 1)) & ~((b) - 1))
#define ALIGN_PTR(x, b) (void*)(((uintptr_t)(x) + ((b) - 1)) & ~((b) - 1))
int jar_xm_create_context_safe(jar_xm_context_t** ctxp, const char* moddata, size_t moddata_length, uint32_t rate) {
#if JAR_XM_DEFENSIVE
    int ret;
#endif
    size_t bytes_needed;
    char* mempool;
    jar_xm_context_t* ctx;

#if JAR_XM_DEFENSIVE
    if((ret = jar_xm_check_sanity_preload(moddata, moddata_length))) {
        DEBUG("jar_xm_check_sanity_preload() returned %i, module is not safe to load", ret);
        return 1;
    }
#endif

    bytes_needed = jar_xm_get_memory_needed_for_context(moddata, moddata_length);
    mempool = JARXM_MALLOC(bytes_needed);
    if(mempool == NULL && bytes_needed > 0) { /* JARXM_MALLOC() failed, trouble ahead */
        DEBUG("call to JARXM_MALLOC() failed, returned %p", (void*)mempool);
        return 2;
    }

    /* Initialize most of the fields to 0, 0.f, NULL or false depending on type */
    memset(mempool, 0, bytes_needed);

    ctx = (*ctxp = (jar_xm_context_t *)mempool);
    ctx->allocated_memory = mempool; /* Keep original pointer for JARXM_FREE() */
    mempool += sizeof(jar_xm_context_t);

    ctx->rate = rate;
    mempool = jar_xm_load_module(ctx, moddata, moddata_length, mempool);
    mempool = ALIGN_PTR(mempool, 16);

    ctx->channels = (jar_xm_channel_context_t*)mempool;
    mempool += ctx->module.num_channels * sizeof(jar_xm_channel_context_t);
    mempool = ALIGN_PTR(mempool, 16);

    ctx->default_global_volume = 1.f;
    ctx->global_volume = ctx->default_global_volume;

    ctx->volume_ramp = (1.f / 128.f);
    ctx->panning_ramp = (1.f / 128.f);

    for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
        jar_xm_channel_context_t *ch = ctx->channels + i;
        ch->ping = true;
        ch->vibrato_waveform = jar_xm_SINE_WAVEFORM;
        ch->vibrato_waveform_retrigger = true;
        ch->tremolo_waveform = jar_xm_SINE_WAVEFORM;
        ch->tremolo_waveform_retrigger = true;
        ch->volume = ch->volume_envelope_volume = ch->fadeout_volume = 1.0f;
        ch->panning = ch->panning_envelope_panning = .5f;
        ch->actual_volume = .0f;
        ch->actual_panning = .5f;
    }

    mempool = ALIGN_PTR(mempool, 16);
    ctx->row_loop_count = (uint8_t *)mempool;
    mempool += MAX_NUM_ROWS * sizeof(uint8_t);

#if JAR_XM_DEFENSIVE
    if((ret = jar_xm_check_sanity_postload(ctx))) {   DEBUG("jar_xm_check_sanity_postload() returned %i, module is not safe to play", ret);
        jar_xm_free_context(ctx);
        return 1;
    }
#endif

    return 0;
}

void jar_xm_free_context(jar_xm_context_t *ctx) {
    if (ctx != NULL) {   JARXM_FREE(ctx->allocated_memory); }
}

void jar_xm_set_max_loop_count(jar_xm_context_t *ctx, uint8_t loopcnt) {
    ctx->max_loop_count = loopcnt;
}

uint8_t jar_xm_get_loop_count(jar_xm_context_t *ctx) {
    return ctx->loop_count;
}

bool jar_xm_mute_channel(jar_xm_context_t *ctx, uint16_t channel, bool mute) {
    bool old = ctx->channels[channel - 1].muted;
    ctx->channels[channel - 1].muted = mute;
    return old;
}

bool jar_xm_mute_instrument(jar_xm_context_t *ctx, uint16_t instr, bool mute) {
    bool old = ctx->module.instruments[instr - 1].muted;
    ctx->module.instruments[instr - 1].muted = mute;
    return old;
}

const char* jar_xm_get_module_name(jar_xm_context_t *ctx) {
    return ctx->module.name;
}

const char* jar_xm_get_tracker_name(jar_xm_context_t *ctx) {
    return ctx->module.trackername;
}

uint16_t jar_xm_get_number_of_channels(jar_xm_context_t *ctx) {
    return ctx->module.num_channels;
}

uint16_t jar_xm_get_module_length(jar_xm_context_t *ctx) {
    return ctx->module.length;
}

uint16_t jar_xm_get_number_of_patterns(jar_xm_context_t *ctx) {
    return ctx->module.num_patterns;
}

uint16_t jar_xm_get_number_of_rows(jar_xm_context_t *ctx, uint16_t pattern) {
    return ctx->module.patterns[pattern].num_rows;
}

uint16_t jar_xm_get_number_of_instruments(jar_xm_context_t *ctx) {
    return ctx->module.num_instruments;
}

uint16_t jar_xm_get_number_of_samples(jar_xm_context_t *ctx, uint16_t instrument) {
    return ctx->module.instruments[instrument - 1].num_samples;
}

void jar_xm_get_playing_speed(jar_xm_context_t *ctx, uint16_t *bpm, uint16_t *tempo) {
    if(bpm) *bpm = ctx->bpm;
    if(tempo) *tempo = ctx->tempo;
}

void jar_xm_get_position(jar_xm_context_t *ctx, uint8_t *pattern_index, uint8_t *pattern, uint8_t *row, uint64_t *samples) {
    if(pattern_index) *pattern_index = ctx->current_table_index;
    if(pattern) *pattern = ctx->module.pattern_table[ctx->current_table_index];
    if(row) *row = ctx->current_row;
    if(samples) *samples = ctx->generated_samples;
}

uint64_t jar_xm_get_latest_trigger_of_instrument(jar_xm_context_t *ctx, uint16_t instr) {
    return ctx->module.instruments[instr - 1].latest_trigger;
}

uint64_t jar_xm_get_latest_trigger_of_sample(jar_xm_context_t *ctx, uint16_t instr, uint16_t sample) {
    return ctx->module.instruments[instr - 1].samples[sample].latest_trigger;
}

uint64_t jar_xm_get_latest_trigger_of_channel(jar_xm_context_t *ctx, uint16_t chn) {
    return ctx->channels[chn - 1].latest_trigger;
}

//* .xm files are little-endian. (XXX: Are they really?)

//* Bound reader macros.
//* If we attempt to read the buffer out-of-bounds, pretend that the buffer is infinitely padded with zeroes.
#define READ_U8(offset) (((offset) < moddata_length) ? (*(uint8_t*)(moddata + (offset))) : 0)
#define READ_U16(offset) ((uint16_t)READ_U8(offset) | ((uint16_t)READ_U8((offset) + 1) << 8))
#define READ_U32(offset) ((uint32_t)READ_U16(offset) | ((uint32_t)READ_U16((offset) + 2) << 16))
#define READ_MEMCPY(ptr, offset, length) memcpy_pad(ptr, length, moddata, moddata_length, offset)

static void memcpy_pad(void *dst, size_t dst_len, const void *src, size_t src_len, size_t offset) {
    uint8_t *dst_c = dst;
    const uint8_t *src_c = src;

    /* how many bytes can be copied without overrunning `src` */
    size_t copy_bytes = (src_len >= offset) ? (src_len - offset) : 0;
    copy_bytes = copy_bytes > dst_len ? dst_len : copy_bytes;

    memcpy(dst_c, src_c + offset, copy_bytes);
    /* padded bytes */
    memset(dst_c + copy_bytes, 0, dst_len - copy_bytes);
}

#if JAR_XM_DEFENSIVE

int jar_xm_check_sanity_preload(const char* module, size_t module_length) {
    if(module_length < 60) { return 4; }
    if(memcmp("Extended Module: ", module, 17) != 0) { return 1; }
    if(module[37] != 0x1A) { return 2; }
    if(module[59] != 0x01 || module[58] != 0x04) { return 3; }  /* Not XM 1.04 */
    return 0;
}

int jar_xm_check_sanity_postload(jar_xm_context_t* ctx) {
    /* Check the POT */
    for(uint8_t i = 0; i < ctx->module.length; ++i) {
        if(ctx->module.pattern_table[i] >= ctx->module.num_patterns) {
            if(i+1 == ctx->module.length && ctx->module.length > 1) {
                DEBUG("trimming invalid POT at pos %X", i);
                --ctx->module.length;
            } else {
                DEBUG("module has invalid POT, pos %X references nonexistent pattern %X", i, ctx->module.pattern_table[i]);
                return 1;
            }
        }
    }

    return 0;
}

#endif

size_t jar_xm_get_memory_needed_for_context(const char* moddata, size_t moddata_length) {
    size_t memory_needed = 0;
    size_t offset = 60; /* 60 = Skip the first header */
    uint16_t num_channels;
    uint16_t num_patterns;
    uint16_t num_instruments;

    /* Read the module header */
    num_channels = READ_U16(offset + 8);
    num_patterns = READ_U16(offset + 10);
    memory_needed += num_patterns * sizeof(jar_xm_pattern_t);
    memory_needed  = ALIGN(memory_needed, 16);
    num_instruments = READ_U16(offset + 12);
    memory_needed += num_instruments * sizeof(jar_xm_instrument_t);
    memory_needed  = ALIGN(memory_needed, 16);
    memory_needed += MAX_NUM_ROWS * READ_U16(offset + 4) * sizeof(uint8_t); /* Module length */

    offset += READ_U32(offset); /* Header size */

    /* Read pattern headers */
    for(uint16_t i = 0; i < num_patterns; ++i) {
        uint16_t num_rows;
        num_rows = READ_U16(offset + 5);
        memory_needed += num_rows * num_channels * sizeof(jar_xm_pattern_slot_t);
        offset += READ_U32(offset) + READ_U16(offset + 7); /* Pattern header length + packed pattern data size */
    }
    memory_needed  = ALIGN(memory_needed, 16);

    /* Read instrument headers */
    for(uint16_t i = 0; i < num_instruments; ++i) {
        uint16_t num_samples;
        uint32_t sample_header_size = 0;
        uint32_t sample_size_aggregate = 0;
        num_samples = READ_U16(offset + 27);
        memory_needed += num_samples * sizeof(jar_xm_sample_t);
        if(num_samples > 0) { sample_header_size = READ_U32(offset + 29); }

        offset += READ_U32(offset);  /* Instrument header size */
        for(uint16_t j = 0; j < num_samples; ++j) {
            uint32_t sample_size;
            uint8_t flags;
            sample_size = READ_U32(offset);
            flags = READ_U8(offset + 14);
            sample_size_aggregate += sample_size;

            if(flags & (1 << 4)) {  /* 16 bit sample */
                memory_needed += sample_size * (sizeof(float) >> 1);
            } else {  /* 8 bit sample */
                memory_needed += sample_size * sizeof(float);
            }
            offset += sample_header_size;
        }
        offset += sample_size_aggregate;
    }

    memory_needed += num_channels * sizeof(jar_xm_channel_context_t);
    memory_needed += sizeof(jar_xm_context_t);
    return memory_needed;
}

char* jar_xm_load_module(jar_xm_context_t* ctx, const char* moddata, size_t moddata_length, char* mempool) {
    size_t offset = 0;
    jar_xm_module_t* mod = &(ctx->module);

    /* Read XM header */
    READ_MEMCPY(mod->name, offset + 17, MODULE_NAME_LENGTH);
    READ_MEMCPY(mod->trackername, offset + 38, TRACKER_NAME_LENGTH);
    offset += 60;

    /* Read module header */
    uint32_t header_size = READ_U32(offset);
    mod->length = READ_U16(offset + 4);
    mod->restart_position = READ_U16(offset + 6);
    mod->num_channels = READ_U16(offset + 8);
    mod->num_patterns = READ_U16(offset + 10);
    mod->num_instruments = READ_U16(offset + 12);
    mod->patterns = (jar_xm_pattern_t*)mempool;
    mod->linear_interpolation = 1; // Linear interpolation can be set after loading
    mod->ramping = 1; // ramping can be set after loading
    mempool += mod->num_patterns * sizeof(jar_xm_pattern_t);
    mempool = ALIGN_PTR(mempool, 16);
    mod->instruments = (jar_xm_instrument_t*)mempool;
    mempool += mod->num_instruments * sizeof(jar_xm_instrument_t);
    mempool = ALIGN_PTR(mempool, 16);
    uint16_t flags = READ_U32(offset + 14);
    mod->frequency_type = (flags & (1 << 0)) ? jar_xm_LINEAR_FREQUENCIES : jar_xm_AMIGA_FREQUENCIES;
    ctx->default_tempo = READ_U16(offset + 16);
    ctx->default_bpm = READ_U16(offset + 18);
    ctx->tempo =ctx->default_tempo;
    ctx->bpm = ctx->default_bpm;

    READ_MEMCPY(mod->pattern_table, offset + 20, PATTERN_ORDER_TABLE_LENGTH);
    offset += header_size;

    /* Read patterns */
    for(uint16_t i = 0; i < mod->num_patterns; ++i) {
        uint16_t packed_patterndata_size = READ_U16(offset + 7);
        jar_xm_pattern_t* pat = mod->patterns + i;
        pat->num_rows = READ_U16(offset + 5);
        pat->slots = (jar_xm_pattern_slot_t*)mempool;
        mempool += mod->num_channels * pat->num_rows * sizeof(jar_xm_pattern_slot_t);
        offset += READ_U32(offset); /* Pattern header length */

        if(packed_patterndata_size == 0) {    /* No pattern data is present */
            memset(pat->slots, 0, sizeof(jar_xm_pattern_slot_t) * pat->num_rows * mod->num_channels);
        } else {
            /* This isn't your typical for loop */
            for(uint16_t j = 0, k = 0; j < packed_patterndata_size; ++k) {
                uint8_t note = READ_U8(offset + j);
                jar_xm_pattern_slot_t* slot = pat->slots + k;
                if(note & (1 << 7)) {
                    /* MSB is set, this is a compressed packet */
                    ++j;
                    if(note & (1 << 0)) {    /* Note follows */
                        slot->note = READ_U8(offset + j);
                        ++j;
                    } else {
                        slot->note = 0;
                    }
                    if(note & (1 << 1)) {    /* Instrument follows */
                        slot->instrument = READ_U8(offset + j);
                        ++j;
                    } else {
                        slot->instrument = 0;
                    }
                    if(note & (1 << 2)) {    /* Volume column follows */
                        slot->volume_column = READ_U8(offset + j);
                        ++j;
                    } else {
                        slot->volume_column = 0;
                    }
                    if(note & (1 << 3)) {    /* Effect follows */
                        slot->effect_type = READ_U8(offset + j);
                        ++j;
                    } else {
                        slot->effect_type = 0;
                    }
                    if(note & (1 << 4)) {    /* Effect parameter follows */
                        slot->effect_param = READ_U8(offset + j);
                        ++j;
                    } else {
                        slot->effect_param = 0;
                    }
                } else {    /* Uncompressed packet */
                    slot->note = note;
                    slot->instrument = READ_U8(offset + j + 1);
                    slot->volume_column = READ_U8(offset + j + 2);
                    slot->effect_type = READ_U8(offset + j + 3);
                    slot->effect_param = READ_U8(offset + j + 4);
                    j += 5;
                }
            }
        }

        offset += packed_patterndata_size;
    }
    mempool = ALIGN_PTR(mempool, 16);

    /* Read instruments */
    for(uint16_t i = 0; i < ctx->module.num_instruments; ++i) {
        uint32_t sample_header_size = 0;
        jar_xm_instrument_t* instr = mod->instruments + i;

        READ_MEMCPY(instr->name, offset + 4, INSTRUMENT_NAME_LENGTH);
        instr->num_samples = READ_U16(offset + 27);

        if(instr->num_samples > 0) {
            /* Read extra header properties */
            sample_header_size = READ_U32(offset + 29);
            READ_MEMCPY(instr->sample_of_notes, offset + 33, NUM_NOTES);

            instr->volume_envelope.num_points = READ_U8(offset + 225);
            instr->panning_envelope.num_points = READ_U8(offset + 226);

            for(uint8_t j = 0; j < instr->volume_envelope.num_points; ++j) {
                instr->volume_envelope.points[j].frame = READ_U16(offset + 129 + 4 * j);
                instr->volume_envelope.points[j].value = READ_U16(offset + 129 + 4 * j + 2);
            }

            for(uint8_t j = 0; j < instr->panning_envelope.num_points; ++j) {
                instr->panning_envelope.points[j].frame = READ_U16(offset + 177 + 4 * j);
                instr->panning_envelope.points[j].value = READ_U16(offset + 177 + 4 * j + 2);
            }

            instr->volume_envelope.sustain_point = READ_U8(offset + 227);
            instr->volume_envelope.loop_start_point = READ_U8(offset + 228);
            instr->volume_envelope.loop_end_point = READ_U8(offset + 229);
            instr->panning_envelope.sustain_point = READ_U8(offset + 230);
            instr->panning_envelope.loop_start_point = READ_U8(offset + 231);
            instr->panning_envelope.loop_end_point = READ_U8(offset + 232);

            uint8_t flags = READ_U8(offset + 233);
            instr->volume_envelope.enabled = flags & (1 << 0);
            instr->volume_envelope.sustain_enabled = flags & (1 << 1);
            instr->volume_envelope.loop_enabled = flags & (1 << 2);

            flags = READ_U8(offset + 234);
            instr->panning_envelope.enabled = flags & (1 << 0);
            instr->panning_envelope.sustain_enabled = flags & (1 << 1);
            instr->panning_envelope.loop_enabled = flags & (1 << 2);
            instr->vibrato_type = READ_U8(offset + 235);
            if(instr->vibrato_type == 2) {
                instr->vibrato_type = 1;
            } else if(instr->vibrato_type == 1) {
                instr->vibrato_type = 2;
            }
            instr->vibrato_sweep = READ_U8(offset + 236);
            instr->vibrato_depth = READ_U8(offset + 237);
            instr->vibrato_rate = READ_U8(offset + 238);
            instr->volume_fadeout = READ_U16(offset + 239);
            instr->samples = (jar_xm_sample_t*)mempool;
            mempool += instr->num_samples * sizeof(jar_xm_sample_t);
        } else {
            instr->samples = NULL;
        }

        /* Instrument header size */
        offset += READ_U32(offset);

        for(int j = 0; j < instr->num_samples; ++j) {
            /* Read sample header */
            jar_xm_sample_t* sample = instr->samples + j;

            sample->length = READ_U32(offset);
            sample->loop_start = READ_U32(offset + 4);
            sample->loop_length = READ_U32(offset + 8);
            sample->loop_end = sample->loop_start + sample->loop_length;
            sample->volume = (float)(READ_U8(offset + 12) << 2) / 256.f;
            if (sample->volume > 1.0f) {sample->volume = 1.f;};
            sample->finetune = (int8_t)READ_U8(offset + 13);

            uint8_t flags = READ_U8(offset + 14);
            switch (flags & 3) {
            case 2:
            case 3:
                sample->loop_type = jar_xm_PING_PONG_LOOP;
            case 1:
                sample->loop_type = jar_xm_FORWARD_LOOP;
                break;
            default:
                sample->loop_type = jar_xm_NO_LOOP;
                break;
            };
            sample->bits = (flags & 0x10) ? 16 : 8;
            sample->stereo = (flags & 0x20) ? 1 : 0;
            sample->panning = (float)READ_U8(offset + 15) / 255.f;
            sample->relative_note = (int8_t)READ_U8(offset + 16);
            READ_MEMCPY(sample->name, 18, SAMPLE_NAME_LENGTH);
            sample->data = (float*)mempool;
            if(sample->bits == 16) {
                /* 16 bit sample */
                mempool += sample->length * (sizeof(float) >> 1);
                sample->loop_start >>= 1;
                sample->loop_length >>= 1;
                sample->loop_end >>= 1;
                sample->length >>= 1;
            } else {
                /* 8 bit sample */
                mempool += sample->length * sizeof(float);
            }
            // Adjust loop points to reflect half of the reported length (stereo)
            if (sample->stereo && sample->loop_type != jar_xm_NO_LOOP) {
                div_t lstart = div(READ_U32(offset + 4), 2);
                sample->loop_start = lstart.quot;
                div_t llength = div(READ_U32(offset + 8), 2);
                sample->loop_length = llength.quot;
                sample->loop_end = sample->loop_start + sample->loop_length;
            };

            offset += sample_header_size;
        }

        // Read all samples and convert them to float values
        for(int j = 0; j < instr->num_samples; ++j) {
            /* Read sample data */
            jar_xm_sample_t* sample = instr->samples + j;
            int length = sample->length;
            if (sample->stereo) {
                // Since it is stereo, we cut the sample in half (treated as single channel)
                div_t result = div(sample->length, 2);
                if(sample->bits == 16) {
                    int16_t v = 0;
                    for(int k = 0; k < length; ++k) {
                        if (k == result.quot) { v = 0;};
                        v = v + (int16_t)READ_U16(offset + (k << 1));
                        sample->data[k] = (float) v / 32768.f ;//* sign;
                        if(sample->data[k] < -1.0)  {sample->data[k] = -1.0;}  else if(sample->data[k] > 1.0)  {sample->data[k] = 1.0;};
                    }
                    offset += sample->length << 1;
                } else {
                    int8_t v = 0;
                    for(int k = 0; k < length; ++k) {
                        if (k == result.quot) { v = 0;};
                        v = v + (int8_t)READ_U8(offset + k);
                        sample->data[k] = (float)v  / 128.f ;//* sign;
                        if(sample->data[k] < -1.0)  {sample->data[k] = -1.0;}  else if(sample->data[k] > 1.0)  {sample->data[k] = 1.0;};
                    }
                    offset += sample->length;
                };
                sample->length = result.quot;
            } else {
                if(sample->bits == 16) {
                    int16_t v = 0;
                    for(int k = 0; k < length; ++k) {
                        v = v + (int16_t)READ_U16(offset + (k << 1));
                        sample->data[k] = (float) v / 32768.f ;//* sign;
                        if(sample->data[k] < -1.0)  {sample->data[k] = -1.0;}  else if(sample->data[k] > 1.0)  {sample->data[k] = 1.0;};
                    }
                    offset += sample->length << 1;
                } else {
                    int8_t v = 0;
                    for(int k = 0; k < length; ++k) {
                        v = v + (int8_t)READ_U8(offset + k);
                        sample->data[k] = (float)v  / 128.f ;//* sign;
                        if(sample->data[k] < -1.0)  {sample->data[k] = -1.0;}  else if(sample->data[k] > 1.0)  {sample->data[k] = 1.0;};
                    }
                    offset += sample->length;
                }
            }
        };
    };
    return mempool;
};

//-------------------------------------------------------------------------------
//THE FOLLOWING IS FOR PLAYING
static float jar_xm_waveform(jar_xm_waveform_type_t, uint8_t);
static void jar_xm_autovibrato(jar_xm_context_t*, jar_xm_channel_context_t*);
static void jar_xm_vibrato(jar_xm_context_t*, jar_xm_channel_context_t*, uint8_t, uint16_t);
static void jar_xm_tremolo(jar_xm_context_t*, jar_xm_channel_context_t*, uint8_t, uint16_t);
static void jar_xm_arpeggio(jar_xm_context_t*, jar_xm_channel_context_t*, uint8_t, uint16_t);
static void jar_xm_tone_portamento(jar_xm_context_t*, jar_xm_channel_context_t*);
static void jar_xm_pitch_slide(jar_xm_context_t*, jar_xm_channel_context_t*, float);
static void jar_xm_panning_slide(jar_xm_channel_context_t*, uint8_t);
static void jar_xm_volume_slide(jar_xm_channel_context_t*, uint8_t);

static float jar_xm_envelope_lerp(jar_xm_envelope_point_t*, jar_xm_envelope_point_t*, uint16_t);
static void jar_xm_envelope_tick(jar_xm_channel_context_t*, jar_xm_envelope_t*, uint16_t*, float*);
static void jar_xm_envelopes(jar_xm_channel_context_t*);

static float jar_xm_linear_period(float);
static float jar_xm_linear_frequency(float);
static float jar_xm_amiga_period(float);
static float jar_xm_amiga_frequency(float);
static float jar_xm_period(jar_xm_context_t*, float);
static float jar_xm_frequency(jar_xm_context_t*, float, float);
static void jar_xm_update_frequency(jar_xm_context_t*, jar_xm_channel_context_t*);

static void jar_xm_handle_note_and_instrument(jar_xm_context_t*, jar_xm_channel_context_t*, jar_xm_pattern_slot_t*);
static void jar_xm_trigger_note(jar_xm_context_t*, jar_xm_channel_context_t*, unsigned int flags);
static void jar_xm_cut_note(jar_xm_channel_context_t*);
static void jar_xm_key_off(jar_xm_channel_context_t*);

static void jar_xm_post_pattern_change(jar_xm_context_t*);
static void jar_xm_row(jar_xm_context_t*);
static void jar_xm_tick(jar_xm_context_t*);

static void jar_xm_next_of_sample(jar_xm_context_t*, jar_xm_channel_context_t*, int);
static void jar_xm_mixdown(jar_xm_context_t*, float*, float*);

#define jar_xm_TRIGGER_KEEP_VOLUME (1 << 0)
#define jar_xm_TRIGGER_KEEP_PERIOD (1 << 1)
#define jar_xm_TRIGGER_KEEP_SAMPLE_POSITION (1 << 2)

                                            // C-2, C#2, D-2, D#2, E-2, F-2, F#2, G-2, G#2, A-2, A#2, B-2, C-3
static const uint16_t amiga_frequencies[] = { 1712, 1616, 1525, 1440, 1357, 1281, 1209, 1141, 1077, 1017,  961,  907, 856 };

                                            // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f
static const float multi_retrig_add[] = { 0.f, -1.f, -2.f, -4.f, -8.f, -16.f, 0.f, 0.f, 0.f, 1.f, 2.f, 4.f, 8.f, 16.f, 0.f, 0.f };

                                            // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f
static const float multi_retrig_multiply[] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, .6666667f, .5f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.5f, 2.f };

#define jar_xm_CLAMP_UP1F(vol, limit) do {            \
        if((vol) > (limit)) (vol) = (limit);    \
    } while(0)
#define jar_xm_CLAMP_UP(vol) jar_xm_CLAMP_UP1F((vol), 1.f)

#define jar_xm_CLAMP_DOWN1F(vol, limit) do {        \
        if((vol) < (limit)) (vol) = (limit);    \
    } while(0)
#define jar_xm_CLAMP_DOWN(vol) jar_xm_CLAMP_DOWN1F((vol), .0f)

#define jar_xm_CLAMP2F(vol, up, down) do {            \
        if((vol) > (up)) (vol) = (up);            \
        else if((vol) < (down)) (vol) = (down); \
    } while(0)
#define jar_xm_CLAMP(vol) jar_xm_CLAMP2F((vol), 1.f, .0f)

#define jar_xm_SLIDE_TOWARDS(val, goal, incr) do {        \
        if((val) > (goal)) {                        \
            (val) -= (incr);                        \
            jar_xm_CLAMP_DOWN1F((val), (goal));            \
        } else if((val) < (goal)) {                    \
            (val) += (incr);                        \
            jar_xm_CLAMP_UP1F((val), (goal));            \
        }                                            \
    } while(0)

#define jar_xm_LERP(u, v, t) ((u) + (t) * ((v) - (u)))
#define jar_xm_INVERSE_LERP(u, v, lerp) (((lerp) - (u)) / ((v) - (u)))

#define HAS_TONE_PORTAMENTO(s) ((s)->effect_type == 3 \
                                 || (s)->effect_type == 5 \
                                 || ((s)->volume_column >> 4) == 0xF)
#define HAS_ARPEGGIO(s) ((s)->effect_type == 0 \
                          && (s)->effect_param != 0)
#define HAS_VIBRATO(s) ((s)->effect_type == 4 \
                         || (s)->effect_param == 6 \
                         || ((s)->volume_column >> 4) == 0xB)
#define NOTE_IS_VALID(n) ((n) > 0 && (n) < 97)
#define NOTE_OFF 97

static float jar_xm_waveform(jar_xm_waveform_type_t waveform, uint8_t step) {
    static unsigned int next_rand = 24492;
    step %= 0x40;
    switch(waveform) {
    case jar_xm_SINE_WAVEFORM: /* No SIN() table used, direct calculation. */
        return -sinf(2.f * 3.141592f * (float)step / (float)0x40);
    case jar_xm_RAMP_DOWN_WAVEFORM: /* Ramp down: 1.0f when step = 0; -1.0f when step = 0x40 */
        return (float)(0x20 - step) / 0x20;
    case jar_xm_SQUARE_WAVEFORM: /* Square with a 50% duty */
        return (step >= 0x20) ? 1.f : -1.f;
    case jar_xm_RANDOM_WAVEFORM: /* Use the POSIX.1-2001 example, just to be deterministic across different machines */
        next_rand = next_rand * 1103515245 + 12345;
        return (float)((next_rand >> 16) & 0x7FFF) / (float)0x4000 - 1.f;
    case jar_xm_RAMP_UP_WAVEFORM: /* Ramp up: -1.f when step = 0; 1.f when step = 0x40 */
        return (float)(step - 0x20) / 0x20;
    default:
        break;
    }
    return .0f;
}

static void jar_xm_autovibrato(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch) {
    if(ch->instrument == NULL || ch->instrument->vibrato_depth == 0) return;
    jar_xm_instrument_t* instr = ch->instrument;
    float sweep = 1.f;
    if(ch->autovibrato_ticks < instr->vibrato_sweep) { sweep = jar_xm_LERP(0.f, 1.f, (float)ch->autovibrato_ticks / (float)instr->vibrato_sweep); }
    unsigned int step = ((ch->autovibrato_ticks++) * instr->vibrato_rate) >> 2;
    ch->autovibrato_note_offset = .25f * jar_xm_waveform(instr->vibrato_type, step) * (float)instr->vibrato_depth / (float)0xF * sweep;
    jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_vibrato(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, uint8_t param, uint16_t pos) {
    unsigned int step = pos * (param >> 4);
    ch->vibrato_note_offset = 2.f * jar_xm_waveform(ch->vibrato_waveform, step) * (float)(param & 0x0F) / (float)0xF;
    jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_tremolo(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, uint8_t param, uint16_t pos) {
    unsigned int step = pos * (param >> 4);
    ch->tremolo_volume = -1.f * jar_xm_waveform(ch->tremolo_waveform, step) * (float)(param & 0x0F) / (float)0xF;
}

static void jar_xm_arpeggio(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, uint8_t param, uint16_t tick) {
    switch(tick % 3) {
    case 0:
        ch->arp_in_progress = false;
        ch->arp_note_offset = 0;
        break;
    case 2:
        ch->arp_in_progress = true;
        ch->arp_note_offset = param >> 4;
        break;
    case 1:
        ch->arp_in_progress = true;
        ch->arp_note_offset = param & 0x0F;
        break;
    }
    jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_tone_portamento(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch) {
    /* 3xx called without a note, wait until we get an actual target note. */
    if(ch->tone_portamento_target_period == 0.f) return;  /* no value, exit */
    if(ch->period != ch->tone_portamento_target_period) {
        jar_xm_SLIDE_TOWARDS(ch->period, ch->tone_portamento_target_period, (ctx->module.frequency_type == jar_xm_LINEAR_FREQUENCIES ? 4.f : 1.f) * ch->tone_portamento_param);
        jar_xm_update_frequency(ctx, ch);
    }
}

static void jar_xm_pitch_slide(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, float period_offset) {
    /* Don't ask about the 4.f coefficient. I found mention of it nowhere. Found by ear. */
    if(ctx->module.frequency_type == jar_xm_LINEAR_FREQUENCIES) {period_offset *= 4.f; }
    ch->period += period_offset;
    jar_xm_CLAMP_DOWN(ch->period);
    /* XXX: upper bound of period ? */
    jar_xm_update_frequency(ctx, ch);
}

static void jar_xm_panning_slide(jar_xm_channel_context_t* ch, uint8_t rawval) {
    if (rawval & 0xF0) {ch->panning += (float)((rawval & 0xF0 )>> 4) / (float)0xFF;};
    if (rawval & 0x0F) {ch->panning -= (float)(rawval & 0x0F) / (float)0xFF;};
};

static void jar_xm_volume_slide(jar_xm_channel_context_t* ch, uint8_t rawval) {
    if (rawval & 0xF0) {ch->volume += (float)((rawval & 0xF0) >> 4) / (float)0x40;};
    if (rawval & 0x0F) {ch->volume -= (float)(rawval & 0x0F) / (float)0x40;};
};

static float jar_xm_envelope_lerp(jar_xm_envelope_point_t* a, jar_xm_envelope_point_t* b, uint16_t pos) {
    /* Linear interpolation between two envelope points */
    if(pos <= a->frame) return a->value;
    else if(pos >= b->frame) return b->value;
    else {
        float p = (float)(pos - a->frame) / (float)(b->frame - a->frame);
        return a->value * (1 - p) + b->value * p;
    }
}

static void jar_xm_post_pattern_change(jar_xm_context_t* ctx) {
    /* Loop if necessary */
    if(ctx->current_table_index >= ctx->module.length) {
        ctx->current_table_index = ctx->module.restart_position;
        ctx->tempo =ctx->default_tempo; // reset to file default value
        ctx->bpm = ctx->default_bpm; // reset to file default value
        ctx->global_volume = ctx->default_global_volume; // reset to file default value
    }
}

static float jar_xm_linear_period(float note) {
    return 7680.f - note * 64.f;
}

static float jar_xm_linear_frequency(float period) {
    return 8363.f * powf(2.f, (4608.f - period) / 768.f);
}

static float jar_xm_amiga_period(float note) {
    unsigned int intnote = note;
    uint8_t a = intnote % 12;
    int8_t octave = note / 12.f - 2;
    uint16_t p1 = amiga_frequencies[a], p2 = amiga_frequencies[a + 1];
    if(octave > 0) {
        p1 >>= octave;
        p2 >>= octave;
    } else if(octave < 0) {
        p1 <<= -octave;
        p2 <<= -octave;
    }
    return jar_xm_LERP(p1, p2, note - intnote);
}

static float jar_xm_amiga_frequency(float period) {
    if(period == .0f) return .0f;
    return 7093789.2f / (period * 2.f); /* This is the PAL value. (we could use the NTSC value also) */
}

static float jar_xm_period(jar_xm_context_t* ctx, float note) {
    switch(ctx->module.frequency_type) {
    case jar_xm_LINEAR_FREQUENCIES:
        return jar_xm_linear_period(note);
    case jar_xm_AMIGA_FREQUENCIES:
        return jar_xm_amiga_period(note);
    }
    return .0f;
}

static float jar_xm_frequency(jar_xm_context_t* ctx, float period, float note_offset) {
    switch(ctx->module.frequency_type) {
    case jar_xm_LINEAR_FREQUENCIES:
        return jar_xm_linear_frequency(period - 64.f * note_offset);
    case jar_xm_AMIGA_FREQUENCIES:
        if(note_offset == 0) { return jar_xm_amiga_frequency(period); };
        int8_t octave;
        float  note;
        uint16_t p1, p2;
        uint8_t a = octave = 0;

        /* Find the octave of the current period */
        if(period > amiga_frequencies[0]) {
            --octave;
            while(period > (amiga_frequencies[0] << -octave)) --octave;
        } else if(period < amiga_frequencies[12]) {
            ++octave;
            while(period < (amiga_frequencies[12] >> octave)) ++octave;
        }

        /* Find the smallest note closest to the current period */
        for(uint8_t i = 0; i < 12; ++i) {
            p1 = amiga_frequencies[i], p2 = amiga_frequencies[i + 1];
            if(octave > 0) {
                p1 >>= octave;
                p2 >>= octave;
            } else if(octave < 0) {
                p1 <<= (-octave);
                p2 <<= (-octave);
            }
            if(p2 <= period && period <= p1) {
                a = i;
                break;
            }
        }
        if(JAR_XM_DEBUG && (p1 < period || p2 > period)) { DEBUG("%i <= %f <= %i should hold but doesn't, this is a bug", p2, period, p1); }
        note = 12.f * (octave + 2) + a + jar_xm_INVERSE_LERP(p1, p2, period);
        return jar_xm_amiga_frequency(jar_xm_amiga_period(note + note_offset));
    }

    return .0f;
}

static void jar_xm_update_frequency(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch) {
    ch->frequency = jar_xm_frequency( ctx, ch->period, (ch->arp_note_offset > 0 ? ch->arp_note_offset : (  ch->vibrato_note_offset + ch->autovibrato_note_offset ))  );
    ch->step = ch->frequency / ctx->rate;
}

static void jar_xm_handle_note_and_instrument(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, jar_xm_pattern_slot_t* s) {
    jar_xm_module_t* mod = &(ctx->module);
    if(s->instrument > 0) {
        if(HAS_TONE_PORTAMENTO(ch->current) && ch->instrument != NULL && ch->sample != NULL) {  /* Tone portamento in effect */
            jar_xm_trigger_note(ctx, ch, jar_xm_TRIGGER_KEEP_PERIOD | jar_xm_TRIGGER_KEEP_SAMPLE_POSITION);
        } else if(s->instrument > ctx->module.num_instruments) {    /* Invalid instrument, Cut current note */
            jar_xm_cut_note(ch);
            ch->instrument = NULL;
            ch->sample = NULL;
        } else {
            ch->instrument = ctx->module.instruments + (s->instrument - 1);
            if(s->note == 0 && ch->sample != NULL) {  /* Ghost instrument, trigger note */
                /* Sample position is kept, but envelopes are reset */
                jar_xm_trigger_note(ctx, ch, jar_xm_TRIGGER_KEEP_SAMPLE_POSITION);
            }
        }
    }

    if(NOTE_IS_VALID(s->note)) {
        // note value is s->note -1
        jar_xm_instrument_t* instr = ch->instrument;
        if(HAS_TONE_PORTAMENTO(ch->current) && instr != NULL && ch->sample != NULL) {
            /* Tone portamento in effect */
            ch->note = s->note + ch->sample->relative_note + ch->sample->finetune / 128.f - 1.f;
            ch->tone_portamento_target_period = jar_xm_period(ctx, ch->note);
        } else if(instr == NULL || ch->instrument->num_samples == 0) {   /* Issue on instrument */
            jar_xm_cut_note(ch);
        } else {
            if(instr->sample_of_notes[s->note - 1] < instr->num_samples) {
                if (mod->ramping) {
                    for(int i = 0; i < jar_xm_SAMPLE_RAMPING_POINTS; ++i) {
                        jar_xm_next_of_sample(ctx, ch, i);
                    }
                    ch->frame_count = 0;
                };
                ch->sample = instr->samples + instr->sample_of_notes[s->note - 1];
                ch->orig_note = ch->note = s->note + ch->sample->relative_note + ch->sample->finetune / 128.f - 1.f;
                if(s->instrument > 0) {
                    jar_xm_trigger_note(ctx, ch, 0);
                } else {  /* Ghost note: keep old volume */
                    jar_xm_trigger_note(ctx, ch, jar_xm_TRIGGER_KEEP_VOLUME);
                }
            } else {
                jar_xm_cut_note(ch);
            }
        }
    } else if(s->note == NOTE_OFF) {
        jar_xm_key_off(ch);
    }

    // Interpret Effect column
    switch(s->effect_type) {
    case 1: /* 1xx: Portamento up */
        if(s->effect_param > 0) {    ch->portamento_up_param = s->effect_param; }
        break;
    case 2: /* 2xx: Portamento down */
        if(s->effect_param > 0) {    ch->portamento_down_param = s->effect_param; }
        break;
    case 3: /* 3xx: Tone portamento */
        if(s->effect_param > 0) {    ch->tone_portamento_param = s->effect_param; }
        break;
    case 4: /* 4xy: Vibrato */
        if(s->effect_param & 0x0F) { ch->vibrato_param = (ch->vibrato_param & 0xF0) | (s->effect_param & 0x0F); }  /* Set vibrato depth */
        if(s->effect_param >> 4) { ch->vibrato_param = (s->effect_param & 0xF0) | (ch->vibrato_param & 0x0F); }   /* Set vibrato speed */
        break;
    case 5: /* 5xy: Tone portamento + Volume slide */
        if(s->effect_param > 0) {  ch->volume_slide_param = s->effect_param; }
        break;
    case 6: /* 6xy: Vibrato + Volume slide */
        if(s->effect_param > 0) {    ch->volume_slide_param = s->effect_param; }
        break;
    case 7: /* 7xy: Tremolo */
        if(s->effect_param & 0x0F) { ch->tremolo_param = (ch->tremolo_param & 0xF0) | (s->effect_param & 0x0F); } /* Set tremolo depth */
        if(s->effect_param >> 4) { ch->tremolo_param = (s->effect_param & 0xF0) | (ch->tremolo_param & 0x0F); }  /* Set tremolo speed */
        break;
    case 8: /* 8xx: Set panning */
        ch->panning = (float)s->effect_param / 255.f;
        break;
    case 9: /* 9xx: Sample offset */
        if(ch->sample != 0) { //&& NOTE_IS_VALID(s->note)) {
            uint32_t final_offset = s->effect_param << (ch->sample->bits == 16 ? 7 : 8);
            switch (ch->sample->loop_type) {
            case jar_xm_NO_LOOP:
                if(final_offset >= ch->sample->length) { /* Pretend the sample dosen't loop and is done playing */
                    ch->sample_position = -1;
                } else {
                    ch->sample_position = final_offset;
                }
                break;
            case jar_xm_FORWARD_LOOP:
                if (final_offset >= ch->sample->loop_end) {
                    ch->sample_position -= ch->sample->loop_length;
                } else if(final_offset >= ch->sample->length) {
                    ch->sample_position = ch->sample->loop_start;
                } else {
                    ch->sample_position = final_offset;
                }
                break;
            case jar_xm_PING_PONG_LOOP:
                if(final_offset >= ch->sample->loop_end) {
                    ch->ping = false;
                    ch->sample_position = (ch->sample->loop_end << 1) - ch->sample_position;
                } else if(final_offset >= ch->sample->length) {
                    ch->ping = false;
                    ch->sample_position -= ch->sample->length - 1;
                } else {
                    ch->sample_position = final_offset;
                };
                break;
            }
        }
        break;
    case 0xA: /* Axy: Volume slide */
        if(s->effect_param > 0) {    ch->volume_slide_param = s->effect_param; }
        break;
    case 0xB: /* Bxx: Position jump */
        if(s->effect_param < ctx->module.length) {
            ctx->position_jump = true;
            ctx->jump_dest = s->effect_param;
        }
        break;
    case 0xC: /* Cxx: Set volume */
        ch->volume = (float)((s->effect_param > 0x40) ? 0x40 : s->effect_param) / (float)0x40;
        break;
    case 0xD: /* Dxx: Pattern break */
        /* Jump after playing this line */
        ctx->pattern_break = true;
        ctx->jump_row = (s->effect_param >> 4) * 10 + (s->effect_param & 0x0F);
        break;
    case 0xE: /* EXy: Extended command */
        switch(s->effect_param >> 4) {
        case 1: /* E1y: Fine portamento up */
            if(s->effect_param & 0x0F) {    ch->fine_portamento_up_param = s->effect_param & 0x0F; }
            jar_xm_pitch_slide(ctx, ch, -ch->fine_portamento_up_param);
            break;
        case 2: /* E2y: Fine portamento down */
            if(s->effect_param & 0x0F) {    ch->fine_portamento_down_param = s->effect_param & 0x0F; }
            jar_xm_pitch_slide(ctx, ch, ch->fine_portamento_down_param);
            break;
        case 4: /* E4y: Set vibrato control */
            ch->vibrato_waveform = s->effect_param & 3;
            ch->vibrato_waveform_retrigger = !((s->effect_param >> 2) & 1);
            break;
        case 5: /* E5y: Set finetune */
            if(NOTE_IS_VALID(ch->current->note) && ch->sample != NULL) {
                ch->note = ch->current->note + ch->sample->relative_note + (float)(((s->effect_param & 0x0F) - 8) << 4) / 128.f - 1.f;
                ch->period = jar_xm_period(ctx, ch->note);
                jar_xm_update_frequency(ctx, ch);
            }
            break;
        case 6: /* E6y: Pattern loop */
            if(s->effect_param & 0x0F) {
                if((s->effect_param & 0x0F) == ch->pattern_loop_count) {   /* Loop is over */
                    ch->pattern_loop_count = 0;
                    ctx->position_jump = false;
                } else {    /* Jump to the beginning of the loop */
                    ch->pattern_loop_count++;
                    ctx->position_jump = true;
                    ctx->jump_row = ch->pattern_loop_origin;
                    ctx->jump_dest = ctx->current_table_index;
                }
            } else {
                ch->pattern_loop_origin = ctx->current_row; /* Set loop start point */
                ctx->jump_row = ch->pattern_loop_origin;    /* Replicate FT2 E60 bug */
            }
            break;
        case 7: /* E7y: Set tremolo control */
            ch->tremolo_waveform = s->effect_param & 3;
            ch->tremolo_waveform_retrigger = !((s->effect_param >> 2) & 1);
            break;
        case 0xA: /* EAy: Fine volume slide up */
            if(s->effect_param & 0x0F) {   ch->fine_volume_slide_param = s->effect_param & 0x0F; }
            jar_xm_volume_slide(ch, ch->fine_volume_slide_param << 4);
            break;
        case 0xB: /* EBy: Fine volume slide down */
            if(s->effect_param & 0x0F) {   ch->fine_volume_slide_param = s->effect_param & 0x0F; }
            jar_xm_volume_slide(ch, ch->fine_volume_slide_param);
            break;
        case 0xD: /* EDy: Note delay */
            /* XXX: figure this out better. EDx triggers the note even when there no note and no instrument. But ED0 acts like like a ghost note, EDx (x != 0) does not. */
            if(s->note == 0 && s->instrument == 0) {
                unsigned int flags = jar_xm_TRIGGER_KEEP_VOLUME;
                if(ch->current->effect_param & 0x0F) {
                    ch->note = ch->orig_note;
                    jar_xm_trigger_note(ctx, ch, flags);
                } else {
                    jar_xm_trigger_note(ctx, ch, flags | jar_xm_TRIGGER_KEEP_PERIOD | jar_xm_TRIGGER_KEEP_SAMPLE_POSITION );
                }
            }
            break;

        case 0xE: /* EEy: Pattern delay */
            ctx->extra_ticks = (ch->current->effect_param & 0x0F) * ctx->tempo;
            break;
        default:
            break;
        }
        break;

    case 0xF: /* Fxx: Set tempo/BPM */
        if(s->effect_param > 0) {
            if(s->effect_param <= 0x1F) {  // First 32 possible values adjust the ticks (goes into tempo)
                ctx->tempo = s->effect_param;
            } else {                       //32 and greater values adjust the BPM
                ctx->bpm = s->effect_param;
            }
        }
        break;

    case 16: /* Gxx: Set global volume */
        ctx->global_volume = (float)((s->effect_param > 0x40) ? 0x40 : s->effect_param) / (float)0x40;
        break;
    case 17: /* Hxy: Global volume slide */
        if(s->effect_param > 0) {    ch->global_volume_slide_param = s->effect_param; }
        break;
    case 21: /* Lxx: Set envelope position */
        ch->volume_envelope_frame_count = s->effect_param;
        ch->panning_envelope_frame_count = s->effect_param;
        break;
    case 25: /* Pxy: Panning slide */
        if(s->effect_param > 0) {   ch->panning_slide_param = s->effect_param; }
        break;
    case 27: /* Rxy: Multi retrig note */
        if(s->effect_param > 0) {
            if((s->effect_param >> 4) == 0) {    /* Keep previous x value */
                ch->multi_retrig_param = (ch->multi_retrig_param & 0xF0) | (s->effect_param & 0x0F);
            } else {
                ch->multi_retrig_param = s->effect_param;
            }
        }
        break;
    case 29: /* Txy: Tremor */
        if(s->effect_param > 0) { ch->tremor_param = s->effect_param; }  /* Tremor x and y params are not separately kept in memory, unlike Rxy */
        break;
    case 33: /* Xxy: Extra stuff */
        switch(s->effect_param >> 4) {
        case 1: /* X1y: Extra fine portamento up */
            if(s->effect_param & 0x0F) {    ch->extra_fine_portamento_up_param = s->effect_param & 0x0F; }
            jar_xm_pitch_slide(ctx, ch, -1.0f * ch->extra_fine_portamento_up_param);
            break;
        case 2: /* X2y: Extra fine portamento down */
            if(s->effect_param & 0x0F) {   ch->extra_fine_portamento_down_param = s->effect_param & 0x0F; }
            jar_xm_pitch_slide(ctx, ch, ch->extra_fine_portamento_down_param);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static void jar_xm_trigger_note(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, unsigned int flags) {
    if (!(flags & jar_xm_TRIGGER_KEEP_SAMPLE_POSITION)) {
        ch->sample_position = 0.f;
        ch->ping = true;
    };

    if (!(flags & jar_xm_TRIGGER_KEEP_VOLUME)) {
       if(ch->sample != NULL) {
        ch->volume = ch->sample->volume;
        };
    };
    ch->panning = ch->sample->panning;
    ch->sustained = true;
    ch->fadeout_volume = ch->volume_envelope_volume = 1.0f;
    ch->panning_envelope_panning = .5f;
    ch->volume_envelope_frame_count = ch->panning_envelope_frame_count = 0;
    ch->vibrato_note_offset = 0.f;
    ch->tremolo_volume = 0.f;
    ch->tremor_on = false;
    ch->autovibrato_ticks = 0;

    if(ch->vibrato_waveform_retrigger) { ch->vibrato_ticks = 0; } /* XXX: should the waveform itself also be reset to sine? */
    if(ch->tremolo_waveform_retrigger) { ch->tremolo_ticks = 0; }
    if(!(flags & jar_xm_TRIGGER_KEEP_PERIOD)) {
        ch->period = jar_xm_period(ctx, ch->note);
        jar_xm_update_frequency(ctx, ch);
    }
    ch->latest_trigger = ctx->generated_samples;
    if(ch->instrument != NULL) { ch->instrument->latest_trigger = ctx->generated_samples; }
    if(ch->sample != NULL) {     ch->sample->latest_trigger = ctx->generated_samples; }
}

static void jar_xm_cut_note(jar_xm_channel_context_t* ch) {
    ch->volume = .0f; /* NB: this is not the same as Key Off */
//    ch->curr_left = .0f;
//    ch->curr_right = .0f;
}

static void jar_xm_key_off(jar_xm_channel_context_t* ch) {
    ch->sustained = false; /* Key Off */
    if(ch->instrument == NULL || !ch->instrument->volume_envelope.enabled) { jar_xm_cut_note(ch); } /* If no volume envelope is used, also cut the note */
}

static void jar_xm_row(jar_xm_context_t* ctx) {
    if(ctx->position_jump) {
        ctx->current_table_index = ctx->jump_dest;
        ctx->current_row = ctx->jump_row;
        ctx->position_jump = false;
        ctx->pattern_break = false;
        ctx->jump_row = 0;
        jar_xm_post_pattern_change(ctx);
    } else if(ctx->pattern_break) {
        ctx->current_table_index++;
        ctx->current_row = ctx->jump_row;
        ctx->pattern_break = false;
        ctx->jump_row = 0;
        jar_xm_post_pattern_change(ctx);
    }
    jar_xm_pattern_t* cur = ctx->module.patterns + ctx->module.pattern_table[ctx->current_table_index];
    bool in_a_loop = false;

    /* Read notes information for all channels into temporary pattern slot */
    for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
        jar_xm_pattern_slot_t* s = cur->slots + ctx->current_row * ctx->module.num_channels + i;
        jar_xm_channel_context_t* ch = ctx->channels + i;
        ch->current = s;
        // If there is no note delay effect (0xED) then...
        if(s->effect_type != 0xE || s->effect_param >> 4 != 0xD) {
            //********** Process the channel slot information **********
            jar_xm_handle_note_and_instrument(ctx, ch, s);
        } else {
            // read the note delay information
            ch->note_delay_param = s->effect_param & 0x0F;
        }
        if(!in_a_loop && ch->pattern_loop_count > 0) {
            // clarify if in a loop or not
            in_a_loop = true;
        }
    }

    if(!in_a_loop) {
        /* No E6y loop is in effect (or we are in the first pass) */
        ctx->loop_count = (ctx->row_loop_count[MAX_NUM_ROWS * ctx->current_table_index + ctx->current_row]++);
    }

    /// Move to next row
    ctx->current_row++; /* uint8 warning: can increment from 255 to 0, in which case it is still necessary to go the next pattern. */
    if (!ctx->position_jump && !ctx->pattern_break && (ctx->current_row >= cur->num_rows || ctx->current_row == 0)) {
        ctx->current_table_index++;
        ctx->current_row = ctx->jump_row; /* This will be 0 most of the time, except when E60 is used */
        ctx->jump_row = 0;
        jar_xm_post_pattern_change(ctx);
    }
}

static void jar_xm_envelope_tick(jar_xm_channel_context_t *ch, jar_xm_envelope_t *env, uint16_t *counter, float *outval) {
    if(env->num_points < 2) {
        if(env->num_points == 1) {
            *outval = (float)env->points[0].value / (float)0x40;
            if(*outval > 1) { *outval = 1; };
        } else {;
            return;
        };
    } else {
        if(env->loop_enabled) {
            uint16_t loop_start = env->points[env->loop_start_point].frame;
            uint16_t loop_end = env->points[env->loop_end_point].frame;
            uint16_t loop_length = loop_end - loop_start;
            if(*counter >= loop_end) { *counter -= loop_length; };
        };
        for(uint8_t j = 0; j < (env->num_points - 1); ++j) {
            if(env->points[j].frame <= *counter && env->points[j+1].frame >= *counter) {
                *outval = jar_xm_envelope_lerp(env->points + j, env->points + j + 1, *counter) / (float)0x40;
                break;
            };
        };
        /* Make sure it is safe to increment frame count */
        if(!ch->sustained || !env->sustain_enabled || *counter != env->points[env->sustain_point].frame) { (*counter)++; };
    };
};

static void jar_xm_envelopes(jar_xm_channel_context_t *ch) {
    if(ch->instrument != NULL) {
        if(ch->instrument->volume_envelope.enabled) {
            if(!ch->sustained) {
                ch->fadeout_volume -= (float)ch->instrument->volume_fadeout / 65536.f;
                jar_xm_CLAMP_DOWN(ch->fadeout_volume);
            };
            jar_xm_envelope_tick(ch, &(ch->instrument->volume_envelope), &(ch->volume_envelope_frame_count), &(ch->volume_envelope_volume));
        };
        if(ch->instrument->panning_envelope.enabled) {
            jar_xm_envelope_tick(ch, &(ch->instrument->panning_envelope), &(ch->panning_envelope_frame_count), &(ch->panning_envelope_panning));
        };
    };
};

static void jar_xm_tick(jar_xm_context_t* ctx) {
    if(ctx->current_tick == 0) {
        jar_xm_row(ctx);        // We have processed all ticks and we run the row
    }

    jar_xm_module_t* mod = &(ctx->module);
    for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
        jar_xm_channel_context_t* ch = ctx->channels + i;
        jar_xm_envelopes(ch);
        jar_xm_autovibrato(ctx, ch);
        if(ch->arp_in_progress && !HAS_ARPEGGIO(ch->current)) {
            ch->arp_in_progress = false;
            ch->arp_note_offset = 0;
            jar_xm_update_frequency(ctx, ch);
        }
        if(ch->vibrato_in_progress && !HAS_VIBRATO(ch->current)) {
            ch->vibrato_in_progress = false;
            ch->vibrato_note_offset = 0.f;
            jar_xm_update_frequency(ctx, ch);
        }

        // Effects in volumne column mostly handled on a per tick basis
        switch(ch->current->volume_column & 0xF0) {
        case 0x50: // Checks for volume = 64
            if(ch->current->volume_column != 0x50) break;
        case 0x10: // Set volume 0-15
        case 0x20: // Set volume 16-32
        case 0x30: // Set volume 32-48
        case 0x40: // Set volume 48-64
            ch->volume = (float)(ch->current->volume_column - 16) / 64.0f;
            break;
        case 0x60: // Volume slide down
            jar_xm_volume_slide(ch, ch->current->volume_column & 0x0F);
            break;
        case 0x70: // Volume slide up
            jar_xm_volume_slide(ch, ch->current->volume_column << 4);
            break;
        case 0x80: // Fine volume slide down
            jar_xm_volume_slide(ch, ch->current->volume_column & 0x0F);
            break;
        case 0x90: // Fine volume slide up
            jar_xm_volume_slide(ch, ch->current->volume_column << 4);
            break;
        case 0xA0: // Set vibrato speed
            ch->vibrato_param = (ch->vibrato_param & 0x0F) | ((ch->current->volume_column & 0x0F) << 4);
            break;
        case 0xB0: // Vibrato
            ch->vibrato_in_progress = false;
            jar_xm_vibrato(ctx, ch, ch->vibrato_param, ch->vibrato_ticks++);
            break;
        case 0xC0: // Set panning
            if(!ctx->current_tick ) {
                ch->panning = (float)(ch->current->volume_column & 0x0F) / 15.0f;
            }
            break;
        case 0xD0: // Panning slide left
            jar_xm_panning_slide(ch, ch->current->volume_column & 0x0F);
            break;
        case 0xE0: // Panning slide right
            jar_xm_panning_slide(ch, ch->current->volume_column << 4);
            break;
        case 0xF0: // Tone portamento
            if(!ctx->current_tick ) {
                if(ch->current->volume_column & 0x0F) { ch->tone_portamento_param = ((ch->current->volume_column & 0x0F) << 4) | (ch->current->volume_column & 0x0F); }
            };
            jar_xm_tone_portamento(ctx, ch);
            break;
        default:
            break;
        }

        // Only some standard effects handled on a per tick basis
        // see jar_xm_handle_note_and_instrument for all effects handling on a per row basis
        switch(ch->current->effect_type) {
        case 0: /* 0xy: Arpeggio */
            if(ch->current->effect_param > 0) {
                char arp_offset = ctx->tempo % 3;
                switch(arp_offset) {
                case 2: /* 0 -> x -> 0 -> y -> x -> ... */
                    if(ctx->current_tick == 1) {
                        ch->arp_in_progress = true;
                        ch->arp_note_offset = ch->current->effect_param >> 4;
                        jar_xm_update_frequency(ctx, ch);
                        break;
                    }
                    /* No break here, this is intended */
                case 1: /* 0 -> 0 -> y -> x -> ... */
                    if(ctx->current_tick == 0) {
                        ch->arp_in_progress = false;
                        ch->arp_note_offset = 0;
                        jar_xm_update_frequency(ctx, ch);
                        break;
                    }
                    /* No break here, this is intended */
                case 0: /* 0 -> y -> x -> ... */
                    jar_xm_arpeggio(ctx, ch, ch->current->effect_param, ctx->current_tick - arp_offset);
                default:
                    break;
                }
            }
            break;

        case 1: /* 1xx: Portamento up */
            if(ctx->current_tick == 0) break;
            jar_xm_pitch_slide(ctx, ch, -ch->portamento_up_param);
            break;
        case 2: /* 2xx: Portamento down */
            if(ctx->current_tick == 0) break;
            jar_xm_pitch_slide(ctx, ch, ch->portamento_down_param);
            break;
        case 3: /* 3xx: Tone portamento */
            if(ctx->current_tick == 0) break;
            jar_xm_tone_portamento(ctx, ch);
            break;
        case 4: /* 4xy: Vibrato */
            if(ctx->current_tick == 0) break;
            ch->vibrato_in_progress = true;
            jar_xm_vibrato(ctx, ch, ch->vibrato_param, ch->vibrato_ticks++);
            break;
        case 5: /* 5xy: Tone portamento + Volume slide */
            if(ctx->current_tick == 0) break;
            jar_xm_tone_portamento(ctx, ch);
            jar_xm_volume_slide(ch, ch->volume_slide_param);
            break;
        case 6: /* 6xy: Vibrato + Volume slide */
            if(ctx->current_tick == 0) break;
            ch->vibrato_in_progress = true;
            jar_xm_vibrato(ctx, ch, ch->vibrato_param, ch->vibrato_ticks++);
            jar_xm_volume_slide(ch, ch->volume_slide_param);
            break;
        case 7: /* 7xy: Tremolo */
            if(ctx->current_tick == 0) break;
            jar_xm_tremolo(ctx, ch, ch->tremolo_param, ch->tremolo_ticks++);
            break;
        case 8: /* 8xy: Set panning */
            break;
        case 9: /* 9xy: Sample offset */
            break;
        case 0xA: /* Axy: Volume slide */
            if(ctx->current_tick == 0) break;
            jar_xm_volume_slide(ch, ch->volume_slide_param);
            break;
        case 0xE: /* EXy: Extended command */
            switch(ch->current->effect_param >> 4) {
            case 0x9: /* E9y: Retrigger note */
                if(ctx->current_tick != 0 && ch->current->effect_param & 0x0F) {
                    if(!(ctx->current_tick % (ch->current->effect_param & 0x0F))) {
                        jar_xm_trigger_note(ctx, ch, 0);
                        jar_xm_envelopes(ch);
                    }
                }
                break;
            case 0xC: /* ECy: Note cut */
                if((ch->current->effect_param & 0x0F) == ctx->current_tick) {
                    jar_xm_cut_note(ch);
                }
                break;
            case 0xD: /* EDy: Note delay */
                if(ch->note_delay_param == ctx->current_tick) {
                    jar_xm_handle_note_and_instrument(ctx, ch, ch->current);
                    jar_xm_envelopes(ch);
                }
                break;
            default:
                break;
            }
            break;
        case 16: /* Fxy: Set tempo/BPM */
            break;
        case 17: /* Hxy: Global volume slide */
            if(ctx->current_tick == 0) break;
            if((ch->global_volume_slide_param & 0xF0) && (ch->global_volume_slide_param & 0x0F)) { break; }; /* Invalid state */
            if(ch->global_volume_slide_param & 0xF0) {    /* Global slide up */
                float f = (float)(ch->global_volume_slide_param >> 4) / (float)0x40;
                ctx->global_volume += f;
                jar_xm_CLAMP_UP(ctx->global_volume);
            } else {                                      /* Global slide down */
                float f = (float)(ch->global_volume_slide_param & 0x0F) / (float)0x40;
                ctx->global_volume -= f;
                jar_xm_CLAMP_DOWN(ctx->global_volume);
            };
            break;

        case 20: /* Kxx: Key off */
            if(ctx->current_tick == ch->current->effect_param) {     jar_xm_key_off(ch); };
            break;
        case 21: /* Lxx: Set envelope position */
            break;
        case 25: /* Pxy: Panning slide */
            if(ctx->current_tick == 0) break;
            jar_xm_panning_slide(ch, ch->panning_slide_param);
            break;
        case 27: /* Rxy: Multi retrig note */
            if(ctx->current_tick == 0) break;
            if(((ch->multi_retrig_param) & 0x0F) == 0) break;
            if((ctx->current_tick % (ch->multi_retrig_param & 0x0F)) == 0) {
                float v = ch->volume * multi_retrig_multiply[ch->multi_retrig_param >> 4]
                    + multi_retrig_add[ch->multi_retrig_param >> 4];
                jar_xm_CLAMP(v);
                jar_xm_trigger_note(ctx, ch, 0);
                ch->volume = v;
            };
            break;

        case 29: /* Txy: Tremor */
            if(ctx->current_tick == 0) break;
            ch->tremor_on = ( (ctx->current_tick - 1) % ((ch->tremor_param >> 4) + (ch->tremor_param & 0x0F) + 2) > (ch->tremor_param >> 4)  );
            break;
        default:
            break;
        };

        float panning, volume;
        panning = ch->panning + (ch->panning_envelope_panning - .5f) * (.5f - fabs(ch->panning - .5f)) * 2.0f;
        if(ch->tremor_on) {
            volume = .0f;
        } else {
            volume = ch->volume + ch->tremolo_volume;
            jar_xm_CLAMP(volume);
            volume *= ch->fadeout_volume * ch->volume_envelope_volume;
        };

        if (mod->ramping) {
            ch->target_panning = panning;
            ch->target_volume = volume;
        } else {
            ch->actual_panning = panning;
            ch->actual_volume = volume;
        };
    };

    ctx->current_tick++; // ok so we understand that ticks increment within the row
    if(ctx->current_tick >= ctx->tempo + ctx->extra_ticks) {
        // This means it reached the end of the row and we reset
        ctx->current_tick = 0;
        ctx->extra_ticks = 0;
    };

    // Number of ticks / second = BPM * 0.4
    ctx->remaining_samples_in_tick += (float)ctx->rate / ((float)ctx->bpm * 0.4f);
};

static void jar_xm_next_of_sample(jar_xm_context_t* ctx, jar_xm_channel_context_t* ch, int previous) {
    jar_xm_module_t* mod = &(ctx->module);

//    ch->curr_left = 0.f;
//    ch->curr_right = 0.f;
    if(ch->instrument == NULL || ch->sample == NULL || ch->sample_position < 0) {
        ch->curr_left = 0.f;
        ch->curr_right = 0.f;
        if (mod->ramping) {
            if (ch->frame_count < jar_xm_SAMPLE_RAMPING_POINTS) {
                if (previous > -1) {
                    ch->end_of_previous_sample_left[previous] = jar_xm_LERP(ch->end_of_previous_sample_left[ch->frame_count], ch->curr_left, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
                    ch->end_of_previous_sample_right[previous] = jar_xm_LERP(ch->end_of_previous_sample_right[ch->frame_count], ch->curr_right, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
                } else {
                    ch->curr_left = jar_xm_LERP(ch->end_of_previous_sample_left[ch->frame_count], ch->curr_left, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
                    ch->curr_right = jar_xm_LERP(ch->end_of_previous_sample_right[ch->frame_count], ch->curr_right, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
                };
            };
        };
        return;
    };
    if(ch->sample->length == 0) {
        return;
    };

    float t = 0.f;
    uint32_t b = 0;
    if(mod->linear_interpolation) {
        b = ch->sample_position + 1;
        t = ch->sample_position - (uint32_t)ch->sample_position; /* Cheaper than fmodf(., 1.f) */
    };

    float u_left, u_right;
    u_left = ch->sample->data[(uint32_t)ch->sample_position];
    if (ch->sample->stereo) {
        u_right = ch->sample->data[(uint32_t)ch->sample_position + ch->sample->length];
    } else {
        u_right = u_left;
    };
    float v_left = 0.f, v_right = 0.f;
    switch(ch->sample->loop_type) {
    case jar_xm_NO_LOOP:
        if(mod->linear_interpolation) {
            v_left = (b < ch->sample->length) ? ch->sample->data[b] : .0f;
            if (ch->sample->stereo) {
                v_right = (b < ch->sample->length) ? ch->sample->data[b + ch->sample->length] : .0f;
            } else {
                v_right = v_left;
            };
        };
        ch->sample_position += ch->step;
        if(ch->sample_position >= ch->sample->length) { ch->sample_position = -1; } // stop playing this sample
        break;
    case jar_xm_FORWARD_LOOP:
        if(mod->linear_interpolation) {
            v_left = ch->sample->data[ (b == ch->sample->loop_end) ? ch->sample->loop_start : b ];
            if (ch->sample->stereo) {
                v_right = ch->sample->data[ (b == ch->sample->loop_end) ? ch->sample->loop_start + ch->sample->length : b + ch->sample->length];
            } else {
                v_right = v_left;
            };
        };
        ch->sample_position += ch->step;
        if (ch->sample_position >= ch->sample->loop_end) {
            ch->sample_position -= ch->sample->loop_length;
        };
        if(ch->sample_position >= ch->sample->length) {
            ch->sample_position = ch->sample->loop_start;
        };
        break;
    case jar_xm_PING_PONG_LOOP:
        if(ch->ping) {
            if(mod->linear_interpolation) {
                v_left = (b >= ch->sample->loop_end) ? ch->sample->data[(uint32_t)ch->sample_position] : ch->sample->data[b];
                if (ch->sample->stereo) {
                    v_right = (b >= ch->sample->loop_end) ? ch->sample->data[(uint32_t)ch->sample_position + ch->sample->length] : ch->sample->data[b + ch->sample->length];
                } else {
                    v_right = v_left;
                };
            };
            ch->sample_position += ch->step;
            if(ch->sample_position >= ch->sample->loop_end) {
                ch->ping = false;
                ch->sample_position = (ch->sample->loop_end << 1) - ch->sample_position;
            };
            if(ch->sample_position >= ch->sample->length) {
                ch->ping = false;
                ch->sample_position -= ch->sample->length - 1;
            };
        } else {
            if(mod->linear_interpolation) {
                v_left = u_left;
                v_right = u_right;
                u_left = (b == 1 || b - 2 <= ch->sample->loop_start) ? ch->sample->data[(uint32_t)ch->sample_position] : ch->sample->data[b - 2];
                if (ch->sample->stereo) {
                    u_right = (b == 1 || b - 2 <= ch->sample->loop_start) ? ch->sample->data[(uint32_t)ch->sample_position + ch->sample->length] : ch->sample->data[b + ch->sample->length - 2];
                } else {
                    u_right = u_left;
                };
            };
            ch->sample_position -= ch->step;
            if(ch->sample_position <= ch->sample->loop_start) {
                ch->ping = true;
                ch->sample_position = (ch->sample->loop_start << 1) - ch->sample_position;
            };
            if (ch->sample_position <= .0f) {
                ch->ping = true;
                ch->sample_position = .0f;
            };
        };
        break;

    default:
        v_left = .0f;
        v_right = .0f;
        break;
    };

    float endval_left = mod->linear_interpolation ? jar_xm_LERP(u_left, v_left, t) : u_left;
    float endval_right = mod->linear_interpolation ? jar_xm_LERP(u_right, v_right, t) : u_right;

    if (mod->ramping) {
        if(ch->frame_count < jar_xm_SAMPLE_RAMPING_POINTS) {
            /* Smoothly transition between old and new sample. */
            if (previous > -1) {
                ch->end_of_previous_sample_left[previous] = jar_xm_LERP(ch->end_of_previous_sample_left[ch->frame_count], endval_left, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
                ch->end_of_previous_sample_right[previous] = jar_xm_LERP(ch->end_of_previous_sample_right[ch->frame_count], endval_right, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
            } else {
                ch->curr_left = jar_xm_LERP(ch->end_of_previous_sample_left[ch->frame_count], endval_left, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
                ch->curr_right = jar_xm_LERP(ch->end_of_previous_sample_right[ch->frame_count], endval_right, (float)ch->frame_count / (float)jar_xm_SAMPLE_RAMPING_POINTS);
            };
        };
    };

    if (previous > -1) {
        ch->end_of_previous_sample_left[previous] = endval_left;
        ch->end_of_previous_sample_right[previous] = endval_right;
    } else {
        ch->curr_left = endval_left;
        ch->curr_right = endval_right;
    };
};

// gather all channel audio into stereo float
static void jar_xm_mixdown(jar_xm_context_t* ctx, float* left, float* right) {
    jar_xm_module_t* mod = &(ctx->module);

    if(ctx->remaining_samples_in_tick <= 0) {
        jar_xm_tick(ctx);
    };
    ctx->remaining_samples_in_tick--;
    *left = 0.f;
    *right = 0.f;
    if(ctx->max_loop_count > 0 && ctx->loop_count > ctx->max_loop_count) { return; }

    for(uint8_t i = 0; i < ctx->module.num_channels; ++i) {
        jar_xm_channel_context_t* ch = ctx->channels + i;
        if(ch->instrument != NULL && ch->sample != NULL && ch->sample_position >= 0) {
            jar_xm_next_of_sample(ctx, ch, -1);
            if(!ch->muted && !ch->instrument->muted) {
                *left  += ch->curr_left * ch->actual_volume * (1.f - ch->actual_panning);
                *right += ch->curr_right * ch->actual_volume * ch->actual_panning;
            };

            if (mod->ramping) {
                ch->frame_count++;
                jar_xm_SLIDE_TOWARDS(ch->actual_volume, ch->target_volume, ctx->volume_ramp);
                jar_xm_SLIDE_TOWARDS(ch->actual_panning, ch->target_panning, ctx->panning_ramp);
            };
        };
    };
    if (ctx->global_volume != 1.0f) {
        *left *= ctx->global_volume;
        *right *= ctx->global_volume;
    };

    // experimental
//    float counter = (float)ctx->generated_samples * 0.0001f
//    *left = tan(&left + sin(counter));
//    *right = tan(&right + cos(counter));

    // apply brick wall limiter when audio goes beyond bounderies
    if(*left < -1.0)  {*left = -1.0;}  else if(*left > 1.0)  {*left = 1.0;};
    if(*right < -1.0) {*right = -1.0;} else if(*right > 1.0) {*right = 1.0;};
};

void jar_xm_generate_samples(jar_xm_context_t* ctx, float* output, size_t numsamples) {
    if(ctx && output) {
        ctx->generated_samples += numsamples;
        for(size_t i = 0; i < numsamples; i++) {
            jar_xm_mixdown(ctx, output + (2 * i), output + (2 * i + 1));
        };
    };
};

uint64_t jar_xm_get_remaining_samples(jar_xm_context_t* ctx) {
    uint64_t total = 0;
    uint8_t currentLoopCount = jar_xm_get_loop_count(ctx);
    jar_xm_set_max_loop_count(ctx, 0);
    while(jar_xm_get_loop_count(ctx) == currentLoopCount) {
        total += ctx->remaining_samples_in_tick;
        ctx->remaining_samples_in_tick = 0;
        jar_xm_tick(ctx);
    }
    ctx->loop_count = currentLoopCount;
    return total;
}

//--------------------------------------------
//FILE LOADER - TODO - NEEDS TO BE CLEANED UP
//--------------------------------------------
#undef DEBUG
#define DEBUG(...) do {      \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr); \
    } while(0)

#define DEBUG_ERR(...) do {      \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr); \
    } while(0)

#define FATAL(...) do {      \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr); \
        exit(1); \
    } while(0)

#define FATAL_ERR(...) do {      \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr); \
        exit(1); \
    } while(0)


int jar_xm_create_context_from_file(jar_xm_context_t** ctx, uint32_t rate, const char* filename) {
    FILE* xmf;
    int size;
    int ret;

    xmf = fopen(filename, "rb");
    if(xmf == NULL) {
        DEBUG_ERR("Could not open input file");
        *ctx = NULL;
        return 3;
    }

    fseek(xmf, 0, SEEK_END);
    size = ftell(xmf);
    rewind(xmf);
    if(size == -1) {
        fclose(xmf);
        DEBUG_ERR("fseek() failed");
        *ctx = NULL;
        return 4;
    }

    char* data = JARXM_MALLOC(size + 1);
    if(!data || fread(data, 1, size, xmf) < size) {
        fclose(xmf);
        DEBUG_ERR(data ? "fread() failed" : "JARXM_MALLOC() failed");
        JARXM_FREE(data);
        *ctx = NULL;
        return 5;
    }

    fclose(xmf);

    ret = jar_xm_create_context_safe(ctx, data, size, rate);
    JARXM_FREE(data);

    switch(ret) {
    case 0:
        break;
    case 1:        DEBUG("could not create context: module is not sane\n");
        *ctx = NULL;
        return 1;
        break;
    case 2:        FATAL("could not create context: malloc failed\n");
        return 2;
        break;
    default:       FATAL("could not create context: unknown error\n");
        return 6;
        break;
    }

    return 0;
}

// not part of the original library
void jar_xm_reset(jar_xm_context_t* ctx) {
    for (uint16_t i = 0; i < jar_xm_get_number_of_channels(ctx); i++) {
        jar_xm_cut_note(&ctx->channels[i]);
    }
    ctx->generated_samples = 0;
    ctx->current_row = 0;
    ctx->current_table_index = 0;
    ctx->current_tick = 0;
    ctx->tempo =ctx->default_tempo; // reset to file default value
    ctx->bpm = ctx->default_bpm; // reset to file default value
    ctx->global_volume = ctx->default_global_volume; // reset to file default value
}


void jar_xm_flip_linear_interpolation(jar_xm_context_t* ctx) {
    if (ctx->module.linear_interpolation) {
        ctx->module.linear_interpolation = 0;
    } else {
        ctx->module.linear_interpolation = 1;
    }
}

void jar_xm_table_jump(jar_xm_context_t* ctx, int table_ptr) {
    for (uint16_t i = 0; i < jar_xm_get_number_of_channels(ctx); i++) {
        jar_xm_cut_note(&ctx->channels[i]);
    }
    ctx->current_row = 0;
    ctx->current_tick = 0;
    if(table_ptr > 0 && table_ptr < ctx->module.length) {
        ctx->current_table_index = table_ptr;
        ctx->module.restart_position = table_ptr; // The reason to jump is to start a new loop or track
    } else {
        ctx->current_table_index = 0;
        ctx->module.restart_position = 0; // The reason to jump is to start a new loop or track
        ctx->tempo =ctx->default_tempo; // reset to file default value
        ctx->bpm = ctx->default_bpm; // reset to file default value
        ctx->global_volume = ctx->default_global_volume; // reset to file default value
    };
}


// TRANSLATE NOTE NUMBER INTO USER VALUE (ie. 1 = C-1, 2 = C#1, 3 = D-1 ... )
const char* xm_note_chr(int number) {
    if (number == NOTE_OFF) {
        return "==";
    };
    number = number % 12;
    switch(number) {
    case 1: return "C-";
    case 2: return "C#";
    case 3: return "D-";
    case 4: return "D#";
    case 5: return "E-";
    case 6: return "F-";
    case 7: return "F#";
    case 8: return "G-";
    case 9: return "G#";
    case 10: return "A-";
    case 11: return "A#";
    case 12: return "B-";
    };
    return "??";
};

const char* xm_octave_chr(int number) {
    if (number == NOTE_OFF) {
        return "=";
    };

    int number2 = number - number % 12;
    int result = floor(number2 / 12) + 1;
    switch(result) {
    case 1: return "1";
    case 2: return "2";
    case 3: return "3";
    case 4: return "4";
    case 5: return "5";
    case 6: return "6";
    case 7: return "7";
    case 8: return "8";
    default: return "?"; /* UNKNOWN */
    };

};

// TRANSLATE NOTE EFFECT CODE INTO USER VALUE
const char* xm_effect_chr(int fx) {
    switch(fx) {
    case 0: return "0";  /* ZERO = NO EFFECT */
    case 1: return "1";  /* 1xx: Portamento up */
    case 2: return "2";  /* 2xx: Portamento down */
    case 3: return "3";  /* 3xx: Tone portamento */
    case 4: return "4";  /* 4xy: Vibrato */
    case 5: return "5";  /* 5xy: Tone portamento + Volume slide */
    case 6: return "6";  /* 6xy: Vibrato + Volume slide */
    case 7: return "7";  /* 7xy: Tremolo */
    case 8: return "8";  /* 8xx: Set panning */
    case 9: return "9";  /* 9xx: Sample offset */
    case 0xA: return "A";/* Axy: Volume slide */
    case 0xB: return "B";/* Bxx: Position jump */
    case 0xC: return "C";/* Cxx: Set volume */
    case 0xD: return "D";/* Dxx: Pattern break */
    case 0xE: return "E";/* EXy: Extended command */
    case 0xF: return "F";/* Fxx: Set tempo/BPM */
    case 16: return "G"; /* Gxx: Set global volume */
    case 17: return "H"; /* Hxy: Global volume slide */
    case 21: return "L"; /* Lxx: Set envelope position */
    case 25: return "P"; /* Pxy: Panning slide */
    case 27: return "R"; /* Rxy: Multi retrig note */
    case 29: return "T"; /* Txy: Tremor */
    case 33: return "X"; /* Xxy: Extra stuff */
    default: return "?"; /* UNKNOWN */
    };
}

#ifdef JAR_XM_RAYLIB

#include "raylib.h" // Need RayLib API calls for the DEBUG display

void jar_xm_debug(jar_xm_context_t *ctx) {
    int size=40;
    int x = 0, y = 0;

    // DEBUG VARIABLES
    y += size; DrawText(TextFormat("CUR TBL = %i", ctx->current_table_index),       x, y, size, WHITE);
    y += size; DrawText(TextFormat("CUR PAT = %i", ctx->module.pattern_table[ctx->current_table_index]),   x, y, size, WHITE);
    y += size; DrawText(TextFormat("POS JMP = %d", ctx->position_jump),             x, y, size, WHITE);
    y += size; DrawText(TextFormat("JMP DST = %i", ctx->jump_dest),                 x, y, size, WHITE);
    y += size; DrawText(TextFormat("PTN BRK = %d", ctx->pattern_break),             x, y, size, WHITE);
    y += size; DrawText(TextFormat("CUR ROW = %i", ctx->current_row),               x, y, size, WHITE);
    y += size; DrawText(TextFormat("JMP ROW = %i", ctx->jump_row),                  x, y, size, WHITE);
    y += size; DrawText(TextFormat("ROW LCT = %i", ctx->row_loop_count),            x, y, size, WHITE);
    y += size; DrawText(TextFormat("LCT     = %i", ctx->loop_count),                x, y, size, WHITE);
    y += size; DrawText(TextFormat("MAX LCT = %i", ctx->max_loop_count),            x, y, size, WHITE);
    x = size * 12; y = 0;

    y += size; DrawText(TextFormat("CUR TCK = %i", ctx->current_tick),              x, y, size, WHITE);
    y += size; DrawText(TextFormat("XTR TCK = %i", ctx->extra_ticks),               x, y, size, WHITE);
    y += size; DrawText(TextFormat("TCK/ROW = %i", ctx->tempo),                     x, y, size, ORANGE);
    y += size; DrawText(TextFormat("SPL TCK = %f", ctx->remaining_samples_in_tick), x, y, size, WHITE);
    y += size; DrawText(TextFormat("GEN SPL = %i", ctx->generated_samples),         x, y, size, WHITE);
    y += size * 7;

    x = 0;
    size=16;
    // TIMELINE OF MODULE
    for (int i=0; i < ctx->module.length; i++) {
        if (i == ctx->jump_dest) {
            if (ctx->position_jump) {
                DrawRectangle(i * size * 2, y - size, size * 2, size, GOLD);
            } else {
                DrawRectangle(i * size * 2, y - size, size * 2, size, BROWN);
            };
        };
        if (i == ctx->current_table_index) {
//            DrawText(TextFormat("%02X", ctx->current_tick), i * size * 2, y - size, size, WHITE);
            DrawRectangle(i * size * 2, y, size * 2, size, RED);
            DrawText(TextFormat("%02X", ctx->current_row), i * size * 2, y - size, size, YELLOW);
        } else {
            DrawRectangle(i * size * 2, y, size * 2, size, ORANGE);
        };
        DrawText(TextFormat("%02X", ctx->module.pattern_table[i]), i * size * 2, y, size, WHITE);
    };
    y += size;

    jar_xm_pattern_t* cur = ctx->module.patterns + ctx->module.pattern_table[ctx->current_table_index];

    /* DISPLAY CURRENTLY PLAYING PATTERN */

    x += 2 * size;
    for(uint8_t i = 0; i < ctx->module.num_channels; i++) {
        DrawRectangle(x, y, 8 * size, size, PURPLE);
        DrawText("N", x, y, size, YELLOW);
        DrawText("I", x + size * 2, y, size, YELLOW);
        DrawText("V", x + size * 4, y, size, YELLOW);
        DrawText("FX", x + size * 6, y, size, YELLOW);
        x += 9 * size;
    };
    x += size;
    for (int j=(ctx->current_row - 14); j<(ctx->current_row + 15); j++) {
        y += size;
        x = 0;
        if (j >=0 && j < (cur->num_rows)) {
            DrawRectangle(x, y, size * 2, size, BROWN);
            DrawText(TextFormat("%02X",j), x, y, size, WHITE);
            x += 2 * size;
            for(uint8_t i = 0; i < ctx->module.num_channels; i++) {
                if (j==(ctx->current_row)) {
                    DrawRectangle(x, y, 8 * size, size, DARKGREEN);
                } else {
                    DrawRectangle(x, y, 8 * size, size, DARKGRAY);
                };
                jar_xm_pattern_slot_t *s = cur->slots + j * ctx->module.num_channels + i;
           //     jar_xm_channel_context_t *ch = ctx->channels + i;
                if (s->note > 0) {DrawText(TextFormat("%s%s", xm_note_chr(s->note), xm_octave_chr(s->note) ), x, y, size, WHITE);} else {DrawText("...", x, y, size, GRAY);};
                if (s->instrument > 0) {
                    DrawText(TextFormat("%02X", s->instrument), x + size * 2, y, size, WHITE);
                    if (s->volume_column == 0) {
                        DrawText(TextFormat("%02X", 64), x + size * 4, y, size, YELLOW);
                    };
                } else {
                    DrawText("..", x + size * 2, y, size, GRAY);
                    if (s->volume_column == 0) {
                        DrawText("..", x + size * 4, y, size, GRAY);
                    };
                };
                if (s->volume_column > 0) {DrawText(TextFormat("%02X", (s->volume_column - 16)), x + size * 4, y, size, WHITE);};
                if (s->effect_type > 0 || s->effect_param > 0) {DrawText(TextFormat("%s%02X", xm_effect_chr(s->effect_type), s->effect_param), x + size * 6, y, size, WHITE);};
                x += 9 * size;
            };
        };
    };

}
#endif // RayLib extension

#endif//end of JAR_XM_IMPLEMENTATION
//-------------------------------------------------------------------------------

#endif//end of INCLUDE_JAR_XM_H
