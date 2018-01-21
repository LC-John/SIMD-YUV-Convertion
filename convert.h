#ifndef CONVERT_H
#define CONVERT_H

#include "mmintrin.h"
#include "xmmintrin.h"
#include "immintrin.h"
#include "emmintrin.h"

typedef enum _SIMD_MODE
{
    SIMD_NO,
    SIMD_MMX,
    SIMD_SSE,
    SIMD_AVX
} SIMD_MODE;

typedef union _MMX_PACK
{
    unsigned char i8[8];
    unsigned short i16[4];
    unsigned int i32[2];
    unsigned long long i64[0];
    __m64 p;
} MMX_PACK;

typedef union _SSE_PACK
{
    unsigned char i8[16];
    unsigned short i16[8];
    unsigned int i32[4];
    unsigned long long i64[2];
    float f32[4];
    double f64[2];
    __m128 p;
    __m128i pi;
} SSE_PACK;

typedef union _AVX_PACK
{
    unsigned char i8[32];
    unsigned short i16[16];
    unsigned int i32[8];
    unsigned long long i64[4];
    float f32[8];
    double f64[4];
    __m256 p;
    __m256d pd;
    __m256i pi;
} AVX_PACK;

const int w = 1920;
const int h = 1080;
const int n_frame = 84;

/***** Functions for task 1 *****/
int load_yuv420(char* a_name);
int yuv420_2_argb8888(unsigned char alpha, SIMD_MODE mode = SIMD_NO);
int argb8888_2_rgb888(SIMD_MODE mode = SIMD_NO);

int store_yuv420(char* a_name);

/***** Functions for task 2 *****/
int load_yuv420_2(char* a1_name, char* a2_name);
int yuv420_2_rgb888_2(SIMD_MODE mode = SIMD_NO);
int merge_rgb888(int alpha, SIMD_MODE mode = SIMD_NO);

/***** Functions shared by the 2 tasks *****/
int rgb888_2_yuv420(int frame, SIMD_MODE mode = SIMD_NO);

int store_rgb888(char* a_name);
int store_yuv420_vedio(char* a_name);
int store_yuv420_frame(char* a_name, int frame);


#endif // CONVERT_H
