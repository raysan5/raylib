#pragma once
#include <immintrin.h>
#include <math.h>


#ifdef sse_sse2

//Sizes
#define __simd_8_size 16
#define __simd_16_size 8
#define __simd_32_size 4
#define __simd_64_size 2

//Variables
#define __simd_f __m128
#define __simd_d __m128d
#define __simd_i __m128i

//Intrinsics
#define __simd_f_set_ps1(a) (_mm_set_ps1(a))

#define __simd_f_add_ps(a, b) (_mm_add_ps((a), (b)))
#define __simd_f_sub_ps(a, b) (_mm_sub_ps(a, b))
#define __simd_f_mul_ps(a, b) (_mm_mul_ps(a, b))
#define __simd_f_reciprocal_sqrt_ps(a) (_mm_rsqrt_ps(a))


#define __simd_f_cmpare_lower_or_equal_ps(a, b) (_mm_cmple_ps(a, b))
#define __simd_f_cmpare_grater_or_equal_ps(a, b) (_mm_cmpge_ps(a, b))
#define __simd_f_cmpare_not_equal_ps(a, b) (_mm_cmpneq_ps(a, b))

#define __simd_i__mm_cast_f_to_i_ps_si128(a) (_mm_castps_si128(a))

#define __simd_f_and_ps(a, b) (_mm_and_ps(a, b))
#define __simd_f_not_and_ps(a, b) (_mm_andnot_ps(a, b))
#else

  
#define __simd_8_size 1
#define __simd_16_size 1
#define __simd_32_size 1
#define __simd_64_size 1



#define __simd_f float
#define __simd_d double
#define __simd_i int

#define __simd_f_set_ps1(a) (a)

#define __simd_f_add_ps(a, b) (a+b)
#define __simd_f_sub_ps(a, b) (a-b)
#define __simd_f_mul_ps(a, b) (a*b)
#define __simd_f_reciprocal_sqrt_ps(a) (1/sqrtf(a))


#define __simd_f_cmpare_lower_or_equal_ps(a, b) (a<=b)
#define __simd_f_cmpare_grater_or_equal_ps(a, b) (a>=b)
#define __simd_f_cmpare_not_equal_ps(a, b) (a!=b)

#define __simd_i__mm_cast_f_to_i_ps_si128(a) ((int)a)

#define __simd_f_and_ps(a, b) (a&&b)

#define __simd_f_not_and_ps(a, b) (!a)

#endif


//simd structs

typedef struct Vector3_simd {
	__simd_f x;
	__simd_f y;
	__simd_f z;
}Vector3_simd;


typedef struct Matrix_simd {
	__simd_f m0, m4, m8, m12;       // Matrix first row (4 components)
	__simd_f m1, m5, m9, m13;       // Matrix second row (4 components)
	__simd_f m2, m6, m10, m14;      // Matrix third row (4 components)
	__simd_f m3, m7, m11, m15;      // Matrix fourth row (4 components)
} Matrix_simd;

typedef struct Color_float_simd{
  
  __simd_f r, g, b, a;
  
}Color_float_simd;

// simd structs

