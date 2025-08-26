// jar_mod.h - v0.01 - public domain C0 - Joshua Reisenauer
//
// HISTORY:
//
//   v0.01  2016-03-12  Setup
//
//
// USAGE:
//
// In ONE source file, put:
//
//    #define JAR_MOD_IMPLEMENTATION
//    #include "jar_mod.h"
//
// Other source files should just include jar_mod.h
//
// SAMPLE CODE:
// jar_mod_context_t modctx;
// short samplebuff[4096];
// bool bufferFull = false;
// int intro_load(void)
// {
//     jar_mod_init(&modctx);
//     jar_mod_load_file(&modctx, "file.mod");
//     return 1;
// }
// int intro_unload(void)
// {
//     jar_mod_unload(&modctx);
//     return 1;
// }
// int intro_tick(long counter)
// {
//     if(!bufferFull)
//     {
//         jar_mod_fillbuffer(&modctx, samplebuff, 4096, 0);
//         bufferFull=true;
//     }
//     if(IsKeyDown(KEY_ENTER))
//         return 1;
//     return 0;
// }
//
//
// LISCENSE:
//
// Written by: Jean-François DEL NERO (http://hxc2001.com/) <Email : jeanfrancoisdelnero <> free.fr>
// Adapted to jar_mod by: Joshua Adam Reisenauer <kd7tck@gmail.com>
// This program is free software. It comes without any warranty, to the
// extent permitted by applicable law. You can redistribute it and/or
// modify it under the terms of the Do What The Fuck You Want To Public
// License, Version 2, as published by Sam Hocevar. See
// http://sam.zoy.org/wtfpl/COPYING for more details.
///////////////////////////////////////////////////////////////////////////////////
// HxCMOD Core API:
// -------------------------------------------
// int  jar_mod_init(jar_mod_context_t * modctx)
//
// - Initialize the jar_mod_context_t buffer. Must be called before doing anything else.
//   Return 1 if success. 0 in case of error.
// -------------------------------------------
// mulong jar_mod_load_file(jar_mod_context_t * modctx, const char* filename)
//
// - "Load" a MOD from file, context must already be initialized.
//   Return size of file in bytes.
// -------------------------------------------
// void jar_mod_fillbuffer( jar_mod_context_t * modctx, short * outbuffer, unsigned long nbsample, jar_mod_tracker_buffer_state * trkbuf )
//
// - Generate and return the next samples chunk to outbuffer.
//   nbsample specify the number of stereo 16bits samples you want.
//   The output format is by default signed 48000Hz 16-bit Stereo PCM samples, otherwise it is changed with jar_mod_setcfg().
//   The output buffer size in bytes must be equal to ( nbsample * 2 * channels ).
//   The optional trkbuf parameter can be used to get detailed status of the player. Put NULL/0 is unused.
// -------------------------------------------
// void jar_mod_unload( jar_mod_context_t * modctx )
// - "Unload" / clear the player status.
// -------------------------------------------
///////////////////////////////////////////////////////////////////////////////////


#ifndef INCLUDE_JAR_MOD_H
#define INCLUDE_JAR_MOD_H

// Allow custom memory allocators
#ifndef JARMOD_MALLOC
    #define JARMOD_MALLOC(sz)    malloc(sz)
#endif
#ifndef JARMOD_FREE
    #define JARMOD_FREE(p)       free(p)
#endif


// Basic type
typedef unsigned char muchar;
typedef unsigned short muint;
typedef short mint;
typedef unsigned long mulong;

#define NUMMAXCHANNELS 32
#define MAXNOTES 12*12
#define DEFAULT_SAMPLE_RATE 48000
//
// MOD file structures
//

#pragma pack(1)

typedef struct {
    muchar  name[22];
    muint   length;
    muchar  finetune;
    muchar  volume;
    muint   reppnt;
    muint   replen;
} sample;

typedef struct {
    muchar  sampperiod;
    muchar  period;
    muchar  sampeffect;
    muchar  effect;
} note;

typedef struct {
    muchar  title[20];
    sample  samples[31];
    muchar  length; // length of tablepos
    muchar  protracker;
    muchar  patterntable[128];
    muchar  signature[4];
    muchar  speed;
} module;

#pragma pack()

//
// HxCMod Internal structures
//
typedef struct {
    char*   sampdata;
    muint   sampnum;
    muint   length;
    muint   reppnt;
    muint   replen;
    mulong  samppos;
    muint   period;
    muchar  volume;
    mulong  ticks;
    muchar  effect;
    muchar  parameffect;
    muint   effect_code;
    mint    decalperiod;
    mint    portaspeed;
    mint    portaperiod;
    mint    vibraperiod;
    mint    Arpperiods[3];
    muchar  ArpIndex;
    mint    oldk;
    muchar  volumeslide;
    muchar  vibraparam;
    muchar  vibrapointeur;
    muchar  finetune;
    muchar  cut_param;
    muint   patternloopcnt;
    muint   patternloopstartpoint;
} channel;

typedef struct {
    module  song;
    char*   sampledata[31];
    note*   patterndata[128];

    mulong  playrate;
    muint   tablepos;
    muint   patternpos;
    muint   patterndelay;
    muint   jump_loop_effect;
    muchar  bpm;
    mulong  patternticks;
    mulong  patterntickse;
    mulong  patternticksaim;
    mulong  sampleticksconst;
    mulong  samplenb;
    channel channels[NUMMAXCHANNELS];
    muint   number_of_channels;
    muint   fullperiod[MAXNOTES * 8];
    muint   mod_loaded;
    mint    last_r_sample;
    mint    last_l_sample;
    mint    stereo;
    mint    stereo_separation;
    mint    bits;
    mint    filter;
    
    muchar *modfile; // the raw mod file
    mulong  modfilesize;
    muint   loopcount;
} jar_mod_context_t;

//
// Player states structures
//
typedef struct track_state_
{
    unsigned char instrument_number;
    unsigned short cur_period;
    unsigned char  cur_volume;
    unsigned short cur_effect;
    unsigned short cur_parameffect;
}track_state;

typedef struct tracker_state_
{
    int number_of_tracks;
    int bpm;
    int speed;
    int cur_pattern;
    int cur_pattern_pos;
    int cur_pattern_table_pos;
    unsigned int buf_index;
    track_state tracks[32];
}tracker_state;

typedef struct tracker_state_instrument_
{
    char name[22];
    int  active;
}tracker_state_instrument;

typedef struct jar_mod_tracker_buffer_state_
{
    int  nb_max_of_state;
    int  nb_of_state;
    int  cur_rd_index;
    int  sample_step;
    char name[64];
    tracker_state_instrument instruments[31];
    tracker_state * track_state_buf;
}jar_mod_tracker_buffer_state;

#ifdef __cplusplus
extern "C" {
#endif

bool   jar_mod_init(jar_mod_context_t * modctx);
bool   jar_mod_setcfg(jar_mod_context_t * modctx, int samplerate, int bits, int stereo, int stereo_separation, int filter);
void   jar_mod_fillbuffer(jar_mod_context_t * modctx, short * outbuffer, unsigned long nbsample, jar_mod_tracker_buffer_state * trkbuf);
void   jar_mod_unload(jar_mod_context_t * modctx);
mulong jar_mod_load_file(jar_mod_context_t * modctx, const char* filename);
mulong jar_mod_current_samples(jar_mod_context_t * modctx);
mulong jar_mod_max_samples(jar_mod_context_t * modctx);
void   jar_mod_seek_start(jar_mod_context_t * ctx);

#ifdef __cplusplus
}
#endif
//--------------------------------------------------------------------



//-------------------------------------------------------------------------------
#ifdef JAR_MOD_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
//#include <stdbool.h>

// Effects list
#define EFFECT_ARPEGGIO              0x0 // Supported
#define EFFECT_PORTAMENTO_UP         0x1 // Supported
#define EFFECT_PORTAMENTO_DOWN       0x2 // Supported
#define EFFECT_TONE_PORTAMENTO       0x3 // Supported
#define EFFECT_VIBRATO               0x4 // Supported
#define EFFECT_VOLSLIDE_TONEPORTA    0x5 // Supported
#define EFFECT_VOLSLIDE_VIBRATO      0x6 // Supported
#define EFFECT_VOLSLIDE_TREMOLO      0x7 // - TO BE DONE -
#define EFFECT_SET_PANNING           0x8 // - TO BE DONE -
#define EFFECT_SET_OFFSET            0x9 // Supported
#define EFFECT_VOLUME_SLIDE          0xA // Supported
#define EFFECT_JUMP_POSITION         0xB // Supported
#define EFFECT_SET_VOLUME            0xC // Supported
#define EFFECT_PATTERN_BREAK         0xD // Supported

#define EFFECT_EXTENDED              0xE
#define EFFECT_E_FINE_PORTA_UP       0x1 // Supported
#define EFFECT_E_FINE_PORTA_DOWN     0x2 // Supported
#define EFFECT_E_GLISSANDO_CTRL      0x3 // - TO BE DONE -
#define EFFECT_E_VIBRATO_WAVEFORM    0x4 // - TO BE DONE -
#define EFFECT_E_SET_FINETUNE        0x5 // - TO BE DONE -
#define EFFECT_E_PATTERN_LOOP        0x6 // Supported
#define EFFECT_E_TREMOLO_WAVEFORM    0x7 // - TO BE DONE -
#define EFFECT_E_SET_PANNING_2       0x8 // - TO BE DONE -
#define EFFECT_E_RETRIGGER_NOTE      0x9 // - TO BE DONE -
#define EFFECT_E_FINE_VOLSLIDE_UP    0xA // Supported
#define EFFECT_E_FINE_VOLSLIDE_DOWN  0xB // Supported
#define EFFECT_E_NOTE_CUT            0xC // Supported
#define EFFECT_E_NOTE_DELAY          0xD // - TO BE DONE -
#define EFFECT_E_PATTERN_DELAY       0xE // Supported
#define EFFECT_E_INVERT_LOOP         0xF // - TO BE DONE -
#define EFFECT_SET_SPEED             0xF0 // Supported
#define EFFECT_SET_TEMPO             0xF2 // Supported

#define PERIOD_TABLE_LENGTH  MAXNOTES
#define FULL_PERIOD_TABLE_LENGTH  ( PERIOD_TABLE_LENGTH * 8 )

static const short periodtable[]=
{
    27392, 25856, 24384, 23040, 21696, 20480, 19328, 18240, 17216, 16256, 15360, 14496,
    13696, 12928, 12192, 11520, 10848, 10240,  9664,  9120,  8606,  8128,  7680,  7248,
     6848,  6464,  6096,  5760,  5424,  5120,  4832,  4560,  4304,  4064,  3840,  3624,
     3424,  3232,  3048,  2880,  2712,  2560,  2416,  2280,  2152,  2032,  1920,  1812,
     1712,  1616,  1524,  1440,  1356,  1280,  1208,  1140,  1076,  1016,   960,   906,
      856,   808,   762,   720,   678,   640,   604,   570,   538,   508,   480,   453,
      428,   404,   381,   360,   339,   320,   302,   285,   269,   254,   240,   226,
      214,   202,   190,   180,   170,   160,   151,   143,   135,   127,   120,   113,
      107,   101,    95,    90,    85,    80,    75,    71,    67,    63,    60,    56,
       53,    50,    47,    45,    42,    40,    37,    35,    33,    31,    30,    28,
       27,    25,    24,    22,    21,    20,    19,    18,    17,    16,    15,    14,
       13,    13,    12,    11,    11,    10,     9,     9,     8,     8,     7,     7
};

static const short sintable[]={
      0,  24,  49,  74,  97, 120, 141,161,
    180, 197, 212, 224, 235, 244, 250,253,
    255, 253, 250, 244, 235, 224, 212,197,
    180, 161, 141, 120,  97,  74,  49, 24
};

typedef struct modtype_
{
    unsigned char signature[5];
    int numberofchannels;
}modtype;

modtype modlist[]=
{
    { "M!K!",4},
    { "M.K.",4},
    { "FLT4",4},
    { "FLT8",8},
    { "4CHN",4},
    { "6CHN",6},
    { "8CHN",8},
    { "10CH",10},
    { "12CH",12},
    { "14CH",14},
    { "16CH",16},
    { "18CH",18},
    { "20CH",20},
    { "22CH",22},
    { "24CH",24},
    { "26CH",26},
    { "28CH",28},
    { "30CH",30},
    { "32CH",32},
    { "",0}
};

///////////////////////////////////////////////////////////////////////////////////

static void memcopy( void * dest, void *source, unsigned long size )
{
    unsigned long i;
    unsigned char * d,*s;

    d=(unsigned char*)dest;
    s=(unsigned char*)source;
    for(i=0;i<size;i++)
    {
        d[i]=s[i];
    }
}

static void memclear( void * dest, unsigned char value, unsigned long size )
{
    unsigned long i;
    unsigned char * d;

    d=(unsigned char*)dest;
    for(i=0;i<size;i++)
    {
        d[i]=value;
    }
}

static int memcompare( unsigned char * buf1, unsigned char * buf2, unsigned int size )
{
    unsigned int i;

    i = 0;

    while(i<size)
    {
        if(buf1[i] != buf2[i])
        {
            return 0;
        }
        i++;
    }

    return 1;
}

static int getnote( jar_mod_context_t * mod, unsigned short period, int finetune )
{
    int i;

    for(i = 0; i < FULL_PERIOD_TABLE_LENGTH; i++)
    {
        if(period >= mod->fullperiod[i])
        {
            return i;
        }
    }

    return MAXNOTES;
}

static void worknote( note * nptr, channel * cptr, char t, jar_mod_context_t * mod )
{
    muint sample, period, effect, operiod;
    muint curnote, arpnote;

    sample = (nptr->sampperiod & 0xF0) | (nptr->sampeffect >> 4);
    period = ((nptr->sampperiod & 0xF) << 8) | nptr->period;
    effect = ((nptr->sampeffect & 0xF) << 8) | nptr->effect;

    operiod = cptr->period;

    if ( period || sample )
    {
        if( sample && sample < 32 )
        {
            cptr->sampnum = sample - 1;
        }

        if( period || sample )
        {
            cptr->sampdata = (char *) mod->sampledata[cptr->sampnum];
            cptr->length = mod->song.samples[cptr->sampnum].length;
            cptr->reppnt = mod->song.samples[cptr->sampnum].reppnt;
            cptr->replen = mod->song.samples[cptr->sampnum].replen;

            cptr->finetune = (mod->song.samples[cptr->sampnum].finetune)&0xF;

            if(effect>>8!=4 && effect>>8!=6)
            {
                cptr->vibraperiod=0;
                cptr->vibrapointeur=0;
            }
        }

        if( (sample != 0) && ( (effect>>8) != EFFECT_VOLSLIDE_TONEPORTA ) )
        {
            cptr->volume = mod->song.samples[cptr->sampnum].volume;
            cptr->volumeslide = 0;
        }

        if( ( (effect>>8) != EFFECT_TONE_PORTAMENTO && (effect>>8)!=EFFECT_VOLSLIDE_TONEPORTA) )
        {
            if (period!=0)
                cptr->samppos = 0;
        }

        cptr->decalperiod = 0;
        if( period )
        {
            if(cptr->finetune)
            {
                if( cptr->finetune <= 7 )
                {
                    period = mod->fullperiod[getnote(mod,period,0) + cptr->finetune];
                }
                else
                {
                    period = mod->fullperiod[getnote(mod,period,0) - (16 - (cptr->finetune)) ];
                }
            }

            cptr->period = period;
        }

    }

    cptr->effect = 0;
    cptr->parameffect = 0;
    cptr->effect_code = effect;

    switch (effect >> 8)
    {
        case EFFECT_ARPEGGIO:
            /*
            [0]: Arpeggio
            Where [0][x][y] means "play note, note+x semitones, note+y
            semitones, then return to original note". The fluctuations are
            carried out evenly spaced in one pattern division. They are usually
            used to simulate chords, but this doesn't work too well. They are
            also used to produce heavy vibrato. A major chord is when x=4, y=7.
            A minor chord is when x=3, y=7.
            */

            if(effect&0xff)
            {
                cptr->effect = EFFECT_ARPEGGIO;
                cptr->parameffect = effect&0xff;

                cptr->ArpIndex = 0;

                curnote = getnote(mod,cptr->period,cptr->finetune);

                cptr->Arpperiods[0] = cptr->period;

                arpnote = curnote + (((cptr->parameffect>>4)&0xF)*8);
                if( arpnote >= FULL_PERIOD_TABLE_LENGTH )
                    arpnote = FULL_PERIOD_TABLE_LENGTH - 1;

                cptr->Arpperiods[1] = mod->fullperiod[arpnote];

                arpnote = curnote + (((cptr->parameffect)&0xF)*8);
                if( arpnote >= FULL_PERIOD_TABLE_LENGTH )
                    arpnote = FULL_PERIOD_TABLE_LENGTH - 1;

                cptr->Arpperiods[2] = mod->fullperiod[arpnote];
            }
        break;

        case EFFECT_PORTAMENTO_UP:
            /*
            [1]: Slide up
            Where [1][x][y] means "smoothly decrease the period of current
            sample by x*16+y after each tick in the division". The
            ticks/division are set with the 'set speed' effect (see below). If
            the period of the note being played is z, then the final period
            will be z - (x*16 + y)*(ticks - 1). As the slide rate depends on
            the speed, changing the speed will change the slide. You cannot
            slide beyond the note B3 (period 113).
            */

            cptr->effect = EFFECT_PORTAMENTO_UP;
            cptr->parameffect = effect&0xff;
        break;

        case EFFECT_PORTAMENTO_DOWN:
            /*
            [2]: Slide down
            Where [2][x][y] means "smoothly increase the period of current
            sample by x*16+y after each tick in the division". Similar to [1],
            but lowers the pitch. You cannot slide beyond the note C1 (period
            856).
            */

            cptr->effect = EFFECT_PORTAMENTO_DOWN;
            cptr->parameffect = effect&0xff;
        break;

        case EFFECT_TONE_PORTAMENTO:
            /*
            [3]: Slide to note
            Where [3][x][y] means "smoothly change the period of current sample
            by x*16+y after each tick in the division, never sliding beyond
            current period". The period-length in this channel's division is a
            parameter to this effect, and hence is not played. Sliding to a
            note is similar to effects [1] and [2], but the slide will not go
            beyond the given period, and the direction is implied by that
            period. If x and y are both 0, then the old slide will continue.
            */

            cptr->effect = EFFECT_TONE_PORTAMENTO;
            if( (effect&0xff) != 0 )
            {
                cptr->portaspeed = (short)(effect&0xff);
            }

            if(period!=0)
            {
                cptr->portaperiod = period;
                cptr->period = operiod;
            }
        break;

        case EFFECT_VIBRATO:
            /*
            [4]: Vibrato
            Where [4][x][y] means "oscillate the sample pitch using a
            particular waveform with amplitude y/16 semitones, such that (x *
            ticks)/64 cycles occur in the division". The waveform is set using
            effect [14][4]. By placing vibrato effects on consecutive
            divisions, the vibrato effect can be maintained. If either x or y
            are 0, then the old vibrato values will be used.
            */

            cptr->effect = EFFECT_VIBRATO;
            if( ( effect & 0x0F ) != 0 ) // Depth continue or change ?
                cptr->vibraparam = (cptr->vibraparam & 0xF0) | ( effect & 0x0F );
            if( ( effect & 0xF0 ) != 0 ) // Speed continue or change ?
                cptr->vibraparam = (cptr->vibraparam & 0x0F) | ( effect & 0xF0 );

        break;

        case EFFECT_VOLSLIDE_TONEPORTA:
            /*
            [5]: Continue 'Slide to note', but also do Volume slide
            Where [5][x][y] means "either slide the volume up x*(ticks - 1) or
            slide the volume down y*(ticks - 1), at the same time as continuing
            the last 'Slide to note'". It is illegal for both x and y to be
            non-zero. You cannot slide outside the volume range 0..64. The
            period-length in this channel's division is a parameter to this
            effect, and hence is not played.
            */

            if( period != 0 )
            {
                cptr->portaperiod = period;
                cptr->period = operiod;
            }

            cptr->effect = EFFECT_VOLSLIDE_TONEPORTA;
            if( ( effect & 0xFF ) != 0 )
                cptr->volumeslide = ( effect & 0xFF );

        break;

        case EFFECT_VOLSLIDE_VIBRATO:
            /*
            [6]: Continue 'Vibrato', but also do Volume slide
            Where [6][x][y] means "either slide the volume up x*(ticks - 1) or
            slide the volume down y*(ticks - 1), at the same time as continuing
            the last 'Vibrato'". It is illegal for both x and y to be non-zero.
            You cannot slide outside the volume range 0..64.
            */

            cptr->effect = EFFECT_VOLSLIDE_VIBRATO;
            if( (effect & 0xFF) != 0 )
                cptr->volumeslide = (effect & 0xFF);
        break;

        case EFFECT_SET_OFFSET:
            /*
            [9]: Set sample offset
            Where [9][x][y] means "play the sample from offset x*4096 + y*256".
            The offset is measured in words. If no sample is given, yet one is
            still playing on this channel, it should be retriggered to the new
            offset using the current volume.
            */

            cptr->samppos = ((effect>>4) * 4096) + ((effect&0xF)*256);

        break;

        case EFFECT_VOLUME_SLIDE:
            /*
            [10]: Volume slide
            Where [10][x][y] means "either slide the volume up x*(ticks - 1) or
            slide the volume down y*(ticks - 1)". If both x and y are non-zero,
            then the y value is ignored (assumed to be 0). You cannot slide
            outside the volume range 0..64.
            */

            cptr->effect = EFFECT_VOLUME_SLIDE;
            cptr->volumeslide = (effect & 0xFF);
        break;

        case EFFECT_JUMP_POSITION:
            /*
            [11]: Position Jump
            Where [11][x][y] means "stop the pattern after this division, and
            continue the song at song-position x*16+y". This shifts the
            'pattern-cursor' in the pattern table (see above). Legal values for
            x*16+y are from 0 to 127.
            */

            mod->tablepos = (effect & 0xFF);
            if(mod->tablepos >= mod->song.length)
            {
                mod->tablepos = 0;
            }
            mod->patternpos = 0;
            mod->jump_loop_effect = 1;

        break;

        case EFFECT_SET_VOLUME:
            /*
            [12]: Set volume
            Where [12][x][y] means "set current sample's volume to x*16+y".
            Legal volumes are 0..64.
            */

            cptr->volume = (effect & 0xFF);
        break;

        case EFFECT_PATTERN_BREAK:
            /*
            [13]: Pattern Break
            Where [13][x][y] means "stop the pattern after this division, and
            continue the song at the next pattern at division x*10+y" (the 10
            is not a typo). Legal divisions are from 0 to 63 (note Protracker
            exception above).
            */

            mod->patternpos = ( ((effect>>4)&0xF)*10 + (effect&0xF) ) * mod->number_of_channels;
            mod->jump_loop_effect = 1;
            mod->tablepos++;
            if(mod->tablepos >= mod->song.length)
            {
                mod->tablepos = 0;
            }

        break;

        case EFFECT_EXTENDED:
            switch( (effect>>4) & 0xF )
            {
                case EFFECT_E_FINE_PORTA_UP:
                    /*
                    [14][1]: Fineslide up
                    Where [14][1][x] means "decrement the period of the current sample
                    by x". The incrementing takes place at the beginning of the
                    division, and hence there is no actual sliding. You cannot slide
                    beyond the note B3 (period 113).
                    */

                    cptr->period -= (effect & 0xF);
                    if( cptr->period < 113 )
                        cptr->period = 113;
                break;

                case EFFECT_E_FINE_PORTA_DOWN:
                    /*
                    [14][2]: Fineslide down
                    Where [14][2][x] means "increment the period of the current sample
                    by x". Similar to [14][1] but shifts the pitch down. You cannot
                    slide beyond the note C1 (period 856).
                    */

                    cptr->period += (effect & 0xF);
                    if( cptr->period > 856 )
                        cptr->period = 856;
                break;

                case EFFECT_E_FINE_VOLSLIDE_UP:
                    /*
                    [14][10]: Fine volume slide up
                    Where [14][10][x] means "increment the volume of the current sample
                    by x". The incrementing takes place at the beginning of the
                    division, and hence there is no sliding. You cannot slide beyond
                    volume 64.
                    */

                    cptr->volume += (effect & 0xF);
                    if( cptr->volume>64 )
                        cptr->volume = 64;
                break;

                case EFFECT_E_FINE_VOLSLIDE_DOWN:
                    /*
                    [14][11]: Fine volume slide down
                    Where [14][11][x] means "decrement the volume of the current sample
                    by x". Similar to [14][10] but lowers volume. You cannot slide
                    beyond volume 0.
                    */

                    cptr->volume -= (effect & 0xF);
                    if( cptr->volume > 200 )
                        cptr->volume = 0;
                break;

                case EFFECT_E_PATTERN_LOOP:
                    /*
                    [14][6]: Loop pattern
                    Where [14][6][x] means "set the start of a loop to this division if
                    x is 0, otherwise after this division, jump back to the start of a
                    loop and play it another x times before continuing". If the start
                    of the loop was not set, it will default to the start of the
                    current pattern. Hence 'loop pattern' cannot be performed across
                    multiple patterns. Note that loops do not support nesting, and you
                    may generate an infinite loop if you try to nest 'loop pattern's.
                    */

                    if( effect & 0xF )
                    {
                        if( cptr->patternloopcnt )
                        {
                            cptr->patternloopcnt--;
                            if( cptr->patternloopcnt )
                            {
                                mod->patternpos = cptr->patternloopstartpoint;
                                mod->jump_loop_effect = 1;
                            }
                            else
                            {
                                cptr->patternloopstartpoint = mod->patternpos ;
                            }
                        }
                        else
                        {
                            cptr->patternloopcnt = (effect & 0xF);
                            mod->patternpos = cptr->patternloopstartpoint;
                            mod->jump_loop_effect = 1;
                        }
                    }
                    else // Start point
                    {
                        cptr->patternloopstartpoint = mod->patternpos;
                    }

                break;

                case EFFECT_E_PATTERN_DELAY:
                    /*
                    [14][14]: Delay pattern
                    Where [14][14][x] means "after this division there will be a delay
                    equivalent to the time taken to play x divisions after which the
                    pattern will be resumed". The delay only relates to the
                    interpreting of new divisions, and all effects and previous notes
                    continue during delay.
                    */

                    mod->patterndelay = (effect & 0xF);
                break;

                case EFFECT_E_NOTE_CUT:
                    /*
                    [14][12]: Cut sample
                    Where [14][12][x] means "after the current sample has been played
                    for x ticks in this division, its volume will be set to 0". This
                    implies that if x is 0, then you will not hear any of the sample.
                    If you wish to insert "silence" in a pattern, it is better to use a
                    "silence"-sample (see above) due to the lack of proper support for
                    this effect.
                    */
                    cptr->effect = EFFECT_E_NOTE_CUT;
                    cptr->cut_param = (effect & 0xF);
                    if(!cptr->cut_param)
                        cptr->volume = 0;
                break;

                default:

                break;
            }
        break;

        case 0xF:
            /*
            [15]: Set speed
            Where [15][x][y] means "set speed to x*16+y". Though it is nowhere
            near that simple. Let z = x*16+y. Depending on what values z takes,
            different units of speed are set, there being two: ticks/division
            and beats/minute (though this one is only a label and not strictly
            true). If z=0, then what should technically happen is that the
            module stops, but in practice it is treated as if z=1, because
            there is already a method for stopping the module (running out of
            patterns). If z<=32, then it means "set ticks/division to z"
            otherwise it means "set beats/minute to z" (convention says that
            this should read "If z<32.." but there are some composers out there
            that defy conventions). Default values are 6 ticks/division, and
            125 beats/minute (4 divisions = 1 beat). The beats/minute tag is
            only meaningful for 6 ticks/division. To get a more accurate view
            of how things work, use the following formula:
                                     24 * beats/minute
                  divisions/minute = -----------------
                                      ticks/division
            Hence divisions/minute range from 24.75 to 6120, eg. to get a value
            of 2000 divisions/minute use 3 ticks/division and 250 beats/minute.
            If multiple "set speed" effects are performed in a single division,
            the ones on higher-numbered channels take precedence over the ones
            on lower-numbered channels. This effect has a large number of
            different implementations, but the one described here has the
            widest usage.
            */

            if( (effect&0xFF) < 0x21 )
            {
                if( effect&0xFF )
                {
                    mod->song.speed = effect&0xFF;
                    mod->patternticksaim = (long)mod->song.speed * ((mod->playrate * 5 ) / (((long)2 * (long)mod->bpm)));
                }
            }

            if( (effect&0xFF) >= 0x21 )
            {
                ///  HZ = 2 * BPM / 5
                mod->bpm = effect&0xFF;
                mod->patternticksaim = (long)mod->song.speed * ((mod->playrate * 5 ) / (((long)2 * (long)mod->bpm)));
            }

        break;

        default:
        // Unsupported effect
        break;

    }

}

static void workeffect( note * nptr, channel * cptr )
{
    switch(cptr->effect)
    {
        case EFFECT_ARPEGGIO:

            if( cptr->parameffect )
            {
                cptr->decalperiod = cptr->period - cptr->Arpperiods[cptr->ArpIndex];

                cptr->ArpIndex++;
                if( cptr->ArpIndex>2 )
                    cptr->ArpIndex = 0;
            }
        break;

        case EFFECT_PORTAMENTO_UP:

            if(cptr->period)
            {
                cptr->period -= cptr->parameffect;

                if( cptr->period < 113 || cptr->period > 20000 )
                    cptr->period = 113;
            }

        break;

        case EFFECT_PORTAMENTO_DOWN:

            if(cptr->period)
            {
                cptr->period += cptr->parameffect;

                if( cptr->period > 20000 )
                    cptr->period = 20000;
            }

        break;

        case EFFECT_VOLSLIDE_TONEPORTA:
        case EFFECT_TONE_PORTAMENTO:

            if( cptr->period && ( cptr->period != cptr->portaperiod ) && cptr->portaperiod )
            {
                if( cptr->period > cptr->portaperiod )
                {
                    if( cptr->period - cptr->portaperiod >= cptr->portaspeed )
                    {
                        cptr->period -= cptr->portaspeed;
                    }
                    else
                    {
                        cptr->period = cptr->portaperiod;
                    }
                }
                else
                {
                    if( cptr->portaperiod - cptr->period >= cptr->portaspeed )
                    {
                        cptr->period += cptr->portaspeed;
                    }
                    else
                    {
                        cptr->period = cptr->portaperiod;
                    }
                }

                if( cptr->period == cptr->portaperiod )
                {
                    // If the slide is over, don't let it to be retriggered.
                    cptr->portaperiod = 0;
                }
            }

            if( cptr->effect == EFFECT_VOLSLIDE_TONEPORTA )
            {
                if( cptr->volumeslide > 0x0F )
                {
                    cptr->volume = cptr->volume + (cptr->volumeslide>>4);

                    if(cptr->volume>63)
                        cptr->volume = 63;
                }
                else
                {
                    cptr->volume = cptr->volume - (cptr->volumeslide);

                    if(cptr->volume>63)
                        cptr->volume=0;
                }
            }
        break;

        case EFFECT_VOLSLIDE_VIBRATO:
        case EFFECT_VIBRATO:

            cptr->vibraperiod = ( (cptr->vibraparam&0xF) * sintable[cptr->vibrapointeur&0x1F] )>>7;

            if( cptr->vibrapointeur > 31 )
                cptr->vibraperiod = -cptr->vibraperiod;

            cptr->vibrapointeur = (cptr->vibrapointeur+(((cptr->vibraparam>>4))&0xf)) & 0x3F;

            if( cptr->effect == EFFECT_VOLSLIDE_VIBRATO )
            {
                if( cptr->volumeslide > 0xF )
                {
                    cptr->volume = cptr->volume+(cptr->volumeslide>>4);

                    if( cptr->volume > 64 )
                        cptr->volume = 64;
                }
                else
                {
                    cptr->volume = cptr->volume - cptr->volumeslide;

                    if( cptr->volume > 64 )
                        cptr->volume = 0;
                }
            }

        break;

        case EFFECT_VOLUME_SLIDE:

            if( cptr->volumeslide > 0xF )
            {
                cptr->volume += (cptr->volumeslide>>4);

                if( cptr->volume > 64 )
                    cptr->volume = 64;
            }
            else
            {
                cptr->volume -= (cptr->volumeslide&0xf);

                if( cptr->volume > 64 )
                    cptr->volume = 0;
            }
        break;

        case EFFECT_E_NOTE_CUT:
            if(cptr->cut_param)
                cptr->cut_param--;

            if(!cptr->cut_param)
                cptr->volume = 0;
        break;

        default:
        break;

    }

}

///////////////////////////////////////////////////////////////////////////////////
bool jar_mod_init(jar_mod_context_t * modctx)
{
    muint i,j;

    if( modctx )
    {
        memclear(modctx, 0, sizeof(jar_mod_context_t));
        modctx->playrate = DEFAULT_SAMPLE_RATE;
        modctx->stereo = 1;
        modctx->stereo_separation = 1;
        modctx->bits = 16;
        modctx->filter = 1;

        for(i=0; i < PERIOD_TABLE_LENGTH - 1; i++)
        {
            for(j=0; j < 8; j++)
            {
                modctx->fullperiod[(i*8) + j] = periodtable[i] - ((( periodtable[i] - periodtable[i+1] ) / 8) * j);
            }
        }

        return 1;
    }

    return 0;
}

bool jar_mod_setcfg(jar_mod_context_t * modctx, int samplerate, int bits, int stereo, int stereo_separation, int filter)
{
    if( modctx )
    {
        modctx->playrate = samplerate;

        if( stereo )
            modctx->stereo = 1;
        else
            modctx->stereo = 0;
            
        if(stereo_separation < 4)
        {
            modctx->stereo_separation = stereo_separation;
        }

        if( bits == 8 || bits == 16 )
            modctx->bits = bits;
        else
            modctx->bits = 16;

        if( filter )
            modctx->filter = 1;
        else
            modctx->filter = 0;

        return 1;
    }

    return 0;
}

// make certain that mod_data stays in memory while playing
static bool jar_mod_load( jar_mod_context_t * modctx, void * mod_data, int mod_data_size )
{
    muint i, max;
    unsigned short t;
    sample *sptr;
    unsigned char * modmemory,* endmodmemory;

    modmemory = (unsigned char *)mod_data;
    endmodmemory = modmemory + mod_data_size;
    
    

    if(modmemory)
    {
        if( modctx )
        {
            memcopy(&(modctx->song), modmemory, 1084);

            i = 0;
            modctx->number_of_channels = 0;
            while(modlist[i].numberofchannels)
            {
                if(memcompare(modctx->song.signature,modlist[i].signature,4))
                {
                    modctx->number_of_channels = modlist[i].numberofchannels;
                }

                i++;
            }

            if( !modctx->number_of_channels )
            {
                // 15 Samples modules support
                // Shift the whole datas to make it look likes a standard 4 channels mod.
                memcopy(&(modctx->song.signature), "M.K.", 4);
                memcopy(&(modctx->song.length), &(modctx->song.samples[15]), 130);
                memclear(&(modctx->song.samples[15]), 0, 480);
                modmemory += 600;
                modctx->number_of_channels = 4;
            }
            else
            {
                modmemory += 1084;
            }

            if( modmemory >= endmodmemory )
                return 0; // End passed ? - Probably a bad file !

            // Patterns loading
            for (i = max = 0; i < 128; i++)
            {
                while (max <= modctx->song.patterntable[i])
                {
                    modctx->patterndata[max] = (note*)modmemory;
                    modmemory += (256*modctx->number_of_channels);
                    max++;

                    if( modmemory >= endmodmemory )
                        return 0; // End passed ? - Probably a bad file !
                }
            }

            for (i = 0; i < 31; i++)
                modctx->sampledata[i]=0;

            // Samples loading
            for (i = 0, sptr = modctx->song.samples; i <31; i++, sptr++)
            {
                t= (sptr->length &0xFF00)>>8 | (sptr->length &0xFF)<<8;
                sptr->length = t*2;

                t= (sptr->reppnt &0xFF00)>>8 | (sptr->reppnt &0xFF)<<8;
                sptr->reppnt = t*2;

                t= (sptr->replen &0xFF00)>>8 | (sptr->replen &0xFF)<<8;
                sptr->replen = t*2;


                if (sptr->length == 0) continue;

                modctx->sampledata[i] = (char*)modmemory;
                modmemory += sptr->length;

                if (sptr->replen + sptr->reppnt > sptr->length)
                    sptr->replen = sptr->length - sptr->reppnt;

                if( modmemory > endmodmemory )
                    return 0; // End passed ? - Probably a bad file !
            }

            // States init

            modctx->tablepos = 0;
            modctx->patternpos = 0;
            modctx->song.speed = 6;
            modctx->bpm = 125;
            modctx->samplenb = 0;

            modctx->patternticks = (((long)modctx->song.speed * modctx->playrate * 5)/ (2 * modctx->bpm)) + 1;
            modctx->patternticksaim = ((long)modctx->song.speed * modctx->playrate * 5) / (2 * modctx->bpm);

            modctx->sampleticksconst = 3546894UL / modctx->playrate; //8448*428/playrate;

            for(i=0; i < modctx->number_of_channels; i++)
            {
                modctx->channels[i].volume = 0;
                modctx->channels[i].period = 0;
            }

            modctx->mod_loaded = 1;

            return 1;
        }
    }

    return 0;
}

void jar_mod_fillbuffer( jar_mod_context_t * modctx, short * outbuffer, unsigned long nbsample, jar_mod_tracker_buffer_state * trkbuf )
{
    unsigned long i, j;
    unsigned long k;
    unsigned char c;
    unsigned int state_remaining_steps;
    int l,r;
    int ll,lr;
    int tl,tr;
    short finalperiod;
    note    *nptr;
    channel *cptr;

    if( modctx && outbuffer )
    {
        if(modctx->mod_loaded)
        {
            state_remaining_steps = 0;

            if( trkbuf )
            {
                trkbuf->cur_rd_index = 0;

                memcopy(trkbuf->name,modctx->song.title,sizeof(modctx->song.title));

                for(i=0;i<31;i++)
                {
                    memcopy(trkbuf->instruments[i].name,modctx->song.samples[i].name,sizeof(trkbuf->instruments[i].name));
                }
            }

            ll = modctx->last_l_sample;
            lr = modctx->last_r_sample;

            for (i = 0; i < nbsample; i++)
            {
                //---------------------------------------
                if( modctx->patternticks++ > modctx->patternticksaim )
                {
                    if( !modctx->patterndelay )
                    {
                        nptr = modctx->patterndata[modctx->song.patterntable[modctx->tablepos]];
                        nptr = nptr + modctx->patternpos;
                        cptr = modctx->channels;

                        modctx->patternticks = 0;
                        modctx->patterntickse = 0;

                        for(c=0;c<modctx->number_of_channels;c++)
                        {
                            worknote((note*)(nptr+c), (channel*)(cptr+c),(char)(c+1),modctx);
                        }

                        if( !modctx->jump_loop_effect )
                            modctx->patternpos += modctx->number_of_channels;
                        else
                            modctx->jump_loop_effect = 0;

                        if( modctx->patternpos == 64*modctx->number_of_channels )
                        {
                            modctx->tablepos++;
                            modctx->patternpos = 0;
                            if(modctx->tablepos >= modctx->song.length)
                            {
                                modctx->tablepos = 0;
                                modctx->loopcount++; // count next loop
                            }
                        }
                    }
                    else
                    {
                        modctx->patterndelay--;
                        modctx->patternticks = 0;
                        modctx->patterntickse = 0;
                    }

                }

                if( modctx->patterntickse++ > (modctx->patternticksaim/modctx->song.speed) )
                {
                    nptr = modctx->patterndata[modctx->song.patterntable[modctx->tablepos]];
                    nptr = nptr + modctx->patternpos;
                    cptr = modctx->channels;

                    for(c=0;c<modctx->number_of_channels;c++)
                    {
                        workeffect(nptr+c, cptr+c);
                    }

                    modctx->patterntickse = 0;
                }

                //---------------------------------------

                if( trkbuf && !state_remaining_steps )
                {
                    if( trkbuf->nb_of_state < trkbuf->nb_max_of_state )
                    {
                        memclear(&trkbuf->track_state_buf[trkbuf->nb_of_state], 0, sizeof(tracker_state));
                    }
                }

                l=0;
                r=0;

                for(j =0, cptr = modctx->channels; j < modctx->number_of_channels ; j++, cptr++)
                {
                    if( cptr->period != 0 )
                    {
                        finalperiod = cptr->period - cptr->decalperiod - cptr->vibraperiod;
                        if( finalperiod )
                        {
                            cptr->samppos += ( (modctx->sampleticksconst<<10) / finalperiod );
                        }

                        cptr->ticks++;

                        if( cptr->replen<=2 )
                        {
                            if( (cptr->samppos>>10) >= (cptr->length) )
                            {
                                cptr->length = 0;
                                cptr->reppnt = 0;

                                if( cptr->length )
                                    cptr->samppos = cptr->samppos % (((unsigned long)cptr->length)<<10);
                                else
                                    cptr->samppos = 0;
                            }
                        }
                        else
                        {
                            if( (cptr->samppos>>10) >= (unsigned long)(cptr->replen+cptr->reppnt) )
                            {
                                cptr->samppos = ((unsigned long)(cptr->reppnt)<<10) + (cptr->samppos % ((unsigned long)(cptr->replen+cptr->reppnt)<<10));
                            }
                        }

                        k = cptr->samppos >> 10;

                        if( cptr->sampdata!=0 && ( ((j&3)==1) || ((j&3)==2) ) )
                        {
                            r += ( cptr->sampdata[k] *  cptr->volume );
                        }

                        if( cptr->sampdata!=0 && ( ((j&3)==0) || ((j&3)==3) ) )
                        {
                            l += ( cptr->sampdata[k] *  cptr->volume );
                        }

                        if( trkbuf && !state_remaining_steps )
                        {
                            if( trkbuf->nb_of_state < trkbuf->nb_max_of_state )
                            {
                                trkbuf->track_state_buf[trkbuf->nb_of_state].number_of_tracks = modctx->number_of_channels;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].buf_index = i;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].cur_pattern = modctx->song.patterntable[modctx->tablepos];
                                trkbuf->track_state_buf[trkbuf->nb_of_state].cur_pattern_pos = modctx->patternpos / modctx->number_of_channels;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].cur_pattern_table_pos = modctx->tablepos;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].bpm = modctx->bpm;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].speed = modctx->song.speed;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].tracks[j].cur_effect = cptr->effect_code;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].tracks[j].cur_parameffect = cptr->parameffect;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].tracks[j].cur_period = finalperiod;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].tracks[j].cur_volume = cptr->volume;
                                trkbuf->track_state_buf[trkbuf->nb_of_state].tracks[j].instrument_number = (unsigned char)cptr->sampnum;
                            }
                        }
                    }
                }

                if( trkbuf && !state_remaining_steps )
                {
                    state_remaining_steps = trkbuf->sample_step;

                    if(trkbuf->nb_of_state < trkbuf->nb_max_of_state)
                        trkbuf->nb_of_state++;
                }
                else
                {
                    state_remaining_steps--;
                }

                tl = (short)l;
                tr = (short)r;

                if ( modctx->filter )
                {
                    // Filter
                    l = (l+ll)>>1;
                    r = (r+lr)>>1;
                }

                if ( modctx->stereo_separation == 1 )
                {
                    // Left & Right Stereo panning
                    l = (l+(r>>1));
                    r = (r+(l>>1));
                }

                // Level limitation
                if( l > 32767 ) l = 32767;
                if( l < -32768 ) l = -32768;
                if( r > 32767 ) r = 32767;
                if( r < -32768 ) r = -32768;

                // Store the final sample.
                outbuffer[(i*2)]   = l;
                outbuffer[(i*2)+1] = r;

                ll = tl;
                lr = tr;

            }

            modctx->last_l_sample = ll;
            modctx->last_r_sample = lr;

            modctx->samplenb = modctx->samplenb+nbsample;
        }
        else
        {
            for (i = 0; i < nbsample; i++)
            {
                // Mod not loaded. Return blank buffer.
                outbuffer[(i*2)]   = 0;
                outbuffer[(i*2)+1] = 0;
            }

            if(trkbuf)
            {
                trkbuf->nb_of_state = 0;
                trkbuf->cur_rd_index = 0;
                trkbuf->name[0] = 0;
                memclear(trkbuf->track_state_buf, 0, sizeof(tracker_state) * trkbuf->nb_max_of_state);
                memclear(trkbuf->instruments, 0, sizeof(trkbuf->instruments));
            }
        }
    }
}

//resets internals for mod context
static bool jar_mod_reset( jar_mod_context_t * modctx)
{
    if(modctx)
    {
        memclear(&modctx->song, 0, sizeof(modctx->song));
        memclear(&modctx->sampledata, 0, sizeof(modctx->sampledata));
        memclear(&modctx->patterndata, 0, sizeof(modctx->patterndata));
        modctx->tablepos = 0;
        modctx->patternpos = 0;
        modctx->patterndelay  = 0;
        modctx->jump_loop_effect = 0;
        modctx->bpm = 0;
        modctx->patternticks = 0;
        modctx->patterntickse = 0;
        modctx->patternticksaim = 0;
        modctx->sampleticksconst = 0;
        modctx->samplenb = 0;
        memclear(modctx->channels, 0, sizeof(modctx->channels));
        modctx->number_of_channels = 0;
        modctx->mod_loaded = 0;
        modctx->last_r_sample = 0;
        modctx->last_l_sample = 0;
        
        return jar_mod_init(modctx);
    }
    return 0;
}

void jar_mod_unload( jar_mod_context_t * modctx)
{
    if(modctx)
    {
        if(modctx->modfile)
        {
            JARMOD_FREE(modctx->modfile);
            modctx->modfile = 0;
            modctx->modfilesize = 0;
            modctx->loopcount = 0;
        }
        jar_mod_reset(modctx);
    }
}

mulong jar_mod_load_file(jar_mod_context_t * modctx, const char* filename)
{
    mulong fsize = 0;
    if(modctx->modfile)
    {
        JARMOD_FREE(modctx->modfile);
        modctx->modfile = 0;
    }
    
    FILE *f = fopen(filename, "rb");
    if(f)
    {
        fseek(f,0,SEEK_END);
        fsize = ftell(f);
        fseek(f,0,SEEK_SET);
        
        if(fsize && fsize < 32*1024*1024)
        {
            modctx->modfile = JARMOD_MALLOC(fsize);
            modctx->modfilesize = fsize;
            memset(modctx->modfile, 0, fsize);
            fread(modctx->modfile, fsize, 1, f);
            fclose(f);
            
            if(!jar_mod_load(modctx, (void*)modctx->modfile, fsize)) fsize = 0;
        } else fsize = 0;
    }
    return fsize;
}

mulong jar_mod_current_samples(jar_mod_context_t * modctx)
{
    if(modctx)
        return modctx->samplenb;
    
    return 0;
}

// Works, however it is very slow, this data should be cached to ensure it is run only once per file
mulong jar_mod_max_samples(jar_mod_context_t * ctx)
{
    mint buff[2];
    mulong len;
    mulong lastcount = ctx->loopcount;
    
    while(ctx->loopcount <= lastcount)
        jar_mod_fillbuffer(ctx, buff, 1, 0);
    
    len = ctx->samplenb;
    jar_mod_seek_start(ctx);
    
    return len;
}

// move seek_val to sample index, 0 -> jar_mod_max_samples is the range
void jar_mod_seek_start(jar_mod_context_t * ctx)
{
    if(ctx && ctx->modfile)
    {
        muchar* ftmp = ctx->modfile;
        mulong stmp = ctx->modfilesize;
        muint lcnt = ctx->loopcount;
        
        if(jar_mod_reset(ctx)){
            jar_mod_load(ctx, ftmp, stmp);
            ctx->modfile = ftmp;
            ctx->modfilesize = stmp;
            ctx->loopcount = lcnt;
        }
    }
}

#endif // end of JAR_MOD_IMPLEMENTATION
//-------------------------------------------------------------------------------


#endif //end of header file
