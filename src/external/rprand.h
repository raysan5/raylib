/**********************************************************************************************
*
*   rprand v1.0 - A simple and easy-to-use pseudo-random numbers generator (PRNG)
*
*   FEATURES:
*       - Pseudo-random values generation, 32 bits: [0..4294967295]
*       - Sequence generation avoiding duplicate values
*       - Using standard and proven prng algorithm (Xoshiro128**)
*       - State initialized with a separate generator (SplitMix64)
*
*   LIMITATIONS:
*       - No negative numbers, up to the user to manage them
*
*   POSSIBLE IMPROVEMENTS:
*       - Support 64 bits generation
*
*   ADDITIONAL NOTES:
*     This library implements two pseudo-random number generation algorithms: 
*
*         - Xoshiro128** : https://prng.di.unimi.it/xoshiro128starstar.c
*         - SplitMix64   : https://prng.di.unimi.it/splitmix64.c
*
*     SplitMix64 is used to initialize the Xoshiro128** state, from a provided seed
*
*     It's suggested to use SplitMix64 to initialize the state of the generators starting from 
*     a 64-bit seed, as research has shown that initialization must be performed with a generator 
*     radically different in nature from the one initialized to avoid correlation on similar seeds.
*
*   CONFIGURATION:
*       #define RPRAND_IMPLEMENTATION
*           Generates the implementation of the library into the included file.
*           If not defined, the library is in header only mode and can be included in other headers
*           or source files without problems. But only ONE file should hold the implementation.
* 
*   DEPENDENCIES: none
*
*   VERSIONS HISTORY:
*       1.0 (01-Jun-2023) First version
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2023 Ramon Santamaria (@raysan5)
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

#ifndef RPRAND_H
#define RPRAND_H

#define RPRAND_VERSION    "1.0"

// Function specifiers in case library is build/used as a shared library (Windows)
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
#if defined(_WIN32)
    #if defined(BUILD_LIBTYPE_SHARED)
        #define RPRAND __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define RPRAND __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#endif

// Function specifiers definition
#ifndef RPRANDAPI
    #define RPRANDAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Allow custom memory allocators
#ifndef RPRAND_CALLOC
    #define RPRAND_CALLOC(ptr,sz)     calloc(ptr,sz)
#endif
#ifndef RPRAND_FREE
    #define RPRAND_FREE(ptr)          free(ptr)
#endif

// Simple log system to avoid RPNG_LOG() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define RPRAND_SHOW_LOG_INFO
#if defined(RPNG_SHOW_LOG_INFO)
  #define RPRAND_LOG(...) printf(__VA_ARGS__)
#else
  #define RPRAND_LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
//...

#ifdef __cplusplus
extern "C" {                // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
RPRANDAPI void rprand_set_seed(unsigned long long seed);        // Set rprand_state for Xoshiro128**, seed is 64bit
RPRANDAPI int rprand_get_value(int min, int max);               // Get random value within a range, min and max included

RPRANDAPI int *rprand_load_sequence(unsigned int count, int min, int max); // Load pseudo-random numbers sequence with no duplicates
RPRANDAPI void rprand_unload_sequence(int *sequence);           // Unload pseudo-random numbers sequence

#ifdef __cplusplus
}
#endif

#endif // RPRAND_H

/***********************************************************************************
*
*   RPRAND IMPLEMENTATION
*
************************************************************************************/

#if defined(RPRAND_IMPLEMENTATION)

#include <stdlib.h>     // Required for: calloc(), free(), abs()
#include <stdint.h>     // Required for data types: uint32_t, uint64_t

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static uint64_t rprand_seed = 0xAABBCCDD;       // SplitMix64 default seed (aligned to rprand_state)
static uint32_t rprand_state[4] = {             // Xoshiro128** state, initialized by SplitMix64
    0x96ea83c1,
    0x218b21e5,
    0xaa91febd,
    0x976414d4
};        

//----------------------------------------------------------------------------------
// Module internal functions declaration
//----------------------------------------------------------------------------------
static uint32_t rprand_xoshiro(void);           // Xoshiro128** generator (uses global rprand_state)
static uint64_t rprand_splitmix64(void);        // SplitMix64 generator (uses seed to generate rprand_state)

//----------------------------------------------------------------------------------
// Module functions definition
//----------------------------------------------------------------------------------
// Set rprand_state for Xoshiro128**
// NOTE: We use a custom generation algorithm using SplitMix64
void rprand_set_seed(unsigned long long seed)
{
    rprand_seed = (uint64_t)seed;    // Set SplitMix64 seed for further use

    // To generate the Xoshiro128** state, we use SplitMix64 generator first
    // We generate 4 pseudo-random 64bit numbers that we combine using their LSB|MSB
    rprand_state[0] = (uint32_t)(rprand_splitmix64() & 0xffffffff);
    rprand_state[1] = (uint32_t)((rprand_splitmix64() & 0xffffffff00000000) >> 32);
    rprand_state[2] = (uint32_t)(rprand_splitmix64() & 0xffffffff);
    rprand_state[3] = (uint32_t)((rprand_splitmix64() & 0xffffffff00000000) >> 32);
}

// Get random value within a range, min and max included
int rprand_get_value(int min, int max)
{
    int value = rprand_xoshiro()%(abs(max - min) + 1) + min;

    return value;
}

// Load pseudo-random numbers sequence with no duplicates, min and max included
int *rprand_load_sequence(unsigned int count, int min, int max)
{
    int *sequence = NULL;
    
    if (count > (unsigned int)(abs(max - min) + 1)) 
    {
        RPRAND_LOG("WARNING: Sequence count required is greater than range provided\n");
        //count = (max - min);
        return sequence;
    }

    sequence = (int *)RPRAND_CALLOC(count, sizeof(int));

    int value = 0;
    bool value_is_dup = false;

    for (unsigned int i = 0; i < count;)
    {
        value = ((unsigned int)rprand_xoshiro()%(abs(max - min) + 1)) + min;

        for (unsigned int j = 0; j < i; j++)
        {
            if (sequence[j] == value)
            {
                value_is_dup = true;
                break;
            }
        }

        if (!value_is_dup)
        {
            sequence[i] = value;
            i++;
        }

        value_is_dup = false;
    }

    return sequence;
}

// Unload pseudo-random numbers sequence
void rprand_unload_sequence(int *sequence)
{
    RPRAND_FREE(sequence);
    sequence = NULL;
}

//----------------------------------------------------------------------------------
// Module internal functions definition
//----------------------------------------------------------------------------------
static inline uint32_t rprand_rotate_left(const uint32_t x, int k)
{
    return (x << k) | (x >> (32 - k));
}

// Xoshiro128** generator info:
//   
//   Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)
//   
//   To the extent possible under law, the author has dedicated all copyright
//   and related and neighboring rights to this software to the public domain
//   worldwide. This software is distributed without any warranty.
//   
//   See <http://creativecommons.org/publicdomain/zero/1.0/>.
//   
//   This is xoshiro128** 1.1, one of our 32-bit all-purpose, rock-solid
//   generators. It has excellent speed, a state size (128 bits) that is
//   large enough for mild parallelism, and it passes all tests we are aware
//   of.
// 
//   Note that version 1.0 had mistakenly s[0] instead of s[1] as state
//   word passed to the scrambler.
// 
//   For generating just single-precision (i.e., 32-bit) floating-point
//   numbers, xoshiro128+ is even faster.
// 
//   The state must be seeded so that it is not everywhere zero.
//
uint32_t rprand_xoshiro(void)
{
    const uint32_t result = rprand_rotate_left(rprand_state[1]*5, 7)*9;
    const uint32_t t = rprand_state[1] << 9;

    rprand_state[2] ^= rprand_state[0];
    rprand_state[3] ^= rprand_state[1];
    rprand_state[1] ^= rprand_state[2];
    rprand_state[0] ^= rprand_state[3];
    
    rprand_state[2] ^= t;
    
    rprand_state[3] = rprand_rotate_left(rprand_state[3], 11);

    return result;
}

// SplitMix64 generator info:
//   
//   Written in 2015 by Sebastiano Vigna (vigna@acm.org)
//   
//   To the extent possible under law, the author has dedicated all copyright
//   and related and neighboring rights to this software to the public domain
//   worldwide. This software is distributed without any warranty.
//   
//   See <http://creativecommons.org/publicdomain/zero/1.0/>.
//   
//
//   This is a fixed-increment version of Java 8's SplittableRandom generator
//   See http://dx.doi.org/10.1145/2714064.2660195 and
//   http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
//   
//   It is a very fast generator passing BigCrush, and it can be useful if
//   for some reason you absolutely want 64 bits of state.
uint64_t rprand_splitmix64()
{
    uint64_t z = (rprand_seed += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30))*0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27))*0x94d049bb133111eb;
    return z ^ (z >> 31);
}

#endif  // RPRAND_IMPLEMENTATION