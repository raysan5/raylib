////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Prediction-Encoded Pixels (pep)
//
//  author(s):
//  ENDESGA - https://x.com/ENDESGA | https://bsky.app/profile/endesga.bsky.social
//
//  contributer(s):
//  Mariusz Dzikowski - https://github.com/demurzasty
//  Akreson - https://github.com/Akreson
//  Alepacho - https://github.com/Alepacho
//
//  https://github.com/ENDESGA/pep
//  2026 - CC0 - FOSS forever
//

// The .pep type is a pixel art format made to compress as small as possible.
// It uses a custom "Prediction by Partial Matching" compression method
// designed from the ground up for palettized pixel art.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region - DEPENDENCIES
//

#pragma once

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <stdint.h> // uint*_t
#include <stdlib.h> // mem-alloc
#include <stdio.h> // FILE
#include <string.h> // memset

#pragma endregion dependencies

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region - MACROS
//

#define __PEP_STRINGIFY( VALUE ) #VALUE
#define _PEP_STRINGIFY( VALUE ) __PEP_STRINGIFY( VALUE )

////////////////////////////////////////////////////////////////
#pragma region - memory

// This defines a set of macros that serve as wrappers for the standard
// C library memory management functions: `malloc`, `realloc`, and `free`.
// These macros can be used to easily replace the underlying memory allocation
// implementation in a project, for example, with a custom allocator or a
// debug-enabled version, without modifying all call sites.
#ifndef PEP_MALLOC
	#define PEP_MALLOC( size ) malloc( size )
	#define PEP_REALLOC( ptr, size ) realloc( ptr, size )
	#define PEP_FREE( ptr ) free( ptr )
#endif

#pragma endregion memory

////////////////////////////////////////////////////////////////
#pragma region - bits

// Provides a cross-platform macro to count leading zeros in a 32-bit integer.
#ifndef PEP_COUNT_LEADING_ZEROS
	#ifdef _MSC_VER
		// Microsoft Visual C++ compiler.
		#define PEP_COUNT_LEADING_ZEROS( x ) __lzcnt( x )
	#else
		// GCC/Clang compilers.
		#define PEP_COUNT_LEADING_ZEROS( x ) __builtin_clz( x )
	#endif
#endif

// How many bits do we need to fit N values?
#define PEP_BITS_TO_FIT( N ) ( ( ( N ) <= 1 ) ? 1 : ( 32 - PEP_COUNT_LEADING_ZEROS( ( N ) - 1 ) ) )

#pragma endregion bits

////////////////////////////////////////////////////////////////
#pragma region - hash

// Standard hash multipliers, there might be better ones:
#define _PEP_HASH_N 1540483477u // 0x5BD1E995 MurmurHash2
#define _PEP_HASH_W 3332679571u // 0xC6A4A793 MurmurHash64A
#define _PEP_HASH_NW 3432918353u // 0xCC9E2D51 MurmurHash3
#define _PEP_HASH_NE 2246822507u // 0x85EBCA6B MurmurHash3
// Hash-prospector, avalanche-optimized, by Chris Wellons:
#define _PEP_HASH_NN 2146121005u // 0x7FEB352D lowbias32
#define _PEP_HASH_WW 2890668881u // 0xAC4C1B51 triple32 round-2
#define _PEP_HASH_NNW 830770091u // 0x31848BAB triple32 round-3
#define _PEP_HASH_NNE 1935289751u // 0x735A2D97 prospector
#define _PEP_HASH_NWW 2943497623u // 0xAF723597 prospector

#pragma endregion hash

#pragma endregion macros

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region - CONSTANTS
//

#define PEP_NAME "pep"

////////////////////////////////////////////////////////////////
#pragma region - version

#define PEP_VERSION_MAJOR 0
#define PEP_VERSION_MINOR 6
#define PEP_VERSION_PATCH 0
#define PEP_VERSION _PEP_STRINGIFY( PEP_VERSION_MAJOR ) "." _PEP_STRINGIFY( PEP_VERSION_MINOR ) "." _PEP_STRINGIFY( PEP_VERSION_PATCH )

#pragma endregion version

////////////////////////////////////////////////////////////////
#pragma region - frequency

// This is the amount of frequencies per context, and the amount of contexts,
// with [256] being the order0 context.
// Originally there were 256*256 contexts, but I found the image didn't get
// much bigger with the same amount of frequencies. Theoretically the more
// contexts you have the smaller the image is...
#define PEP_FREQ_N 257
#define PEP_FREQ_END ( PEP_FREQ_N - 1 )
#define PEP_CONTEXTS_MAX PEP_FREQ_END

// PEP_FREQ_MAX is the maximum accumulative frequency.
// This is the starting maximum, and is scaled as the image compresses, via
// the palette-delta; which seems to roughly correlate with the complexity.
// This value works better for low-res images.
#define PEP_FREQ_MAX ( PEP_FREQ_END >> 1 )

#pragma endregion frequency

////////////////////////////////////////////////////////////////
#pragma region - arithmetic -coding

// These constants are for the 63bit arithmetic-coding, specifically not 64bit
// because of overflow.
#define PEP_CODE_BITS 24lu
#define PEP_CODE_BITS_INV ( 32lu - PEP_CODE_BITS )
#define PEP_FREQ_MAX_BITS 14lu
#define PEP_PROB_MAX_VALUE ( 1 << PEP_FREQ_MAX_BITS )
#define PEP_CODE_MAX_VALUE ( ( 1 << PEP_CODE_BITS ) - 1 )

#pragma endregion arithmetic -coding

#pragma endregion defaults

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region - DECLARATIONS
//

////////////////////////////////////////////////////////////////
#pragma region - formats

// .pep can automatically convert from RGBA to BGRA (little/big endian),
// which is often for low-level/backend rendering pipelines.
// Windows only supports ARGB for example, and so sometimes it's easier to
// make the whole application use that format.
typedef enum
{
	pep_rgba,
	pep_bgra,

	pep_abgr,
	pep_argb
}
pep_format;

// Palette colors can be restricted in the serialization phase to a maximum
// amount of bits per channel.
// The default is 8 bits per channel (standard 32 bit colors).
// This is NOT bits-per-pixel and does NOT cap palette_size.
typedef enum
{
	pep_1bit,
	pep_2bit,
	pep_4bit,
	pep_8bit
}
pep_channel_bits;

#pragma endregion formats

////////////////////////////////////////////////////////////////
#pragma region - arithmetic -coding

// During the compression process the context per frequency-group needs to be
// tracked, with the sum of all frequencies being stored.
typedef struct
{
	uint16_t freq[ PEP_FREQ_N ];
	uint32_t sum;
}
_pep_context;

// Arithmetic coding structures:
typedef struct
{
	uint8_t* data_ref;
	uint32_t low;
	uint32_t range;
}
_pep_ac_encode;

typedef struct
{
	uint8_t* data_ref;
	uint8_t* end_of_data;
	uint32_t low;
	uint32_t range;
	uint32_t code;
}
_pep_ac_decode;

typedef struct
{
	uint32_t high;
	uint32_t low;
	uint32_t scale;
}
_pep_prob;

typedef struct
{
	_pep_prob prob;
	uint32_t symbol;
}
_pep_sym_decode;

#pragma endregion arithmetic -coding

////////////////////////////////////////////////////////////////
#pragma region - pep

// This is the main struct-type that contains values for using this format.
//
// `channel_bits` controls how many bits each R/G/B/A component of every
// palette entry is quantized to during serialization. It does NOT limit
// the number of palette entries (that's `palette_size`, up to 256).
// For example, pep_2bit means each channel has 4 possible values (0/85/170/255),
// so the palette can only draw from 4*4*4 = 64 distinct RGB colors.
typedef struct
{
	uint8_t* bytes;
	uint64_t bytes_size;
	uint16_t width;
	uint16_t height;
	pep_format format;
	uint32_t palette[ 256 ];
	uint8_t palette_size;
	pep_channel_bits channel_bits;
	uint8_t model;
}
pep;

#pragma endregion pep

////////////////////////////////////////////////////////////////
#pragma region - functions

pep pep_compress( const uint32_t* in_pixels, const uint16_t width, const uint16_t height, const pep_format in_format, const pep_channel_bits in_channel_bits );
uint32_t* pep_decompress( const pep* const in_pep, const pep_format out_format, const uint8_t first_color_transparent, uint8_t const pre_multiply );
void pep_free( pep* in_pep );
uint8_t* pep_serialize( const pep* in_pep, uint32_t* const out_size );
pep pep_deserialize( const uint8_t* const in_bytes, const uint32_t in_bytes_size );
uint8_t pep_save( const pep* const in_pep, const char* const file_path );
pep pep_load( const char* const file_path );

#pragma endregion functions

#pragma endregion declarations

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region - DEFINITIONS
//

#ifdef PEP_IMPLEMENTATION

#pragma region - private / functions

static _pep_prob _pep_get_prob_from_context( const _pep_context* const context, const uint32_t symbol );
static void _pep_arith_encode( _pep_ac_encode* const ac, const _pep_prob prob );
static void _pep_arith_encode_normalize( _pep_ac_encode* const ac );
static uint32_t _pep_arith_decode_curr_freq( _pep_ac_decode* const ac, const uint32_t scale );
static void _pep_arith_decode_update( _pep_ac_decode* const ac, const _pep_prob prob );
static _pep_sym_decode _pep_get_sym_from_freq( const _pep_context* const context, const uint32_t target_freq );
static uint32_t _pep_pre_multiply( const uint32_t pixel, const pep_format format );
static uint32_t _pep_reformat( const uint32_t in_color, const pep_format in_format, const pep_format out_format );

#pragma endregion private / functions

#ifdef _MSC_VER
	// Intrin header is only needed for implementation.
	#include <intrin.h> // __lzcnt

	// Disable unsafe fopen usage warning on MSVC compiler.
	#pragma warning( push )
	#pragma warning( disable : 4996 )
#endif

////////////////////////////////////////////////////////////////
#pragma region - arithmetic

////////////////////////////////
#pragma region | encode

// Getting cumulative frequnce of symbol
static _pep_prob _pep_get_prob_from_context( const _pep_context* const context, const uint32_t symbol )
{
	_pep_prob prob = { 0 };
	prob.scale = context->sum;

	for( uint32_t index = 0; index < symbol; ++index )
	{
		prob.low += context->freq[ index ];
	}

	prob.high = prob.low + context->freq[ symbol ];
	return prob;
}

// This encodes a symbol into the arithmetic-coding range. It scales the
// current range based on the symbol's frequency and total frequency count.
static void _pep_arith_encode( _pep_ac_encode* const ac, const _pep_prob prob )
{
	ac->range /= prob.scale;
	ac->low += prob.low * ac->range;
	ac->range *= prob.high - prob.low;
}

// Adjusts the arithmetic-coding range by removing a boundary value
// Main goal of this process is to keep our range from getting
// too small.
static void _pep_arith_encode_normalize( _pep_ac_encode* const ac )
{
	while( 1 )
	{
		if( ( ac->low ^ ( ac->low + ac->range ) ) >= PEP_CODE_MAX_VALUE )
		{
			if( ac->range >= PEP_PROB_MAX_VALUE ) break;

			ac->range = PEP_PROB_MAX_VALUE - ( ac->low & ( PEP_PROB_MAX_VALUE - 1 ) );
		}

		uint8_t byte = ac->low >> PEP_CODE_BITS;
		ac->low <<= PEP_CODE_BITS_INV;
		ac->range <<= PEP_CODE_BITS_INV;
		*ac->data_ref++= byte;
	}
}

#pragma endregion encode

////////////////////////////////
#pragma region | decode

// Getting current frequency by doing reverse trasformation
static uint32_t _pep_arith_decode_curr_freq( _pep_ac_decode* const ac, const uint32_t scale )
{
	ac->range /= scale;
	uint32_t result = ( ac->code - ac->low ) / ( ac->range );
	return result;
}

// Same as with the encode_normalize, only on decode we reading in value
static void _pep_arith_decode_update( _pep_ac_decode* const ac, const _pep_prob prob )
{
	ac->low += ac->range * prob.low;
	ac->range *= prob.high - prob.low;

	while( 1 )
	{
		if( ( ac->low ^ ( ac->low + ac->range ) ) >= PEP_CODE_MAX_VALUE )
		{
			if( ac->range < PEP_PROB_MAX_VALUE )
			{
				ac->range = PEP_PROB_MAX_VALUE - ( ac->low & ( PEP_PROB_MAX_VALUE - 1 ) );
			}
			else break;
		}

		uint8_t in_byte = 0;
		if( ac->data_ref != ac->end_of_data )
		{
			in_byte = *ac->data_ref++;
		}

		ac->code = ( ac->code << 8 ) | in_byte;
		ac->range <<= 8;
		ac->low <<= 8;
	}
}

static _pep_sym_decode _pep_get_sym_from_freq( const _pep_context* const context, const uint32_t target_freq )
{
	_pep_sym_decode result = { 0 };

	uint32_t s = 0;
	uint32_t freq = 0;
	for( ; s <= PEP_FREQ_END; ++s )
	{
		freq += context->freq[ s ];
		if( freq > target_freq ) break;
	}
	if( s > PEP_FREQ_END ) s = PEP_FREQ_END;

	result.prob.high = freq;
	result.prob.low = freq - context->freq[ s ];
	result.prob.scale = context->sum;
	result.symbol = s;

	return result;
}

#pragma endregion decode

#pragma endregion arithmetic

////////////////////////////////////////////////////////////////
#pragma region - compression

////////////////////////////////
#pragma region | compression / hidden

// pep supports pre-multiplying the RGB channels with the A channel.
static uint32_t _pep_pre_multiply( const uint32_t pixel, const pep_format format )
{
	uint8_t* bytes = ( uint8_t* )( &pixel );

	if( format <= pep_bgra )
	{
		uint32_t scaled_a = ( uint32_t )( bytes[ 3 ] ) * 257;
		bytes[ 1 ] = ( uint8_t )( ( ( uint32_t )( bytes[ 1 ] ) * scaled_a + 32896 ) >> 16 );
		bytes[ 2 ] = ( uint8_t )( ( ( uint32_t )( bytes[ 2 ] ) * scaled_a + 32896 ) >> 16 );
		bytes[ 3 ] = ( uint8_t )( ( ( uint32_t )( bytes[ 3 ] ) * scaled_a + 32896 ) >> 16 );
	}
	else
	{
		uint32_t scaled_a = ( uint32_t )( bytes[ 0 ] ) * 257;
		bytes[ 0 ] = ( uint8_t )( ( ( uint32_t )( bytes[ 0 ] ) * scaled_a + 32896 ) >> 16 );
		bytes[ 1 ] = ( uint8_t )( ( ( uint32_t )( bytes[ 1 ] ) * scaled_a + 32896 ) >> 16 );
		bytes[ 2 ] = ( uint8_t )( ( ( uint32_t )( bytes[ 2 ] ) * scaled_a + 32896 ) >> 16 );
	}

	return pixel;
}

// pep supports "dynamic formats", where you can specify what the in-bytes are,
// and reformat to a different channel-order.
// This means two "identical" pep files can have different formats, but you
// can choose how to reformat it when it decompresses!
static uint32_t _pep_reformat( const uint32_t in_color, const pep_format in_format, const pep_format out_format )
{
	if( in_format == out_format ) return in_color;

	if( in_format <= pep_bgra && out_format <= pep_bgra )
	{ // RGBA <-> BGRA: swap R and B
		return( in_color & 0xff00ff00 ) | ( ( in_color & 0x000000ff ) << 16 ) | ( ( in_color & 0x00ff0000 ) >> 16 );
	}
	else if( in_format >= pep_abgr && out_format >= pep_abgr )
	{ // ABGR <-> ARGB: swap R and B
		return( in_color & 0x00ff00ff ) | ( ( in_color & 0x0000ff00 ) << 16 ) | ( ( in_color & 0xff000000 ) >> 16 );
	}
	else if( ( in_format ^ out_format ) == 2 )
	{ // Alpha flip: RGBA <-> ARGB or BGRA <-> ABGR
		return( ( in_color & 0x000000ff ) << 24 ) | ( ( in_color & 0x0000ff00 ) << 8 ) | ( ( in_color & 0x00ff0000 ) >> 8 ) | ( ( in_color & 0xff000000 ) >> 24 );
	}
	else if( in_format < out_format )
	{ // Flip: RGBA/BGRA -> ABGR/ARGB
		return( ( in_color & 0xff000000 ) >> 24 ) | ( ( in_color & 0x00ffffff ) << 8 );
	}
	else
	{ // Flip: ABGR/ARGB -> RGBA/BGRA
		return( ( in_color & 0x000000ff ) << 24 ) | ( ( in_color & 0xffffff00 ) >> 8 );
	}
}

// Update the frequency table after encoding/decoding a symbol.
// This increments the symbol's frequency and the total sum.
// When we hit freq_max, we increase the freq_max via a complexity
// approximation via the palette-size, then we scale everything down
// to a quarter to keep the frequencies manageable.
// This dynamic part helps the compression adapt to the image's patterns.
#define _PEP_UPDATE( CONTEXT, SYMBOL, FREQ_MAX, PALETTE_SIZE )\
	do\
	{\
		CONTEXT->freq[ SYMBOL ] += 2;\
		CONTEXT->sum += 2;\
		if( CONTEXT->freq[ SYMBOL ] >= FREQ_MAX || CONTEXT->sum >= PEP_PROB_MAX_VALUE )\
		{\
			FREQ_MAX += ( PEP_FREQ_END - PALETTE_SIZE ) >> 1;\
			CONTEXT->sum = 0;\
			for( uint64_t f = 0; f < PEP_FREQ_N; f++ )\
			{\
				const uint16_t _f = CONTEXT->freq[ f ];\
				if( _f == 0 ) continue;\
				const uint16_t scaled = ( _f + 1 ) >> 1;\
				CONTEXT->freq[ f ] = scaled;\
				CONTEXT->sum += scaled;\
			}\
		}\
	}\
	while( 0 )

#pragma endregion hidden

////////////////////////////////
#pragma region | compression / visible

#define PEP_HASH_BIT_CONTEXT ( 1u << 16 )

enum
{
	_PEP_TAP_HASH,
	_PEP_TAP_NW,
	_PEP_TAP_W
};

// Different context models perform differently depending on the pixel data.
// From lots of testing it seems these 3 structures give the best results
// across the 3 major types of pixel art.
// Pep will sacrifice some time to ensure the file is as small as possible
// by testing all 3 of these without assumptions.
static const int8_t _pep_models[] =
	{
		_PEP_TAP_HASH, // typically for ~2 colors
		_PEP_TAP_NW, // typically for ~4-8 colors
		_PEP_TAP_W // typically for >=64 colors
	};
#define PEP_NUM_MODELS ( ( uint8_t )( sizeof( _pep_models ) / sizeof( _pep_models[ 0 ] ) ) )

typedef struct
{
	_pep_context* contexts;
	uint32_t index_end;
	uint32_t index_escape;
	int8_t kind;
}
_pep_model_context;

// Allocate only the context regions a given model uses, then seed order-0.
static uint8_t _pep_model_init( _pep_model_context* m, int8_t kind, uint32_t pcount )
{
	memset( m, 0, sizeof( *m ) );
	m->kind = kind;
	m->index_escape = pcount;
	m->index_end = pcount + 1;

	// order0 starts at offset 0, followed by the specific context level
	uint32_t off = 1;
	switch( kind )
	{
		case _PEP_TAP_HASH:
		{
			off += PEP_HASH_BIT_CONTEXT;
			break;
		}
		case _PEP_TAP_NW:
		{
			off += m->index_end * m->index_end;
			break;
		}
		case _PEP_TAP_W:
		{
			off += m->index_end;
			break;
		}
	}

	m->contexts = ( _pep_context* )calloc( off, sizeof( _pep_context ) );
	if( !m->contexts ) return 0;

	_pep_context* order0 = &m->contexts[ 0 ];
	for( uint32_t index = 0; index < pcount; index++ ) order0->freq[ index ] = 1;
	order0->sum = pcount;
	return 1;
}

static void _pep_model_free( _pep_model_context* m )
{
	if( m->contexts )
	{
		free( m->contexts );
		m->contexts = NULL;
	}
}

// Resolve the context pointer for the specific level of this pixel.
static _pep_context* _pep_build_level( const _pep_model_context* m, const uint8_t* idx, uint32_t pos, uint32_t x, uint32_t y, uint16_t width )
{
	// It's only possible to tap previous values:
	/*
	 -   NNW  NN  NNE
	NWW  NW   N   NE
	WW   W   [x]
	*/
	// The 10th NNWW tap has dimishing returns

	const uint32_t SENT = m->index_escape;
	const uint32_t index_end = m->index_end;
	const uint32_t Wv = ( x > 0 ) ? idx[ pos - 1 ] : SENT;
	const uint32_t Nv = ( y > 0 ) ? idx[ pos - width ] : SENT;

	switch( m->kind )
	{
		case _PEP_TAP_HASH:
		{
			const uint32_t NWv = ( x > 0 && y > 0 ) ? idx[ pos - width - 1 ] : SENT;
			const uint32_t NNv = ( y > 1 ) ? idx[ pos - 2 * width ] : SENT;
			const uint32_t WWv = ( x > 1 ) ? idx[ pos - 2 ] : SENT;
			const uint32_t NEv = ( ( x + 1 ) < width && y > 0 ) ? idx[ pos - width + 1 ] : SENT;
			const uint32_t NNWv = ( y > 1 && x > 0 ) ? idx[ pos - 2 * width - 1 ] : SENT;
			const uint32_t NNEv = ( y > 1 && ( x + 1 ) < width ) ? idx[ pos - 2 * width + 1 ] : SENT;
			const uint32_t NWWv = ( x > 1 && y > 0 ) ? idx[ pos - width - 2 ] : SENT;

			uint32_t h = Nv * _PEP_HASH_N + Wv * _PEP_HASH_W + NWv * _PEP_HASH_NW + NEv * _PEP_HASH_NE + NNv * _PEP_HASH_NN + WWv * _PEP_HASH_WW + NNWv * _PEP_HASH_NNW + NNEv * _PEP_HASH_NNE + NWWv * _PEP_HASH_NWW;
			h ^= h >> 15;
			return &m->contexts[ 1 + ( h & ( PEP_HASH_BIT_CONTEXT - 1 ) ) ];
		}
		case _PEP_TAP_NW:
		{
			return &m->contexts[ 1 + Nv * index_end + Wv ];
		}
		case _PEP_TAP_W:
		{
			return &m->contexts[ 1 + Wv ];
		}
	}
	return NULL;
}

// Encode all pixels with one model and return the size of the byte stream.
static uint64_t _pep_encode_model( const uint8_t* idx, uint16_t width, uint16_t height, uint32_t pcount, uint8_t model, uint8_t* out )
{
	_pep_model_context m;
	if( !_pep_model_init( &m, _pep_models[ model ], pcount ) ) return 0;
	const uint32_t index_escape = m.index_escape;

	_pep_ac_encode ac = { 0 };
	ac.range = ( uint32_t )( ( 1llu << 32 ) - 1 );
	ac.data_ref = out;
	uint16_t freq_max = PEP_FREQ_MAX;
	_pep_context* order0 = &m.contexts[ 0 ];

	uint32_t pos = 0;
	for( uint32_t y = 0; y < height; y++ )
	{
		for( uint32_t x = 0; x < width; x++, pos++ )
		{
			const uint32_t symbol = idx[ pos ];
			_pep_context* context = _pep_build_level( &m, idx, pos, x, y, width );
			uint8_t encoded_in_level = 0;

			if( context->sum != 0 )
			{
				if( context->freq[ symbol ] != 0 )
				{
					_pep_prob prob = _pep_get_prob_from_context( context, symbol );
					_pep_arith_encode( &ac, prob );
					_pep_arith_encode_normalize( &ac );
					_PEP_UPDATE( context, symbol, freq_max, pcount );
					encoded_in_level = 1;
				}
				else
				{
					_pep_prob prob = _pep_get_prob_from_context( context, index_escape );
					_pep_arith_encode( &ac, prob );
					_pep_arith_encode_normalize( &ac );
					context->freq[ index_escape ] += 1;
					context->sum += 1;
				}
			}

			if( !encoded_in_level )
			{
				_pep_prob prob = _pep_get_prob_from_context( order0, symbol );
				_pep_arith_encode( &ac, prob );
				_pep_arith_encode_normalize( &ac );
				_PEP_UPDATE( order0, symbol, freq_max, pcount );

				if( context->sum == 0 )
				{
					context->freq[ index_escape ] = 1;
					context->sum = 1;
				}
				context->freq[ symbol ] = 1;
				context->sum += 1;
			}
		}
	}

	for( uint8_t index = 0; index < 4; index++ )
	{
		uint8_t byte = ac.low >> PEP_CODE_BITS;
		ac.low <<= PEP_CODE_BITS_INV;
		*ac.data_ref++= byte;
	}

	uint64_t size = ( uint64_t ) ( ac.data_ref - out );
	_pep_model_free( &m );
	return size;
}

// The format of the in_pixels has to be the same as in_format.
// out_format is the one applied to the newly compressed pep

// Compresses raw pixels into a pep, trying each context model and keeping the
// smallest result (the winning model index is stored in out.model).
pep pep_compress( const uint32_t* in_pixels, const uint16_t width, const uint16_t height, const pep_format in_format, const pep_channel_bits in_channel_bits )
{
	pep out_pep = { 0 };
	uint32_t area = ( uint32_t )width * height;
	if( in_pixels == NULL || area == 0 ) return out_pep;

	out_pep.width = width;
	out_pep.height = height;
	out_pep.format = in_format;
	out_pep.channel_bits = in_channel_bits;

	uint8_t* idx = ( uint8_t* )PEP_MALLOC( area );
	
	uint32_t palette[ 256 ];
	uint16_t count = 0; // 1-256 distinct colours
	const uint32_t* p = in_pixels;
	const uint32_t* p_end = p + area;
	uint32_t last_p = 0,
	this_p = 0;
	while( p < p_end )
	{
		this_p = *p;
		if( p > in_pixels && this_p == last_p )
		{
			p++;
			continue;
		}
		uint16_t n = 0;
		while( n < count && this_p != palette[ n ] ) n++;
		if( n >= count && count < 256 ) palette[ count++ ] = this_p;
		last_p = this_p;
		p++;
	}
	memcpy( out_pep.palette, palette, ( size_t ) count * sizeof( uint32_t ) );
	out_pep.palette_size = ( uint8_t )count;
	for( uint32_t index = 0; index < area; index++ )
	{
		uint32_t c = in_pixels[ index ];
		uint16_t k = 0;
		while( k < count && palette[ k ] != c ) k++;
		idx[ index ] = ( uint8_t )( k < count ? k : 0 );
	}
	const uint32_t pcount = out_pep.palette_size ? out_pep.palette_size : 256;

	uint8_t* best = ( uint8_t* )PEP_MALLOC( ( size_t ) area * sizeof( uint32_t ) + 64 );
	uint8_t* tmp = ( uint8_t* )PEP_MALLOC( ( size_t ) area * sizeof( uint32_t ) + 64 );
	uint64_t best_size = 0;
	uint8_t best_model = 0;

	for( uint8_t model = 0; model < PEP_NUM_MODELS; model++ )
	{
		uint64_t size = _pep_encode_model( idx, width, height, pcount, model, tmp );
		if( size == 0 ) continue;
		
		if( best_size == 0 || size < best_size )
		{
			best_size = size;
			best_model = model;
			memcpy( best, tmp, size );
		}
	}

	out_pep.bytes = ( uint8_t* )PEP_REALLOC( best, best_size );
	out_pep.bytes_size = best_size;
	out_pep.model = ( uint8_t )best_model;

	PEP_FREE( tmp );
	PEP_FREE( idx );
	return out_pep;
}

// Decode all pixels with one model (mirror of _pep_encode_model).
static void _pep_decode_model( const uint8_t* in_bytes, uint64_t in_size, uint16_t width, uint16_t height, uint32_t pcount, uint8_t model, const uint32_t* palette, pep_format in_format, pep_format out_format, uint8_t pre_multiply, uint8_t* idx, uint32_t* out_pixels )
{
	_pep_model_context m;
	if( !_pep_model_init( &m, _pep_models[ model ], pcount ) ) return;
	const uint32_t index_escape = m.index_escape;

	_pep_ac_decode ac = { 0 };
	ac.range = ( uint32_t )( ( 1llu << 32 ) - 1 );
	ac.data_ref = ( uint8_t* )in_bytes;
	ac.end_of_data = ( uint8_t* )in_bytes + in_size;
	for( uint8_t index = 0; index < 4; ++index )
	{
		uint8_t b = 0;
		if( ac.data_ref != ac.end_of_data ) b = *ac.data_ref++;
		ac.code = ( ac.code << 8 ) | b;
	}

	uint16_t freq_max = PEP_FREQ_MAX;
	_pep_context* order0 = &m.contexts[ 0 ];
	uint32_t pos = 0;

	for( uint32_t y = 0; y < height; y++ )
	{
		for( uint32_t x = 0; x < width; x++, pos++ )
		{
			_pep_context* context = _pep_build_level( &m, idx, pos, x, y, width );
			uint32_t symbol = 0;
			uint8_t decoded_in_level = 0;

			if( context->sum != 0 )
			{
				uint32_t f = _pep_arith_decode_curr_freq( &ac, context->sum );
				_pep_sym_decode r = _pep_get_sym_from_freq( context, f );
				_pep_arith_decode_update( &ac, r.prob );

				if( r.symbol != index_escape )
				{
					symbol = r.symbol;
					_PEP_UPDATE( context, symbol, freq_max, pcount );
					decoded_in_level = 1;
				}
				else
				{
					context->freq[ index_escape ] += 1;
					context->sum += 1;
				}
			}

			if( !decoded_in_level )
			{
				uint32_t f = _pep_arith_decode_curr_freq( &ac, order0->sum );
				_pep_sym_decode r = _pep_get_sym_from_freq( order0, f );
				_pep_arith_decode_update( &ac, r.prob );
				symbol = r.symbol;
				_PEP_UPDATE( order0, symbol, freq_max, pcount );

				if( context->sum == 0 )
				{
					context->freq[ index_escape ] = 1;
					context->sum = 1;
				}
				context->freq[ symbol ] = 1;
				context->sum += 1;
			}

			idx[ pos ] = ( uint8_t )symbol;
			uint32_t pixel = ( symbol < pcount ) ? _pep_reformat( palette[ symbol ], in_format, out_format ) : 0;
			if( pre_multiply != 0 ) pixel = _pep_pre_multiply( pixel, out_format );
			out_pixels[ pos ] = pixel;
		}
	}
	_pep_model_free( &m );
}

uint32_t* pep_decompress( const pep* const in_pep, const pep_format out_format, const uint8_t transparent_first_color, uint8_t const pre_multiply )
{
	if( in_pep == NULL ) return NULL;
	if( in_pep->bytes == NULL || in_pep->bytes_size == 0 || in_pep->width == 0 || in_pep->height == 0 ) return NULL;

	const uint16_t width = in_pep->width,
	height = in_pep->height;
	const uint32_t area = ( uint32_t )width * height;
	uint32_t* out_pixels = ( uint32_t* )PEP_MALLOC( area * sizeof( uint32_t ) );
	uint8_t* idx = ( uint8_t* )PEP_MALLOC( area );

	const uint32_t pcount = in_pep->palette_size ? in_pep->palette_size : 256;
	static uint32_t palette[ 256 ];
	memcpy( palette, in_pep->palette, pcount * sizeof( uint32_t ) );
	if( transparent_first_color != 0 )
	{
		if( in_pep->format <= pep_bgra ) palette[ 0 ] &= 0x00ffffff;
		else palette[ 0 ] &= 0xffffff00;
	}

	uint8_t model = in_pep->model;
	if( model >= PEP_NUM_MODELS ) model = 0;

	_pep_decode_model( in_pep->bytes, in_pep->bytes_size, width, height, pcount, model, palette, in_pep->format, out_format, pre_multiply, idx, out_pixels );

	PEP_FREE( idx );
	return out_pixels;
}

void pep_free( pep* in_pep )
{
	if( in_pep && in_pep->bytes )
	{
		free( in_pep->bytes );
		in_pep->bytes = NULL;
		in_pep->bytes_size = 0;
	}
}

#pragma endregion visible

#pragma endregion compression

////////////////////////////////////////////////////////////////
#pragma region - serialization

uint8_t* pep_serialize( const pep* in_pep, uint32_t* const out_size )
{
	if( !in_pep || !in_pep->width || !in_pep->height || !in_pep->bytes_size || !in_pep->bytes )
	{
		*out_size = 0;
		return NULL;
	}

	uint16_t palette_count = in_pep->palette_size ? in_pep->palette_size : 256;

	const uint16_t w = in_pep->width - 1;
	const uint16_t h = in_pep->height - 1;
	const uint8_t is_small = ( w <= 255 && h <= 255 ) ? 1 : 0;
	const uint8_t dim_bytes = is_small ? 2 : 3;

	// calculate variable-length size bytes
	uint8_t size_bytes = 0;
	uint32_t temp_size = in_pep->bytes_size;
	while( temp_size >= 0x80 )
	{
		size_bytes++;
		temp_size >>= 7;
	}
	++size_bytes;

	// check if bitmap (black and white)
	uint8_t is_bitmap = 0;
	if( palette_count == 2 )
	{
		const uint8_t* const color0 = ( uint8_t* )( &in_pep->palette[ 0 ] );
		const uint8_t* const color1 = ( uint8_t* )( &in_pep->palette[ 1 ] );

		const uint8_t is_white0 = ( color0[ 0 ] == 255 && color0[ 1 ] == 255 && color0[ 2 ] == 255 && color0[ 3 ] == 255 );
		const uint8_t is_white1 = ( color1[ 0 ] == 255 && color1[ 1 ] == 255 && color1[ 2 ] == 255 && color1[ 3 ] == 255 );

		if( in_pep->format <= pep_bgra )
		{
			is_bitmap = ( ( is_white0 && color1[ 0 ] == 0 && color1[ 1 ] == 0 && color1[ 2 ] == 0 && color1[ 3 ] == 255 ) || ( color0[ 0 ] == 0 && color0[ 1 ] == 0 && color0[ 2 ] == 0 && color0[ 3 ] == 255 && is_white1 ) );
		}
		else
		{
			is_bitmap = ( ( is_white0 && color1[ 0 ] == 255 && color1[ 1 ] == 0 && color1[ 2 ] == 0 && color1[ 3 ] == 0 ) || ( color0[ 0 ] == 255 && color0[ 1 ] == 0 && color0[ 2 ] == 0 && color0[ 3 ] == 0 && is_white1 ) );
		}
	}

	// check if all palette alphas are 255
	uint8_t only_rgb = 1;
	if( !is_bitmap )
	{
		for( uint16_t index = 0; index < palette_count; index++ )
		{
			if( ( ( ( uint8_t* )( &in_pep->palette[ index ] ) )[ 3 ] ) != 0xff )
			{
				only_rgb = 0;
				break;
			}
		}
	}

	// calculate palette bytes
	uint16_t palette_bytes = 0;
	if( !is_bitmap )
	{
		const uint16_t channel_bits = 1 << in_pep->channel_bits;
		if( channel_bits == 8 )
		{
			palette_bytes = palette_count * ( only_rgb ? 3 : 4 );
		}
		else
		{
			const uint16_t channels = only_rgb ? 3 : 4;
			palette_bytes = ( channel_bits * channels * palette_count + 7 ) >> 3;
		}
	}

	// allocate the exact size (subtract 1 for palette_size byte if bitmap)
	const uint64_t total_size = dim_bytes + size_bytes + ( is_bitmap ? 0 : 1 ) + palette_bytes + in_pep->bytes_size + 4 + 1;
	uint8_t* out_bytes = ( uint8_t* )PEP_MALLOC( total_size );
	uint8_t* out_bytes_ref = out_bytes;

	// flags: format (2), channel_bits (2), is_small (1), only_rgb (1), is_bitmap (1)
	*out_bytes_ref++= ( in_pep->format & 0x3 ) | ( ( in_pep->channel_bits & 0x3 ) << 2 ) | ( ( is_small & 0x1 ) << 4 ) | ( ( only_rgb & 0x1 ) << 5 ) | ( ( is_bitmap & 0x1 ) << 6 );
	*out_bytes_ref++= in_pep->model; // takes the last extra byte

	// width/height
	if( is_small )
	{
		*out_bytes_ref++= w & 0xff;
		*out_bytes_ref++= h & 0xff;
	}
	else
	{
		const uint32_t packed_dims = ( ( w & 0xfff ) << 12 ) | ( h & 0xfff );
		*out_bytes_ref++= ( packed_dims >> 16 ) & 0xff;
		*out_bytes_ref++= ( packed_dims >> 8 ) & 0xff;
		*out_bytes_ref++= packed_dims & 0xff;
	}

	// variable-length size
	uint32_t size = in_pep->bytes_size;
	while( size >= 0x80 )
	{
		*out_bytes_ref++= ( size | 0x80 ) & 0xff;
		size >>= 7;
	}
	*out_bytes_ref++= size;

	if( !is_bitmap )
	{
		// palette size
		*out_bytes_ref++= in_pep->palette_size;

		// palette
		const uint16_t channel_bits = 1 << in_pep->channel_bits;
		const uint8_t shift = 8 - channel_bits;
		const uint8_t mask = ( 1 << channel_bits ) - 1;

		if( channel_bits == 8 )
		{
			for( uint16_t index = 0; index < palette_count; index++ )
			{
				uint8_t* color = ( uint8_t* )( &( in_pep->palette[ index ] ) );
				*out_bytes_ref++= color[ 0 ];
				*out_bytes_ref++= color[ 1 ];
				*out_bytes_ref++= color[ 2 ];
				if( !only_rgb ) *out_bytes_ref++= color[ 3 ];
			}
		}
		else
		{
			uint32_t bit_buffer = 0;
			uint8_t bit_count = 0;

			for( uint16_t index = 0; index < palette_count; index++ )
			{
				uint8_t* color = ( uint8_t* )( &( in_pep->palette[ index ] ) );

				bit_buffer = ( bit_buffer << channel_bits ) | ( ( color[ 0 ] >> shift ) &mask );
				bit_buffer = ( bit_buffer << channel_bits ) | ( ( color[ 1 ] >> shift ) &mask );
				bit_buffer = ( bit_buffer << channel_bits ) | ( ( color[ 2 ] >> shift ) &mask );
				bit_count += channel_bits * 3;

				if( !only_rgb )
				{
					bit_buffer = ( bit_buffer << channel_bits ) | ( ( color[ 3 ] >> shift ) &mask );
					bit_count += channel_bits;
				}

				while( bit_count >= 8 )
				{
					bit_count -= 8;
					*out_bytes_ref++= ( bit_buffer >> bit_count ) & 0xff;
				}
			}

			if( bit_count > 0 )
			{
				*out_bytes_ref++= ( bit_buffer << ( 8 - bit_count ) ) & 0xff;
			}
		}
	}

	memcpy( out_bytes_ref, in_pep->bytes, in_pep->bytes_size );
	out_bytes_ref += in_pep->bytes_size;
	*out_bytes_ref = '\0';

	*out_size = out_bytes_ref - out_bytes;
	return out_bytes;
}

pep pep_deserialize( const uint8_t* const in_bytes, const uint32_t in_bytes_size )
{
	pep out_pep = { 0 };

	if( !in_bytes || in_bytes_size == 0 ) return out_pep;

	const uint8_t* bytes_ref = in_bytes;
	const uint8_t* const bytes_end = in_bytes + in_bytes_size;

	// packed flags
	if( bytes_ref + 1 > bytes_end ) return out_pep;
	uint8_t packed_flags = *bytes_ref++;
	if( bytes_ref + 1 > bytes_end ) return out_pep;
	out_pep.model = *bytes_ref++;
	out_pep.format = ( pep_format ) ( packed_flags & 0x3 );
	out_pep.channel_bits = ( pep_channel_bits ) ( ( packed_flags >> 2 ) & 0x3 );
	uint8_t is_small = ( packed_flags >> 4 ) & 0x1;
	uint8_t only_rgb = ( packed_flags >> 5 ) & 0x1;
	uint8_t is_bitmap = ( packed_flags >> 6 ) & 0x1;

	// width/height
	uint8_t dim_bytes = is_small ? 2 : 3;
	if( bytes_ref + dim_bytes > bytes_end ) return out_pep;
	uint16_t w,
	h;
	if( is_small )
	{
		w = *bytes_ref++;
		h = *bytes_ref++;
	}
	else
	{
		uint32_t packed_dims = ( *bytes_ref++ << 16 );
		packed_dims |= ( *bytes_ref++ << 8 );
		packed_dims |= *bytes_ref++;
		w = ( packed_dims >> 12 ) & 0xfff;
		h = packed_dims & 0xfff;
	}
	out_pep.width = w + 1;
	out_pep.height = h + 1;

	// variable-length size
	uint32_t bytes_size = 0;
	uint8_t shift = 0;
	uint8_t byte_val;
	do
	{
		if( bytes_ref >= bytes_end ) return out_pep;
		byte_val = *bytes_ref++;
		if( shift < 32 ) bytes_size |= ( ( uint32_t )( byte_val &0x7f ) ) << shift;
		shift += 7;
	}
	while( ( byte_val & 0x80 ) && shift < 35 );
	out_pep.bytes_size = bytes_size;

	// handle bitmap or read palette
	uint32_t remaining = ( uint32_t )( bytes_end - bytes_ref );

	if( is_bitmap )
	{
		if( remaining < bytes_size ) return out_pep;

		out_pep.palette_size = 2;
		out_pep.palette[ 0 ] = out_pep.format <= pep_bgra ? 0xff000000 : 0x000000ff;
		out_pep.palette[ 1 ] = 0xffffffff;
	}
	else
	{
		// palette size
		if( remaining < 1 ) return out_pep;
		out_pep.palette_size = *bytes_ref++;
		remaining--;

		uint16_t palette_count = out_pep.palette_size ? out_pep.palette_size : 256;

		// palette
		const uint8_t channel_bits = 1 << out_pep.channel_bits;
		const uint8_t mask = ( 1 << channel_bits ) - 1;
		uint32_t palette_bytes;
		if( channel_bits == 8 )
		{
			palette_bytes = palette_count * ( only_rgb ? 3 : 4 );
		}
		else
		{
			uint16_t channels = only_rgb ? 3 : 4;
			palette_bytes = ( channel_bits * channels * palette_count + 7 ) >> 3;
		}

		if( remaining < palette_bytes || remaining - palette_bytes < bytes_size ) return out_pep;

		if( channel_bits == 8 )
		{
			for( uint16_t index = 0; index < palette_count; index++ )
			{
				uint8_t* color = ( uint8_t* )( &( out_pep.palette[ index ] ) );
				color[ 0 ] = *bytes_ref++;
				color[ 1 ] = *bytes_ref++;
				color[ 2 ] = *bytes_ref++;
				color[ 3 ] = only_rgb ? 0xff : *bytes_ref++;
			}
		}
		else
		{
			uint32_t bit_buffer = 0;
			uint8_t bit_count = 0;

			for( uint16_t index = 0; index < palette_count; index++ )
			{
				uint8_t channels = only_rgb ? 3 : 4;
				uint8_t channel_values[ 4 ];

				for( uint8_t c = 0; c < channels; c++ )
				{
					while( bit_count < channel_bits )
					{
						bit_buffer = ( bit_buffer << 8 ) | *bytes_ref++;
						bit_count += 8;
					}
					bit_count -= channel_bits;
					channel_values[ c ] = ( bit_buffer >> bit_count ) & mask;

					uint8_t scaled = channel_values[ c ] << ( 8 - channel_bits );
					if( channel_bits < 8 )
					{
						scaled |= ( scaled >> channel_bits );
						if( channel_bits < 4 )
						{
							scaled |= ( scaled >> ( 2 * channel_bits ) );
						}
					}
					channel_values[ c ] = scaled;
				}

				uint8_t* color = ( uint8_t* )( &( out_pep.palette[ index ] ) );
				color[ 0 ] = channel_values[ 0 ];
				color[ 1 ] = channel_values[ 1 ];
				color[ 2 ] = channel_values[ 2 ];
				color[ 3 ] = only_rgb ? 0xff : channel_values[ 3 ];
			}
		}
	}

	// copy image data
	out_pep.bytes = ( uint8_t* )PEP_MALLOC( bytes_size );
	if( out_pep.bytes )
	{
		memcpy( out_pep.bytes, bytes_ref, bytes_size );
	}

	return out_pep;
}

#pragma endregion serialization

////////////////////////////////////////////////////////////////
#pragma region - file

// For both save/load, file_path should end in ".pep":
// e.g. "texture.pep", "assets/image.pep"

// Saves pep into a file.
// Returns 0 on failure, 1 on success
uint8_t pep_save( const pep* const in_pep, const char* const file_path )
{
	if( !in_pep || !file_path )
	{
		return 0;
	}

	uint32_t bytes_size = 0;
	uint8_t* bytes = pep_serialize( in_pep, &bytes_size );

	if( !bytes || bytes_size == 0 )
	{
		return 0;
	}

	FILE * file = fopen( file_path, "wb" );
	if( !file )
	{
		PEP_FREE( bytes );
		return 0;
	}

	size_t written = fwrite( bytes, 1, bytes_size, file );

	fclose( file );
	PEP_FREE( bytes );

	#ifdef PEP_DEBUG
		printf( "pep: %lld\nfile: %zu\nmodel: %d\n", in_pep->bytes_size, written, in_pep->model );
	#endif

	return written == bytes_size;
}

// Loads .pep file into returned pep struct
pep pep_load( const char* const file_path )
{
	pep out_pep = { 0 };

	if( !file_path )
	{
		return out_pep;
	}

	FILE * file = fopen( file_path, "rb" );
	if( !file )
	{
		return out_pep;
	}

	fseek( file, 0, SEEK_END );
	long file_size = ftell( file );
	fseek( file, 0, SEEK_SET );

	if( file_size <= 0 )
	{
		fclose( file );
		return out_pep;
	}

	uint8_t* bytes = ( uint8_t* )PEP_MALLOC( file_size );

	size_t read = fread( bytes, 1, file_size, file );
	fclose( file );

	if( read != ( size_t ) file_size )
	{
		PEP_FREE( bytes );
		return out_pep;
	}

	out_pep = pep_deserialize( bytes, ( uint32_t )read );
	PEP_FREE( bytes );

	#ifdef PEP_DEBUG
		printf( "\npep: %lld\nfile: %ld\n", out_pep.bytes_size, file_size );
	#endif

	return out_pep;
}

#pragma endregion file

#ifdef _MSC_VER
	#pragma warning( pop )
#endif

#endif // PEP_IMPLEMENTATION


#pragma endregion definitions

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
